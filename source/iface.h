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


#ifndef AEOLUS_IFACE_H
#define AEOLUS_IFACE_H


#include "../../clthreads/include/clthreads.h"
#include "messages.h"

/**
 * Virtual base class for the user interface thread. In derived classes, this can implement any type of
 * user interface - graphical, command line, or like here, interaction with a user interface
 * interacting via jni calls, or possibly other interfaces
 */

class Iface : public A_thread
{
public:

    /** Constructor */
    Iface () : A_thread ("Iface") {}
    /** Destructor */
    virtual ~Iface () {}
    /** Stop the interface thread */
    virtual void stop () = 0;
    /** Is the Aeolus synthesizer initializing?
     * @return True if initializing, false otherwise
     */
    virtual bool isInitializing()=0;
    /** Get the number of divisions
     * @return Number of divisions making up the Aeolus synthesizer
     */
    virtual int get_n_divisions()=0;
    /**
     * Get the label for a given division
     * @param division_index The division for which we need the text label
     * @return Text label as string. The labels are defined in the instrument definition file
     */
    virtual const char* getLabelForDivision(int division_index)=0;
    /**
     * Terminate the application
     */
    void terminate () {  ITC_ctrl::put_event (EV_EXIT, 1); }


private:

    void thr_main () override = 0;
};



typedef Iface *iface_cr (int ac, char *av []);


#endif
