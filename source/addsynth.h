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


#ifndef AEOLUS_ADDSYNTH_H
#define AEOLUS_ADDSYNTH_H


#include <cstdio>
#include <cstdint>


#define N_NOTE 11
#define N_HARM 64
#define NOTE_MIN 36
#define NOTE_MAX 96


// Nota bene: The additive synthesizer needs to be populated by readon from an Aeolus "stop" file


/**
 * Interpolation functions from a set of N support values
 * applicable to the N_NOTE=11 base notes of the octave
 * or alternatively for interpolation across the targetted keyboard range with support notes
 * at midi 0 to 60; in Aeolus, used at midi notes after substraction of 36 as an offset and so
 * with explicit support points at midi notes 36, 42, 48, 54, 60, 66, 72, 78, 84, 90, 96
 * The class also maintains a bit-mask value for indicating which of the 11 notes have been
 * set (0-indexing based): 0b00000000001 = index 0 value set; 0b00000000010 = index 1 value set
 * 0b00000000010 = index 0 and 1 set, and so forth
 */


class N_func
{
public:

    N_func ();
    /**
     * Reset to constant float value
     * Also resets bit-mask to 0b00000010000
     * @param v Constant value for the 11 notes to be set
     */
    void reset (float v);
    /**
     * Set float value for i-th note, and register in bit mask
     * \n\n
     * This function is particular in the sense that it extends / interpolates the value
     * set as a function of the values already set before. The algorithm is as follows:
     * \n If none of the values below index i were set, set all float values from index 0 to i-1
     * to the float value v. If some some value below index i is explicitly set (meaning the
     * corresponding bit in the bitmask is set to 1), interpolate between.
     * \n Same approach homologously above i
     * \n In any case, the bit mask is only modified in the sense to indicate that the ith value
     * has been explicitly set
     * @param i I-th note to be set
     * @param v Vale for the i-th note be set, with interpolation / extension as indicated by the
     * algorithm described above
     */
    void setv (int i, float v);
    /**
     * Clear explicity value at index i
     * \n\n
     * This function clears the value set at index i. As a caveat, it does nothing if
     * either the value set at i is the only value explicitly set, or if it was
     * already not set. Otherwise, it clears the value at i and replaced the values to
     * explicity set as per the bit mask with interpolated values from the explicity set values
     * @param i
     */
    void clrv (int i);
    /**
     * Get the value stored at present for note i
     * @param i Note index i
     * @return Float value
     */
    [[nodiscard]] float vs (int i) const { return _v [i]; }
    /**
     * Has value been set for index i
     * @param i Index to be queried
     * @return 1 (true) if the value is explicitly set, 0 otherwise
     */
    [[nodiscard]] int   st (int i) const { return (_b & (1 << i)) ? 1 : 0; }
    /**
     * Get interpolated value.
     * \n\n
     * The parameter n is divided by 6 and then
     * the float values are interpolated linearly between the 11 individual basic values
     * @param n Midi note value to be interpolated (0-60 for interpolation, linear
     * extrapolation outside this range)
     * @return Interpolated float value
     */
    [[nodiscard]] float vi (int n) const // value interpolated, index scaled by factor 6
    {
        int   i = n / 6; // interpolation point
        int   k = n - 6 * i; // offset from interpolation point
        float v = _v [i]; // value at interpolation point
        if (k) v += k * (_v [i + 1] - v) / 6; // linear interpolation
        return v;
    }
    /**
     * Write binary mask and support points to file
     * @param F pointer to file open for writing
     */
    void write (FILE *F);
    /**
     * Read binary mask and support points from file
     * @param F pointer to file open for reading
     */
    void read (FILE *F);

private:

    /** Bitmask for indicating which support points have already been set explicity
     *  This is 0-based bitmask: 0b00000000001 = index 0 value set; 0b00000000010 = index 1 value set
     *  0b00000000010 = index 0 and 1 set, and so forth
     */

    int   _b;
    /** Note values. If used directly, via the vs function, these are the values at the 11 half-tone notes
     * across the octave. If used with with vi function, linear interpolation with support points
     * In Aeolus, 36 is typically subtracted before using vi, and so the effective support points are
     * typically at midi notes 36, 42, 48, 54, 60 (middle C), 66, 72, 78 , 84 , 90 , 96
     */
    float _v [N_NOTE];
};


/** Linear interpolation for N_HARM=64 harmonics across N_NOTE=11 support points
 *
 */
