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
// ----------------------------------------------------------------------------


#ifndef AEOLUS_MODEL_H
#define AEOLUS_MODEL_H


#include "../../clthreads/include/clthreads.h"
#include "messages.h"
#include "lfqueue.h"
#include "addsynth.h"
#include "rankwave.h"
#include "global.h"

#ifndef MULTISTOP
# define MULTISTOP 1
#endif

class Asect
{
public:

    Asect () { *_label = 0; }

    char    _label [64];
};


class Rank
{
public:

    int         _count;
    Addsynth   *_sdef; // This is the definition of the rank, that is, its synthesis parameters
    Rankwave   *_wave;
};

 /**
  * The Divis class holds a set of ranks, which in turn correspond to a set of pipes that
  * make the same type of sound but at different notes (frequencies). There are also
  * some per-division properties
  */
class Divis
{
public:

    enum { HAS_SWELL = 1, HAS_TREM = 2, NRANK = 32 };
    enum { SWELL, TFREQ, TMODD, NPARAM };

    Divis ();

    char        _label [16]; // The label of the division
    /** Division flags indicating division configuration:
     * The least significant bit indicates whether the division has a swell (volume variation): binary and with HAS_SWELL
     * The second least significant bit indicates whether the division has a tremulant: binary and with HAS_TREM
     */
    int         _flags;
    int         _dmask; // Default division mask indicating to which keyboard(s) the ranks of the division should react
    int         _nrank; // actual number of ranks
    int         _asect; // index of the audio section associated with the division
    int         _keybd; // keyboard nominally associated with the keyboards
    Fparm       _param [NPARAM]; // division common parameters (SWELL, TFEQ, TMODD, NPARAM) listed above
    Rank        _ranks [NRANK]; //N division ranks
};

// Keyboard for playing the division
class Keybd
{
public:

    enum { IS_PEDAL = 256 };

    Keybd ();

    char    _label [16]; // The label of the division
    int     _flags; // keyboard binary flags: is it a pedal or hand keyboard (and with IS_PEDAL)
};

/**
 * Interface elements visible to the user. Ifelm objects of type
 * DIVRANK correspond to "stops", which are either individual ranks or pre-defined
 * mixtures of ranks
 */


/**
 * User interface element
 */
class Ifelm
{
public:
    typedef enum
    { DIVRANK, KBDRANK, COUPLER, TREMUL } Ifelm_type;


    Ifelm ();

    char      _label [32]; // Label of the interface element
    char      _mnemo [8]; // short label
    int       _type; // Type: See Ifelm_type
    int       _keybd;
    int       _state; // on or off
#if MULTISTOP
    uint32_t  _action[2][8];
    uint32_t& _action0;
    uint32_t& _action1;
#else
    uint32_t  _action0;
    uint32_t  _action1;
#endif
};

/**
 * User interface groups, including stops, but also couplers (to use several groups) and others
 */
class Group
{
public:

    enum { NIFELM = 32 };

    Group ();

    char     _label [16]; // label of the group
    int      _nifelm; // actual number of interface elements
    Ifelm    _ifelms [NIFELM]; // interface elements, maximum NIFELM
};

/**
 * Midi channel configuration preset. This is for the upper bits, the actual midi to keyboard mapping is
 * to be set with AeolusAudio::setMidiMapBit
 */
class Chconf
{
public:

    Chconf () { memset (_bits, 0, 16 * sizeof (uint16_t)); }

    uint16_t  _bits [16]; // bit map, for application see set_mconf
};

/**
 * User interface preset. A preset consists of a uint32 (32-bit) mask for every group in the user
 * interface, the bits in the masking indicating the on/off state of each user interface element in order
 */
class Preset
{
public:

    Preset () { memset (_bits, 0, NGROUP * sizeof (uint32_t)); }

    uint32_t  _bits [NGROUP]; // The 32-bit masks indicating the state of the  user interface elements (on/off)
};

    

class Model : public A_thread
{
public:

    Model (Lfq_u32      *qcomm,
           Lfq_u8       *qmidi,
	   uint16_t     *midimap,
           const char   *appname,
           const char   *stops,
           const char   *instr,
           const char   *waves,
           bool          uhome);

    virtual ~Model ();
   
    void terminate () {  put_event (EV_EXIT, 1); }

    [[maybe_unused]] Group* getGroupWithLabel(const char* theLabel);

    [[maybe_unused]] int get_n_tunings();

    [[maybe_unused]] int getCurrentTuning();

    [[maybe_unused]] const char* getTuningLabel(int index_tuning);

    [[maybe_unused]] float getBaseFrequency();

