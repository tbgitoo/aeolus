// ----------------------------------------------------------------------------
//
//  Copyright (C) 2003-2013 Fons Adriaensen <fons@linuxaudio.org>
//    
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Thomas Braschler/Mathis Braschler (2025): Return and set functions for specific parameters
// Also, renamed class to AeolusAudio since audio is really quite generic
// ----------------------------------------------------------------------------


#ifndef AEOLUS_AUDIO_H
#define AEOLUS_AUDIO_H

#include "asection.h"
#include "division.h"
#include "lfqueue.h"
#include "reverb.h"
#include "global.h"
#include "../../clthreads/include/clthreads.h"

/**
 * Base class for the audio processing part of the Aeolus synthesizer. This class holds and orchestrates
 * the divisions (holding the ranks), the audio sections (for spatialization) as well as the reverb processor and
 * orchestrates their action.<br /><br />
 * In terms of thread architecture, this class operates at two levels. Firstly, upon invoking the start function
 * this class works as its own thread capable of receiving messages through the clthreads framework. This serves to
 * set settings, and also to process incoming notes to keep the list of currently playing keyboards (in the
 * simplest implementation, as in here, divisions) in the _keymap field
 * up to date.<br /> Additionnally,
 * the proc_synth function is intended to be invoked from the audio processing system (in this Android implementation
 * through the Audio framework oboe, more precisely through
 * onAudioReady in AeolusOscillator, and fillAudioBuffer in the AeolusSynthesizer class, which is derived from this class).
 * Thread-wise, this invokation of proc_synth through the audio system allocates optimal ressources to the real-time
 * processing, while background tasks and messages are performed on the background thread.
 */
class AeolusAudio : public A_thread
{
public:
    /**
     * Constructor with application name, and note (midi) and communication queues
     * @param jname Application name
     * @param qnote The midi note queue
     * @param qcomm Communication queue for the other threads
     */
    AeolusAudio (const char *jname, Lfq_u32 *qnote, Lfq_u32 *qcomm);
    /**
     * Destructor, includes destruction of the divisions, audiosections and the reverb processor
     */
    virtual ~AeolusAudio ();
    /**
     * Start the message handling thread. From the point of view of this class, this is the proper main thread
     */
    virtual void  start ();

    /**
     * Get the application name
     * @return Application name as 0-terminated C string
     */
    [[nodiscard]] const char  *appname () const { return _appname; }
    /**
     * Get the midi map
     * @return Midi map. The midimap has 16 entries, 1 for each midi channel. Each entry is a 16-bit number, although
     *                   at present only the second (least significant) byte is used.<br /><br />
     *                   The entries encode routing of the midi channels to the divisions, 1 being active routing,
     *                   and 0 indicating no routing of the particular midi channel to the chosen division.
     *                   The divisions are associated to bits of increasing significance. For example,
     *                   if midimap[2]=0b00000000 00010100, this means that midi channel #3 (that is the midichannel
     *                   with the C-based index of 2, since indexing in C is 0-based) is routed
     *                   to division #3 and division #5 (aka divisions 2 and 4 with zero-based indexing). In practice,
     *                   bit 8 is used internally by the Aeolus synthesizer, and only up to 7 divisions can be encoded.
     */
    [[nodiscard]] uint16_t    *midimap () const { return (uint16_t *) _midimap; }

    [[nodiscard]] int  policy () const { return _policy; }
    /**
     * Get thread priority
     * @return Absolute thread priority
     */
    [[nodiscard]] int  abspri () const { return _abspri; }
    /**
     * Get thread priority
     * @return Relative thread priority
     */
    [[nodiscard]] int  relpri () const { return _relpri; }
    /**
     * Length of the midimap
     * @return By default, 16 in this implementation
     */

    [[nodiscard]] int get_midimap_length() const {return 16; }

    /**
     * Get the midi map entry for a specific midi channel
     * @param midi_index The midi channel index
     * @return The midimap entry as 16-bit binary mask for the divisions. It seems that bit masking
     *         works only up to 7 divisions (127 as a maximum value for the midimap entry)
     */

