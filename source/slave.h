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


#ifndef AEOLUS_SLAVE_H
#define AEOLUS_SLAVE_H


#include "../../clthreads/include/clthreads.h"
#include "messages.h"

/**
 * Class for separate slave thread for
 * tasks that take a long time: calculating, saving and loading the ranks
 * After starting this thread, it runs a loop in thr_main and waits for cltrhead messages
 * by which it gets instructed to do the rank calculation, saving and loading tasks
 */
class Slave : public A_thread
{
public:
    /**
     * Constructur
     */
    Slave () : A_thread ("Slave") {}
    /**
     * Destructor
     */
    virtual ~Slave () {}
    /**
     * Terminate, including sending the termination message to all other threads
     */
    void terminate () {  put_event (EV_EXIT, 1); }

private:
    /**
     * Main thread routing. This is the looping routing waiting for the messages
     * to be handled
     */
     void thr_main () override;
};


#endif
