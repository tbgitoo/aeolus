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


#ifndef AEOLUS_IMIDI_H
#define AEOLUS_IMIDI_H


#include <cstdlib>
#include <cstdio>
#include "../../clthreads/include/clthreads.h"
#include "lfqueue.h"
#include "messages.h"

enum {
    SND_SEQ_EVENT_NOTEON = 1,
    SND_SEQ_EVENT_NOTEOFF,
    SND_SEQ_EVENT_CONTROLLER,
    SND_SEQ_EVENT_PGMCHANGE,
    SND_SEQ_EVENT_NONE
};

/**
 * Base class for the midi thread. The basic role of the midi thread is to convert incoming
 * midi messages to note queue messages. This among others involves applying the midimapping,
 * i.e. associating the midi messages to the appplicable preconfigured keyboards
 */
class Imidi : public ITC_ctrl
{
public:
    /** Constructor with pointers to the queues and midimaps */
    Imidi (Lfq_u32 *qnote, Lfq_u8 *qmidi, uint16_t *midimap, const char *appname);
    ~Imidi () override;
    /**
     * Terminate the application
     */
    void terminate ();
    /** Open the midi layer */
    void open_midi ();
    /** Close the midi layer */
    void close_midi ();
    /** predefined midi event structure */
    struct MidiEvent
    {
        int type;
        union {
            struct { int channel, note, velocity; } note;
            struct { int channel, param, value; } control;
        };
    };
    /** Handle midi event. This applieds the midimap lookup (midi channels to keyboards) and
     * routes control messages to the qmidi queue and note messages to the note queue
     * @param ev The midi event to be handled by the synthesizer
     */
    void proc_midi_event(const MidiEvent& ev);

    
protected:



    virtual void on_open_midi() = 0;
    virtual void on_close_midi() = 0;
    virtual void on_terminate() = 0;

protected:
    const char     *_appname;
    int             _client;
    int             _ipport;

private:
    Lfq_u32        *_qnote;
    Lfq_u8         *_qmidi; 
    uint16_t       *_midimap;
};


#endif