    [[nodiscard]] uint16_t get_midi_map_entry(int midi_index);

    /**
     * Set a midi map bit
     * @param my_division_index The division to be activated/deactived
     * @param my_midi_channel_index The midi-channel for which the division is to be activated / deactived
     *                              It seems that this should be 0-7 and not larger.
     * @param is_checked true for activation, false for inactivation
     */

    void setMidiMapBit(int my_division_index,int my_midi_channel_index,bool is_checked);

    /**
     * Get the volume for a division
     * @param division_index The division for which the volume should be read
     * @return Volume as linear gain
     */
    [[nodiscard]] float getVolumeForDivision(int division_index);

    /**
     * Set the volume for a division
     * @param division_index The index of the division
     * @param division_volume The volume to be set, as linear gain
     */

    void setVolumeForDivision(int division_index, float division_volume);

    /**
     * Is the tremulant for this division on?
     * @param division_index The division index
     * @return true if tremulant is on, false if it is off or if the division does not have a tremulant (c.f.
     * instrument definition file)
     */

    [[nodiscard]] bool tremulantIsOn(int division_index);


protected:

    /**
     * Initialization. This initializes audio sections and reverb. The divisions take longer because
     * of the loading or creation of the ranks and is handled through the model and slave threads.
     */
    void init_audio ();
    /**
     * Process messaging (from modeL) or midi (via incoming midi messages) queue
     * Regarding the midi pathway, the midi messages have already processed such that
     * the associated mask (terminal byte) reflects the impacted keyboards
     */
    void proc_queue (Lfq_u32 *);

    /**
     * Process synthesizers. nframes is the number of frames to be filled. A frame in audio buffer terminology
     * corresponds to the values played at the same time. For example, in a mono audio stream,
     * there is exactly 1 sample per frame, while in stereo there is two. This nframes parameter
     * is provided by the hardware audio driver, in the oboe framework through the corresponding
     * parameter in the call to onAudioReady (implemented here in AeolusOscillator). Call this
     * function as part of the call back from the audio driver (here, oboe, through AeolusOscillator,
     * invoking fillAudioBuffer on AeolusSynthesizer, which in turn invokes this function since
     * AeolusSynthesizer inherits from this class).
     */
    void proc_synth (int nframes);
    /**
     * Update divisions to take into account the current state of keys recently pushed (the ones with the
     * 128-status bit set
     */
    void proc_keys1 ();
    /**
      * Update divisions to take into account newly activated or deactivated keys
      */
    void proc_keys2 ();
    /**
     * Process messages from other threads (clthreads framework)
     */
    void proc_mesg ();

    /**
     * Hook for a possible additional function. This is invoked for every synth period (there are several of them
     * per audio driver invocation call as the number of samples process at once is limited to PERIOD=64 samples
     * as defined in Asect.h). This event is invoked before the division and audio section processing for each period.
     */
    virtual void on_synth_period(int) {}

    /**
     * Indexes to the global instrument parameters stored in
     * the _audiopar field of this class:<br />
     * VOLUME: Global volume of the Aeolus synthesizer (linear gain)<br />
     * REVSIZE: Reverb parameter<br />
     * REVTIME: Reverb parameter<br />
     * STPOSIT: Position front back for spatialization (fixed in mono- and stereo)
     */
    enum { VOLUME, REVSIZE, REVTIME, STPOSIT };

    /**
     * Switch off key for midi channels determined by mask b
     * @param n Midi key (e.g. middle C =60, half-tone scale as defined by the midi standard)
     * @param b Mask for accessing the divisions. This is a binary mask, with intended values from 0
     *          to 127. Only divisions with a 1 at the corresponding place will be switched on, for
     *          example, providing b=4=0b0000 0100 will switch off division #3 (index=2 since zero-based)
     */

    void key_off (int n, int b)
    {

        _keymap [n] &= ~b;
        _keymap [n] |= 128;

    }