    [[maybe_unused]] bool is_retuning();





private:
    /** Main thread loop for the model. This consists in processing the incoming messages from other threads,
     * and termination upon receiving EV_Exit
     */
    void thr_main () override;
    /** Save the wavetables of the ranks (via slave thread) */
    void save_ranks();
    /**
     * Init the instrument. For this the instrument definition file is read and the presets loaded from the preset file
     */
    void init ();
    /** Close the model. For this, the presets are written to file */
    void fini ();
    /** Process ITC inter-thread messages in the queue for the model thread */
    void proc_mesg (ITC_mesg *M);
    /** Process the incoming midi queue. Only control messages, not note messages, are processed. The note
     * messages are processed in the Aoelus_audio thread.*/
    void proc_qmidi ();
    /** Init audio synthesis by transmitting the division information to the audio thread. Use
     * this only once the audio system has been initialized, that is, once M_audio_info message has been received<br />
     * For the detailed startup sequence, see AeolusSynthesizer::AeolusSynthesizer
     */
    void init_audio ();
    /** Init the user interface thread by transmitting division, group, instrument and temperament information to the
     * user interface. Also, load the default preset (preset 0 for bank 0)
     */
    void init_iface ();
    /**
     * Calculate or load ranks
     * @param comm MT_CALC_RANK for calculating the new ranks or MT_LOAD_RANK for loading the ranks from their wavetable file
     */
    void init_ranks (int comm);
    /** Process a given rank. Processing means either to (re-)calculate for comm=MT_CALC_RANK or
     * to load from a file for comm=MT_LOAD_RANK
     * @param g User interface group
     * @param i Index of rank in user interface group
     * @param comm MT_CALC_RANK for calculating the new rank or MT_LOAD_RANK for loading the rank from the wavetable file
     */
    void proc_rank (int g, int i, int comm);
    /**
     * Set the on-off state of a user interface element. This is also triggers the actions
     * configured for the actions of switching on or off for the interface element (if any)
     * @param g The group index of the interface element
     * @param i The index of the interface element within the groupè
     * @param m 0 for off, 1 for on, 2 for toggling (off to on respectively on to off)
     */
    void set_ifelm (int g, int i, int m);
    /**
     * Clear group in user interface, that is, switch all user interface element in the group off, along with
     * triggering the actions associated with switching off the interface element (if not already off)
     * @param g The interface groiup for which the user interface element should be switched off
     */
    void clr_group (int g);
    /**
    * Set audio parameter
    * @param s Source id (to indicate source of change in user interface, not used at present)
    * @param a index of the audio section for which the parameter should be modified. a<0 indicates global instrument settings
    * @param p index of the parameter with the audio section (or global instrument parameters)
    * @param v value of the paramter to be set (min and max values will be adjusted to encompass the new value)
 */
    void set_aupar (int s, int a, int p, float v);
    void set_dipar (int s, int d, int p, float v);
    /**
 * Set midi configuration. This accesses the upper byte of the midimap entries, which are configuration entries.<br /><br />
  * The lower byte is set to zero (except for when the lowest 3 bits are simultaneously 1 as a shortcut for
  * routing midi channels to all keyboards). <br />For setting the lower byte, which codes for the actual
  * midi-to-keyboard mapping, do not use mconf. Instead,
  * use AeolusAudio::setMidiMapBit, which in this implementation is called via the Android java AeolussynthManager.setMidiMapping
 * function and jni mapping in AeolusSynth_jni_functions-
 * @param i Not used here, but transmitted to the user interface. Maybe this is intended to be a preset index??
 * @param d Pointer to beginning of 16 element array of type uint16, there need to be 16 elements in the underlying array
 */
    void set_mconf (int i, uint16_t *d);
    /**
     * Get the user inteface state as binary mask. This is to generate and store a preset as binary mask
     * @param bits. Pointer to the start of a pre-allocated uint32 array to hold the masks for each use interface group.
     *   There is one 32-bit mask per user interface group, which contains bit-wise the on/off state of the user
     *   interface element.
     */
    void get_state (uint32_t *bits);
    /**
     * Select a preset. The functions gets the desired preset from its internal preset store and sets the
     * user interface elements correspondingly. Attention, does not notify the user interface class of the change
     * @param bank The preset bank from which to select the preset
     * @param pres The preset in the bank to select
     */
    void set_state (int bank, int pres);
    /** Turn off activated midi channels<br />
     * This function runs through all the midi notes and turns off the activated ones if they are on the
     * channels selected by the binary mask. This ultimately operates on the keymap of the Aeolus Audio thread.
     * @param mask Binary mask indicating the midi channels to turn off (if presently on; otherwise they are
     *             left in their off state).
     */
    void midi_off (int mask);
    /**
     * Change tuning to new base frequency and new temperament
     * @param freq The new base frequency
     * @param temp The new selection of temperament
     */
    void retune (float freq, int temp);
    /**
     * Recalculate rank (that is, regenerate the wavetables)
     * @param g User interface group
     * @param i Index of the rank in the user interface group
     */
    void recalc (int g, int i);
    /**
     * Save model state: ranks, instrument state, presets
     */
    void save ();
    /**
     * Find the rank associated with a given interface element, if there is one
     * @param g User interface group
     * @param i User interface element index within group
     * @return Pointer to the rank, null if no rank associated with the user interface element
     */
    Rank *find_rank (int g, int i);
    /**
     * Read the instrument definition file to generate divisions, keyboards, tunings and groups
     * @return Success status of the read operation
     */
    int  read_instr ();
    /** Write instrument definition file corresponding to the current configuration in terms
     * of divisions, keyboards, tunings and groups
     * @return success status of the file write operation
     */
    int  write_instr ();
    /**
     * Populate the bits array provided with the desired preset in the given bank<br />
     * A preset is a uint32 array, each element coding a bitwise mask for a user interface group
     * @param bank bank id
     * @param pres preset within bank
     * @param bits Pre-allocated array, to be safe, of length NGROUP
     * @return number of bitmasks returned, corresponding to the actual number of groups (_ngroup<=NGROUP) or
     *         zero if the preset or bank index or outside the allowed range
     */
    int  get_preset (int bank, int pres, uint32_t *bits);
    /**
     * Set the preset bits for a given bank and preset index.<br />
     * A preset is a uint32 array, each element coding a bitwise mask for a user interface group
     * @param bank bank id
     * @param pres preset within bank
     * @param bits Pointer to the beginning of a uint32 array for the group bit masks.
     */
    void set_preset (int bank, int pres, uint32_t *bits);
    /**
     * Insert a new preset in a bank. This will insert the preset at position pres and move
     * up the existing presets above pres, for as long as slots are still available.
     * @param bank The bank to which the preset should be added
     * @param pres Preset insertion position
     * @param bits Pointer to the beginning of the array for the preset bit mask
     */
    void ins_preset (int bank, int pres, uint32_t *bits);
    /**
     * Delete preset in a bank at position pres. The presets above the pres are moved down by 1 in the list
     * @param bank The bank
     * @param pres The preset position to delete
     */
    void del_preset (int bank, int pres);
    /**
     * Load the presets from a preset file
     * @return Success status: 0 on success, 1 on error
     */
    int  read_presets ();
    /**
     * Write the presets from current memory state to file
     * @return Success status: 0 on success, 1 on error
     */
    int  write_presets ();

