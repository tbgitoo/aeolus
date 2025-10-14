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


#include <cmath>
#include <cstring>
#include <android/log.h>
#include "division.h"


Division::Division (Asection *asect, float fsam) :
    _asect (asect),
    _nrank (0),
    _dmask (0),
    _trem (0),
    _fsam (fsam),
    _swel (1.0f),
    _gain (0.1f),
    _w (0.0f),
    _c (1.0f),
    _s (0.0f),
    _m (0.0f)
{
    for (int i = 0; i < NRANKS; i++) _ranks [i] = 0;
}


Division::~Division ()
{
}


void Division::process ()
{
    int    i;
    float  d, g, t;
    float  *p, *q; 

    memset (_buff, 0, NCHANN * PERIOD * sizeof (float));

    for (i = 0; i < _nrank; i++) _ranks [i]->play (1);

    g = _swel;
    if (_trem)
    {
	_s += _w * _c;
	_c -= _w * _s;
        t = sqrtf (_c * _c + _s * _s);
        _c /= t;
        _s /= t;
        if ((_trem == 2) && (fabsf (_s) < 0.05f))
        {
	    _trem = 0;
            _c = 1;
            _s = 0;
	}
        g *= 1.0f + _m * _s;
    }

    t = 1.05f * _gain;
    if (g > t) g = t;
    t = 0.95f * _gain;
    if (g < t) g = t;

    d = (g - _gain) / PERIOD;    
    g = _gain;
    p = _buff;
    q = _asect->get_wptr ();

    for (i = 0; i < PERIOD; i++)
    {
        g += d; // G is the varying gain for swelling and tremulus modulation
        q [0 * PERIOD * MIXLEN] += p [0 * PERIOD] * g * _paramgain; // _paramgain is the fixed gain for this division
        q [1 * PERIOD * MIXLEN] += p [1 * PERIOD] * g * _paramgain;
        q [2 * PERIOD * MIXLEN] += p [2 * PERIOD] * g * _paramgain;
        q [3 * PERIOD * MIXLEN] += p [3 * PERIOD] * g * _paramgain;
        p++;
        q++;
    }
    _gain = g;
}


void Division::set_rank (int ind, Rankwave *W, int pan, int del)
{
    Rankwave *C;

    C = _ranks [ind];
    if (C)
    {
        W->_nmask = C->_cmask;
        delete C;
    }
    else W->_nmask = 0;
    W->_cmask = 0;
    _ranks [ind] = W;
    del = (int)(1e-3f *(float) del * _fsam / PERIOD);
    if (del > 31) del = 31;
    W->set_param (_buff, del, pan);
    if (_nrank < ++ind) _nrank = ind;
}


void Division::update (int note, int mask)
{
    int             r;
    Rankwave       *W;

    for (r = 0; r < _nrank; r++)
    {
	W = _ranks [r];

        __android_log_print(android_LogPriority::ANDROID_LOG_INFO,
                            "Division::update",
                                            "1: c-mask %d n-mask %d ",W->_cmask, W->_nmask);

        if (W->_cmask & 127)
	{     
	    if (mask & W->_cmask) W->note_on (note + 36);
	    else                  W->note_off (note + 36);
	}
    }
}


void Division::update (unsigned char *keys)
{
    int            d, r, m, n, n0, n1;
    unsigned char  *k;
    Rankwave       *W;

    // run through the ranks for this division
    for (r = 0; r < _nrank; r++)
    {
	W = _ranks [r]; // get the rank
        // check with xor (^) whether there is a difference in the lower byte between the rank's c and n mask
        if ((W->_cmask ^ W->_nmask) & 127)
	{

        __android_log_print(android_LogPriority::ANDROID_LOG_INFO,
                            "Division::update",
                            "2: c-mask %d n-mask %d ",W->_cmask, W->_nmask);

            // m is the 7 lowest bits of the ranks n-mask
            m = W->_nmask & 127;               
            if (m) // the rank as a non-zero n-mask
	    {            
		n0 = W->n0 (); // lowest midi note in the ranks range
		n1 = W->n1 (); // highest midi note in the ranks range
                k = keys; // recover a pointer k to the first byte of the keys array
                d = n0 - 36; // in Aeolus, the lowest note should be 36
                if (d > 0) k += d; // if in this rank, we start higher, move up in the keys
                for (n = n0; n <= n1; n++)  // go through all the notes
	        {
                    if (*k++ & m) W->note_on (n); // if the current byte has bits aligned with ranks mask, play it
                    else          W->note_off (n); // otherwise, switch it off
		}
	    }
            else W->all_off (); // for simplicity, if there is no bit set in the ranks mask, switch it off completely
	}
        W->_cmask = W->_nmask; // update the mask: the c-mask is set equal to the n-mask
    }
}


void Division::set_div_mask (int bits)
{
    int       r;
    Rankwave *W;

    bits &= 127;
    _dmask |= bits;
    for (r = 0; r < _nrank; r++)
    {
	W = _ranks [r];
        if (W->_nmask & 128) W->_nmask |= bits;
    } 
}


void Division::clr_div_mask (int bits)
{
    int       r;
    Rankwave *W;

    bits &= 127;
    _dmask &= ~bits;
    for (r = 0; r < _nrank; r++)
    {
	W = _ranks [r];
        if (W->_nmask & 128) W->_nmask &= ~bits;
    } 
}


void Division::set_rank_mask (int ind, int bits)
{
    Rankwave *W = _ranks [ind];

    if (bits == 128) bits |= _dmask;
    W->_nmask |= bits;
}


void Division::clr_rank_mask (int ind, int bits)
{
    Rankwave *W = _ranks [ind];

    if (bits == 128) bits |= _dmask;
    W->_nmask &= ~bits;
}

void Division::setParamGain(float division_volume_gain) {
   _paramgain=abs(division_volume_gain);
}

float Division::getParamGain() {
    return _paramgain;
}