    /**
     * Switch on key for midi channels determined by mask b
     * @param n Midi key (e.g. middle C =60, half-tone scale as defined by the midi standard)
     * @param b Mask for accessing the midi channels.
     */

    void key_on (int n, int b)
    {

        _keymap [n] |= b | 128;

    }

    /** Conditional key off
     * With conditional key off, you can condition the switch-off operation on the current value of the keymap
     * entries. This function runs through the entire keymap (so all midi notes), and if there is a match
     * between current activation state , the mask b
     * is used to switch off desired channels
     * @param m Mask to select notes to operate on
     * @param b Channel mask to select channels to switch off
     */

    void cond_key_off (int m, int b)
    {
        int            i;
        unsigned char  *p;

        for (i = 0, p = _keymap; i < NNOTES; i++, p++)
        {
            if (*p & m)
            {
                *p &= ~b;
                *p |= 128;
            }
        }
    }

    /** Conditional key on
     * With conditional key on, you can condition the switch-on operation on the current value of the keymap
     * entries.
     * @param m Mask to select notes to operate on
     * @param b Channel mask to select to switch on
     */

    void cond_key_on (int m, int b)
    {
        int            i;
        unsigned char  *p;

        for (i = 0, p = _keymap; i < NNOTES; i++, p++)
        {
            if (*p & m)
            {
                *p |= b | 128;
            }
        }
    }


    /** internal application name */
    const char     *_appname;

    /** Midimap indicating the routing of midi messages to the divisions, initialized to all zero (all divisions
     * off for all midi channels
     */
    uint16_t        _midimap [16]{};
    /** Incoming midi note queue */
    Lfq_u32        *_qnote;
    /** Incoming communication queue from other threads */
    Lfq_u32        *_qcomm;
    /** Is the process associated with this class running (this concerns the message handling process, not the audio process) */
    volatile bool   _running;

    int             _policy;
    int             _abspri;
    int             _relpri;
    int             _hold;
    /**
     * Number of output channels (1=mono, 2=stereo, 8 for multichannel custom processing),
     * this needs to be obtained from the audio system driver (here, oboe) because it conditions the output to the
     * audio system
     * See constructor of the derived class AeolusSynthesizer for definition of this value
     */
    int             _nplay;
    /**
     * Sampling frequency, needs to be in agreement with audio driver
     */
    unsigned int    _fsamp;
    /**
     * The number of samples to be filled into the audio buffer for output to the audio system.
     * This needs to obtained from the audio system driver (here, oboe) in order to be in
     * agreement with the requirements for audio output feeding. For definition of these values
     * see constructor of the derived
     * class AeolusSynthesizer
     */
    unsigned int    _fsize;
    bool            _bform;
    /**
     * Number of audio sections actually in use
     */
    int             _nasect;
    /**
     * Number of divisions actually in use. Depends on the definition file
     */
    int             _ndivis;
    /** Audio sections. The audiosections allow for spatialization of the sound coming form the
     * different divisions. Each division directs its output towards an audio section
     */
    Asection       *_asectp [NASECT];
    /**
     * Divisions of the Aeolus synthesizer. Each division correponds to a set of ranks. The ranks
     * are activated / de-activated individually, but their output is globally channelled to the
     * audio section associated with the division
     */
    Division       *_divisp [NDIVIS];
    /**
     * The reverb processor, acting on the output of the audio sections
     */
    Reverb          _reverb;
    /**
     * This class assumes that you provide, and through class inheritage, appropriately set the
     * output buffer _outbuf. The length of 8 is the maximum length of different buffers, but
     * in reality only up to _nplay buffers are used, and each one needs to be allocated
     * to an array of length _fsize. In this implementation, the
     * allocation of the output buffer is done in the derived class AeolusSynthesizer, and
     * more precisely in AeolusSynthesizer::allocateOutputBuffer()
     */
    float          *_outbuf [8]; // Maximum 8 samples per frame for spatial processing, usually only 1 (mono) or 2 (stereo) used
    unsigned char   _keymap [NNOTES]; //
    Fparm           _audiopar [4];
    float           _revsize;
    float           _revtime;




};


#endif

