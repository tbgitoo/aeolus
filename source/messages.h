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


#ifndef AEOLUS_MESSAGES_H
#define AEOLUS_MESSAGES_H

#include "../../clthreads/include/clthreads.h"
#include <cstring>
#include "rankwave.h"
#include "asection.h"
#include "addsynth.h"
#include "global.h"

/*
 * Predefined threads to indicate origin and destination of the messages
 */
enum
{
    FM_SLAVE =  8,
    FM_IFACE =  9,
    FM_MODEL = 10,
    FM_IMIDI = 11,
    FM_AUDIO = 12,
    FM_TXTIP = 13,
    TO_SLAVE =  8,
    TO_IFACE =  9,
    TO_MODEL = 10,
    TO_IMIDI = 11,
    TO_AUDIO = 12,
    EV_RLINE = 0,
    EV_XWIN  = 16,
    EV_QMIDI = 24,
    EV_SYNC  = 30,
    EV_EXIT  = 31
};

/**
 * Enum to indicate message type
 */
enum
{
    MT_AUDIO_INFO,
    MT_AUDIO_SYNC,
    MT_MIDI_INFO,
    MT_NEW_DIVIS,
    MT_CALC_RANK,
    MT_LOAD_RANK,
    MT_SAVE_RANK,

    MT_IFC_INIT,
    MT_IFC_READY,
    MT_IFC_ELCLR, // must be in this order
    MT_IFC_ELSET, //
    MT_IFC_ELXOR, //
    MT_IFC_ELATT,
    MT_IFC_GRCLR,
    MT_IFC_AUPAR,
    MT_IFC_DIPAR,
    MT_IFC_RETUNE,
    MT_IFC_ANOFF,
    MT_IFC_MCSET,
    MT_IFC_MCGET,
    MT_IFC_PRRCL,
    MT_IFC_PRDEC,
    MT_IFC_PRINC,
    MT_IFC_PRSTO,
    MT_IFC_PRINS,    
    MT_IFC_PRDEL,
    MT_IFC_PRGET,
    MT_IFC_EDIT,
    MT_IFC_APPLY,
    MT_IFC_SAVE,
    MT_IFC_TXTIP,
    MT_IFC_RETUNING_DONE
};


#define SRC_GUI_DRAG  100
#define SRC_GUI_DONE  101
#define SRC_MIDI_PAR  200

/**
 * Message used to transmit the relevant parameters and objects after initialization of the audio thread
 * to the model thread. The message transmits instrument and individiual audio section parameters, Its ITC message type
 * is MT_AUDIO_INFO
 */
class M_audio_info : public ITC_mesg
{
public:

    M_audio_info () : ITC_mesg (MT_AUDIO_INFO) {}

    float           _fsamp; // sampling frequency
    int             _fsize; // audio buffer size
    int             _nasect; // number of audio section
    Fparm          *_instrpar; // pointer to instrument parameters
    Fparm          *_asectpar [NASECT]; // array of pointers to the beginning of the audio section parameters

    /** Create a copy of a message
     * @param original The original message
     */
    static M_audio_info* createCopy(M_audio_info* original);
};

/**
 * Message used to transmit the relevant parameters initialization of the midi thread
 * to the model thread. The message transmits midi client and IP info (not used in this Android
 * implementation), and a pointer to beginning of the midimap array, which is an array of bitmasks
 * linking the midi channels to the keyboards of the Aeolus synthesizer. The ITC message type
 * is MT_MIDI_INFO
 */
class M_midi_info : public ITC_mesg
{
public:

    M_midi_info ();

    int       _client{}; // Client if midi over IP is used
    int       _ipport{}; // IP port if midi over IP is used
    uint16_t*  _chbits; // pointer to the midimap
    /** Create a copy of a message
     * @param original The original message
     */
    static M_midi_info* createCopy(M_midi_info* original);
};

/**
 * Message used by the model to transmit the information about the divisions to the audio thread
 * This message is of ITC message type MT_NEW_DIVIS
 *
 */
class M_new_divis : public ITC_mesg
{
public:

    M_new_divis () : ITC_mesg (MT_NEW_DIVIS) {}

    int             _flags;
    int             _dmask; // division keyboard mask
    int             _asect;
    float           _swell; // swell parameter
    float           _tfreq;
    float           _tmodd;
};

/**
 * Generate and then integrate a new rank. This message is used in a cascade between model, slave
 * and Aeolus audio thread.<br /><br />
 * In detail, the M_def_rank message is initiated by the model thread when the need for (re-)loading ranks is identified.
 * It is sent to the slave for rank loading. Once the ranks generated
 * (with wavetables calculatated or loaded from file),
 * the message is completed with the newly loaded ranks and is transmitted to the Aeolus audio thread
 * for integration into the divisions by the M_def_rank
 * message. This message is either of type MT_CALC_RANK for calculating the new rank or
 * MT_LOAD_RANK for loading the rank from a file
 */
class M_def_rank : public ITC_mesg
{
public:

    M_def_rank (int type) : ITC_mesg (type) {}

    int             _divis; // The division to which the rank belong
    int             _rank; // The id within the division
    int             _group; // The user interface group to which the rank belong (typically equal to the division)
    int             _ifelm; // Interface element id within the user interface group
    float           _fsamp; // sampling rate
    float           _fbase; // base frequency for the tuning of the rank
    float          *_scale; // Pointer to the tuning scale
    Addsynth       *_sdef; // Used to transmit some parameters for the rank
    Rankwave       *_wave; // Pointer to the rank
    const char     *_path; // path for the wavetable
};

/** Message for initialization of the user interface<br />
 * This message is generated by the model and transmits the information needed by the user
 * thread for generation of the user interface. In the Android implementation used here,
 * the native user interface thread has an internal representation of the user interface,
 * but does not display it. Rather, the user interface is displayed by the Android part; as the
 * Android part queries the native part for the structure of the user interface, it is nevertheless
 * the content of this message that dictates the user interface structure. Also, the
 * native representation of the user interface is persistent while the Aeolus application
 * runs, while each Android Activity
 * regenerates its user interface when loaded, based on the persistent native information.
 */
class M_ifc_init : public ITC_mesg
{
public:

    M_ifc_init () : ITC_mesg (MT_IFC_INIT) {}

    /** Path to the stops directory.<br />
     * Here, absolute path to the directory holding the stops as installed by the Android app
     * The stops are stored as .ae0 files in the stops directory.
     */
    const char         *_stops;
    /** Path to the waves directory.<br />
     * Here, absolute path to the directory holding the wavetables as generated and saved upon loading the ranks
     * The waves (aka, wavetables) are stored as  .ae1 files in the waves directory.
     */
    const char         *_waves;

    /** Path to the instrument definition directory.<br />
     * Here, absolute path to the directory holding the instrument definition file as installed by the Android app <br /><br />
     * The instrument directory holds the instrument definition file ("definition" file) and the "presets" file which
     * jointly define the layout (from the definition file) and startup state of the Aeolus synthesizer
     */
    const char         *_instr;
    /**
     * Name of the app
     */
    const char         *_appid;
    /** Connected midi client; not used in this implementatino */
    int                 _client;
    /** Midi over IP port; not used here */
    int                 _ipport;
    /** Number of audio sections; with the default definition file, one audio section per division */
    int                 _nasect;
    /** Number of keyboards; with the default definition file, one keyboard per division */
    int                 _nkeybd;
    /** Number of division */
    int                 _ndivis;
    /** Number of user interface groups; with the default definition file, one group per division */
    int                 _ngroup;
    /** Number of preconfigured temperaments */
    int                 _ntempe;

    struct
    {
	const char     *_label; // Graphical label of the keyboard
        int             _flags;  // Possibility to set flags, not used in this implementation
    }                   _keybdd [NKEYBD];  // array of keyboard definitions
    struct 
    {
        const char     *_label; // label of the division
        int             _asect; // index of the audio section associated with this division
        int             _flags; // Possibility to set flags, not used in this implementation
    }                   _divisd [NDIVIS]; // division definition
    struct 
    {
        const char     *_label; // group lable
	int             _nifelm; // number of interface elements in this group
	struct
	{
            const char *_label; // interface element label
            const char *_mnemo; // interace element label short
            int         _type;
	}               _ifelmd [32]; // interface element definition
    }                   _groupd [8];  // definition of the groups
    struct 
    {
        const char     *_label; // tuning label
        const char     *_mnemo; // tuning label short
    }                   _temped [16]; // definition of the temperaments (tunings)

    static M_ifc_init* createCopy(M_ifc_init* original);
};


/** This message indicates that the on/off state of an interface element or an interface element attribute
 * have changed. This message is typically sent to the user interface by the model, but it can also be
 * sent other threads to the model for relay to the user interface. ITC message types are
 * MT_IFC_ELXOR (toggle element on/off), MT_IF_ELCLR (clear element, that is, turn it off),
 * MT_IFC_GRCLR (clear entire group), MT_IFC_ELATT (element attributes have changed) or MT_IFC_ELSET
 * (turn element on)
 */

class M_ifc_ifelm : public ITC_mesg
{
public:
    /**
     * Constructor
     * @param type ITC message type, one of MT_IFC_ELXOR , MT_IF_ELCLR ,
     *             MT_IFC_GRCLR , MT_IFC_ELATT,  MT_IFC_ELSET
     * @param g Group of the element to update
     * @param i Index within the group of the element to update
     */
    M_ifc_ifelm (int type, int g, int i) :
        ITC_mesg (type),
        _group (g),
        _ifelm (i)
    {}

    int      _group; // the group of the interface element
    int      _ifelm; // the index of the element

};

/**
 * Message for seting an audio parameter. Among others used by the model thread to transmit audio parameter settings
 * from the model to the user interface. ITC message type MT_IFC_AUPAR
 */