class HN_func
{
public:

    HN_func ();
    /**
     * Reset all the N_HARM=64 harmonics across N_NOTE=11 supporting notes to a constant value
     * @param v Constant value v for resetting the Harmonics/Notes table 64x11 table
     */
    void reset (float v);
    /**
     * Set support value across the N_HARM=64 harmonics for note i
     * @param i The note i (double use: either 11 notes across octave or 11 supporting points
     *          for entire keyboard range, see class N_func)
     * @param v The value to be set
     */
    void setv (int i, float v);
    /**
     * Clear the support value at note i across the harmonics
     * @param i The note (midi value) for which the values should be cleared
     */
    void clrv (int i);
    /**
     * Set support value for harmonic h and note i
     * @param h Harmonic h to be modified
     * @param i The note i (double use: either 11 notes across octave or 11 supporting points
     *          for entire keyboard range, see class N_func)
     * @param v The value to be set
     */
    void setv (int h, int i, float v) { _h [h].setv (i, v); }
    /**
     * Clear the support value at note i for harmonic h
     * @param h Harmonic h to be modified
     * @param i The note (midi value) for which the values should be cleared
     */
    void clrv (int h, int i) { _h [h].clrv (i); }
    /**
     * Get the value stored at present for note i for harmonic h
     * @param h Harmonic to be used
     * @param i Note index i
     * @return Float value as interpolated for the note and harmonic
     */
    [[nodiscard]] float vs (int h, int i) const { return _h [h].vs (i); }
    /**
     * Has value been set for note of index i in harmonic h
     * @param h Index of harmonic to be queried
     * @param i Index of note to be queried
     * @return 1 (true) if the value is explicitly set, 0 otherwise
     */
    [[nodiscard]] int   st (int h, int i) const { return _h [h].st (i); }
    /**
     * Get interpolated value.
     * \n\n
     * The parameter n is divided by 6 and then
     * the float values are interpolated linearly between the 11 individual basic values
     * @param h The harmonic to be queried
     * @param n Midi note value to be interpolated (0-66 for interpolation, linear
     * extrapolation outside this range)
     * @return Interpolated float value
     */
    [[nodiscard]] float vi (int h, int n) const { return _h [h].vi (n); }
    /** Write the support points for the first k harmonics to file
     * @param F File open for writing
     * @param k Write up to k harmonics
     */
    void write (FILE *F, int k);
    /** Read the support points for the first k harmonics from a file
     * @param F File open for reading
     * @param k Read up to k harmonics
     */
    void read (FILE *F, int k);

private:
    /**
     * The support points are stored as an array of N_func, 1 per harmonic
     */
    N_func _h [N_HARM];
};


/**
 * Additive synthesizer configuration parameters for a rank
 * In Aeolus, a "Rank" as defined by class Rank in model.h consists of an
 * Addsynth object and a Rankwave object, where the Addsynth object holds
 * the global parameters needed for synthesis
 */


class Addsynth
{
public:

    Addsynth ();
    /** Reset all parameters to default values */
    void reset ();

    int save (const char *sdir);
    int load (const char *sdir);

    char       _filename [64];
    char       _stopname [32];
    char       _copyrite [56];
    char       _mnemonic [8];
    char       _comments [56];
    char       _reserved [8];
    int32_t    _n0;     // first note
    int32_t    _n1;     // last note
    int32_t    _fn;     // frequency multiplier numerator, 1 for 16', 2 for 4'
    int32_t    _fd;     // frequency multiplier denominator, 2 for 16', 1 for 4'
    N_func     _n_vol;  // pipe amplitude in dB
    N_func     _n_off;  // pipe offset in Hz
    N_func     _n_ran;  // random pipe offset amplitude in Hz
    N_func     _n_ins;  // instability in cents
    N_func     _n_att;  // attack duration in s
    N_func     _n_atd;  // attack detune in cents
    N_func     _n_dct;  // release time in s
    N_func     _n_dcd;  // release detune in cents
    HN_func    _h_lev;  // harmonic's amplitude level in dB
    HN_func    _h_ran;  // harmonic's random amplitude level variation in dB (?)
    HN_func    _h_att;  // harmonic's attack duration in s
    HN_func    _h_atp;  // harmonic's attack peak in dB

    char       _pan;    // panning position: 'L', 'C', 'R', or 'W'
    int32_t    _del;    // reverb delay in ms
};


#endif