    Lfq_u32        *_qcomm; // inter-process general message communication (model to audio thread)
    Lfq_u8         *_qmidi; // inter-process midi message communication (midi to audio thread)
    uint16_t       *_midimap; // maps midi channels to keyboards
    const char     *_appname; // internall application name
    const char     *_stops; // path to the directory holding the stops
    char            _instr [1024]; // path to the instrument definition and presets directory
    char            _waves [1024]; // path to the wavetable file storage location
    bool            _uhome; // use user's home?
    bool            _ready; // is everything loaded?
    bool            _isRetuning;  // true during the retuning process

    Asect           _asect [NASECT]; // Array of audio sections, attention initialized only to _nasect
    Keybd           _keybd [NKEYBD]; // Array of keyboards, attention initialized only to _nkeybd
    Divis           _divis [NDIVIS]; // Array of divisions, attention initialized only to _ndivis
    Group           _group [NGROUP]; // Array of user interface groups, attention only initialized to _ngroup

    int             _nasect; // actual number of audio sections used
    int             _ndivis; // actual number of division defined
    int             _nkeybd; // actual number of keyboards defined
    int             _ngroup; // actual number of user interface groups defined
    float           _fbase; // Current base frequency for tuing
    int             _itemp; // The current temperament
    int             _count;
    int             _bank; // Current preset bank
    int             _pres; // Currently selected preset
    int             _client; // midi client for midi over IP, not used at present
    int             _portid; // portid for midi over IP, not used at present
    int             _sc_cmode; // stop control command mode
    int             _sc_group; // stop control group number
    Chconf          _chconf [8]; // midi channel configurations (128-bit and above)
    Preset         *_preset [NBANK][NPRES]; // Presets, organized in banks
    M_audio_info   *_audio; // Access to information about the audio synthesis system
    M_midi_info    *_midi; // Access to information about the midi receiving system
};


#endif