class M_ifc_aupar : public ITC_mesg
{
public:

    M_ifc_aupar (int s, int a, int p, float v) :
        ITC_mesg (MT_IFC_AUPAR),
        _srcid (s),
        _asect (a),
        _parid (p),
        _value (v)
    {}

    int    _srcid; // possibility to indicate the type of event that caused the change, for example drag in the user interface
    int    _asect; // audio section concerned by the change, -1 for global instrument parameter
    /**
     * For the global instrument parameters, the parameter ids are: 0=Volume, 1=reverb room size, 2= reverb time,
     * 3=position (front back)
     */
    int    _parid;
    /**
     * Value for the audio parameter
     */
    float  _value;
};

/** Message for setting a division <br />
 * Used among others by the model to indicate an update for a division parameter. ITC message type MT_IFC_DIPAR
 */
class M_ifc_dipar : public ITC_mesg
{
public:

    M_ifc_dipar (int s, int d, int p, float v) :
        ITC_mesg (MT_IFC_DIPAR),
        _srcid (s),
        _divis (d),
        _parid (p),
        _value (v)
    {}

    int    _srcid; // possibility to indicate the type of event that caused the change, for example drag in the user interface
    int    _divis; // The division to modify
    int    _parid; // id of the param: 0 is swelling amplitude, 1 is tremulation frequency, 2 is tremulation amplitude
    float  _value; // value of the parameter. In accordance with midi values, from 0 (minimum) to 127 (maximum)
};

/** Message indicating that the synthesizer should be retuned.<br />
 * This message is triggered through jni when the user requests a retuning in Android, and
 * causes AeolusSynthesizer to post a M_ifc_retune message to the model. In response, the model mandates the
 * slave to do the retuning
 */
class M_ifc_retune : public ITC_mesg
{
public:

    M_ifc_retune (float f, int t) : 
        ITC_mesg (MT_IFC_RETUNE),
        _freq (f),
        _temp (t)
    {}

    float  _freq; // new base frequency
    int    _temp; // new tuning temperament
};

/**
 * All notes off, send this to model to stop all notes from playing (as in panic to remove stuck midi notes)
 * The ITC message type is MT_IFC_ANOFF
 */
class M_ifc_anoff : public ITC_mesg
{
public:

    M_ifc_anoff (int bits) :
        ITC_mesg (MT_IFC_ANOFF),
        _bits (bits)
    {
    }

    int  _bits; // keyboards matching this bit mask will be switched off
};

/** Set the active midi-to-keyboard mapping (midi map).<br />
 * This message can be of type MT_IFC_MCSET (instructing the model to set the midi
 * channel mapping according to the incoming message), or MT_IFC_MCGET, in which case the
 * model applies its own channel settings to the midimap in use.
 */
class M_ifc_chconf : public ITC_mesg
{
public:

    M_ifc_chconf (int type, int index, uint16_t *bits) :
        ITC_mesg (type),
        _index (index)
    {
	if (bits) memcpy (_bits, bits, 16 * sizeof (uint16_t));
        else      memset (_bits, 0, 16 * sizeof (uint16_t)); 
    }

    int       _index; // The midi channel preset, a priori, only preset 0 is used here
    uint16_t  _bits [16]; // The midi map for the preset, 16 input midimaps mapped to the keyboards via a bit map
};

/** Message for transmitting a preset
 * A preset is the state of the interface elements in each group. This
 * is represented by uint32 array, 1 element per group, and each 32-bit element coding for
 * on/off state of at most 32 interface elements
 */
class M_ifc_preset : public ITC_mesg
{
public:

    M_ifc_preset (int type, int bank, int pres, int stat, uint32_t *bits) :
        ITC_mesg (type),
        _bank (bank),
        _pres (pres),
        _stat (stat)
    {
	if (bits) memcpy (_bits, bits, NGROUP * sizeof (uint32_t));
        else      memset (_bits, 0, NGROUP * sizeof (uint32_t));
    }

    int       _bank; // midi program bank
    int       _pres; // preset index in bank
    int       _stat;
    uint32_t  _bits [NGROUP]; // For each group, the state of maximum 32 interface elements as =0/1 bitmask
};


/**
 * Message to indicate that a rank should be edited
 */
class M_ifc_edit : public ITC_mesg
{
public:

    M_ifc_edit (int type, int group, int ifelm, Addsynth *synth) :
        ITC_mesg (type),
        _group (group),
        _ifelm (ifelm),
        _synth (synth)
    {}

    int        _group; // the user interface group in which the rank should be edited
    int        _ifelm; // the rank (index in the group)
    Addsynth  *_synth; // pointer to the additive synth, which holds the parameters to be edited
};

/**
 * Send a command line for interpretation by the text user inteface
 */
class M_ifc_txtip : public ITC_mesg
{
public:

    M_ifc_txtip () :
        ITC_mesg (MT_IFC_TXTIP),
        _line (nullptr)
    {}

    char  *_line; // The command line
};


#endif
 
