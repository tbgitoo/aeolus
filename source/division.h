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
// Changes by Thomas Braschler / Mathis Braschler (2025): More detailed comments
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------


#ifndef AEOLUS_DIVISION_H
#define AEOLUS_DIVISION_H


#include "asection.h"
#include "rankwave.h"


/**
 * Set of rankwaves i.e. set of organ voices associated with an audio section
 */
class Division
{
public:
    /**
     * Constructor for a division
     * @param asect Pointer to audio section associated with this division
     * @param fsam Sampling frequency
     */
    Division (Asection *asect, float fsam);
    ~Division ();

    /**
     * Set the ind-th rankwave, if already set, replace ind-th rankwave
     * The rankwave is configured to use the common output buffer _buff
     * @param ind ind-th rank wave
     * @param W Rankwave (organ voice, register)
     * @param pan Audio panning (left, right or center)
     * @param del Time delta for sampling
     */
    void set_rank (int ind, Rankwave *W, int pan, int del);
    /**
     *
     * @param stat
     */
    void set_swell (float stat) { _swel = 0.2 + 0.8 * stat * stat; }
    void set_tfreq (float freq) { _w = 6.283184f * PERIOD * freq / _fsam; }
    void set_tmodd (float modd) { _m = modd; }
    /**
     * Set division mask in terms of keyboard played by default the ranks in this
     * division. It is possible to set the rank masks differently (via the detailed set_rank_mask function below), but
     * typically, 128 is passed to set_rank_mask such that the division mask is applied to the rank at hand.
     * @param bits The common bits imposed on the ranks, set by or-ing to the ranks with the 128 bit set (for synchronization with division)
     */
    void set_div_mask (int bits);
    /**
     * Clear the division mask for this division
     * @param bits The bits (keyboards) that shold be cleared from the division mask
     */
    void clr_div_mask (int bits);
    /** Set individual rank mask
     * The rank mask determines to which keyboards the rank responds, that is, for which keybaords the rank
     * will play incoming notes
     * @param ind Index of the rank being addressed
     * @param bits Bitwise indication of the keyboards to which the rank should respond
     */
    void set_rank_mask (int ind, int bits);
    /** Set individual rank mask
     * The rank mask determines to which keyboards the rank responds, that is, for which keybaords the rank
     * will play incoming notes
     * @param ind Index of the rank being addressed
     * @param bits Bitwise indication of the keyboards to which the rank should respond
     */
    void clr_rank_mask (int ind, int bits);
    /**
     * Switch the divisions tremulant effect on
     */
    void trem_on ()  { _trem = 1; }
    /**
     * Switch the divisions tremulant effect off
     */
    void trem_off () { _trem = 2; }
    /**
     * Is tremulant effect of this division currently on?
     * @return true if the tremulant is on, false if tremulant is off
     */
    bool tremulantIsOn() {if(_trem==1) return true; return false;}

    /** Set division volume
     * @param division_volume_gain Linear gain applied to sound signal from this division
     */
    void setParamGain(float division_volume_gain);
    /** Get division volume
    * @return Linear gain of the division
    */
    float getParamGain();
/**
 * Process the rankwave for this division
 * The output is directly transmitted to the audio section associated with this division via
 * internal pointer exchange
 */
    void process ();
    /**
     * Update whether the ranks are playing a given note. For this,
     * the note and a binary mask is provided. The note is given as the delta from midi note 36
     * (so for example note=2 means midi note 38). the binary mask corresponds to the keyboards on which
     * the note is played, which in this Android implementation is also the division<br /><br />
     * This function updates the on/off state of the ranks regarding the different notes, e.g., a note
     * is played if it is being played on at least one of the keyboards to which the rank responds. This function does
     * not change their rank masks, this task is performed first by the set_rank_mask and clr_rank_mask methods,
     * which change the ranks' _nmask field, and then by the update(unsigned char *keys) function that checks
     * for discrepancy between _nmask and _cmask field
     * @param note Midi note (above 36)
     * @param mask Mask for activation, with bits indicating keyboards (and as simplified implementation, divisions)
     */
    void update (int note, int mask);

    /** Update the ranks to coordinate note activation state upon incoming
 * activation or deactivation of ranks. Activation state change that has occurred recently (between the last audio
 * and this audio cycle, that is) will not be correctly reflected in the note activation. It is recognized by a
 * discrepancy between the incoming rank mask (_nmask, for new mask) and the currently applied mask (_cmask, for current mask)
 * If such a discrepancy exists, readjust the notes playing or not by going through the entire key mapping passed as parameter
 * @param keys Keyboard activation map, from midi note 36 on, each entry indiciating bit-wise the keyboards currently playing the note
 */
    void update (unsigned char *keys);

private:
   /** The audio section associated with this division */
    Asection  *_asect;
    /** The array of ranks in this division, dimensioned for a maximum of NRANKS */
    Rankwave  *_ranks [NRANKS];
    /** The actual number of ranks in this division */
    int        _nrank;
    /**
     * Division mask. This is the default mask defining the keyboards to which the ranks in this division should respond
     * when they are set as active
     */
    int        _dmask;
    /**
     * Flag indicating whether the Tremulant (small sound level variation) is on for this division
     */
    int        _trem;
    /**
     * Audio sampling rate
     */
    float      _fsam;
    float      _swel; // swell parameter
    float      _gain; // Actual gain, vaies with tremuli and other effects
    float      _paramgain=1.0f; // Parametric gain, applied to division as a constant mulitplicator
    float      _w; // Rate of tremulation
    float      _c; // internal variable for tremulant (cos part of the tremulant variation signal)
    float      _s; // internal variable for tremulant (sin part of the tremulant variation signal)
    float      _m; // magnitude of termulation
    /** Output buffer. At each audio cycle, this is initialized to 0,0,... .
     * The ranks (and within the pipes) then add their signal in turn. After modulation with the
     * tremulant modulation and swell gain, a pointer to the audio section (via the write pointer)
     * for spatial modulation
     */
    float      _buff [NCHANN * PERIOD];

};


#endif

