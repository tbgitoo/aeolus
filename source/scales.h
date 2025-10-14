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


#ifndef AEOLUS_SCALES_H
#define AEOLUS_SCALES_H

/**
 * Tuning temperament. The data is a an array with 12
 * members that indicates the frequency multiplier from
 * the lower base note (do) to each of the half-tones
 */
struct temper
{
    const char *_label; // label of the temperament
    const char *_mnemo; // short label
    float      *_data; // 12-membered array with the frequency ratio relative to base-note
};


#define NSCALES 11 // fixed number of scales

extern struct temper scales [NSCALES]; // predefined scales
  

#endif

