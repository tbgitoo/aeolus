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


#ifndef AEOLUS_RANKWAVE_H
#define AEOLUS_RANKWAVE_H


#include "addsynth.h"
#include "rngen.h"
#include <android/log.h>


#define PERIOD 64


class Pipewave
{
private:

    /**
     * Constructor, initializes to null values and non-defined data arrays (nullptr)
     */
    Pipewave () :
            _p0 (nullptr), _p1 (nullptr), _p2 (nullptr), _l1 (0), _k_s (0),  _k_r (0), _m_r (0),
            _link (nullptr), _sbit (0), _sdel (0),
            _p_p (nullptr), _p_f(0), _y_p (0), _z_p (0), _p_r (0), _y_r (0), _g_r (0), _i_r (0)
    {}

    /**
     *  Destructor, frees the wave point buffer array starting at point _p0
     */

    ~Pipewave () { delete[] _p0; }

    friend class Rankwave;
    /**
     * Generate wave: Allocate memory for the data array starting at _p0 and generate
     * wavetable to be used for a particular pipe. The synthesis algorithm is based on
     * harmonics, a loop length to fit in the harmonics as closely as possible without glitch,
     * and a transition from attach harmonic distribution to continuous harmonic distribution
     * @param D Addsynth holding the applicable parameters
     * @param n The midi note n of this pipe
     * @param fsamp sampling frequency
     * @param fpipe Base frequency of this pipe
     */
    void genwave (Addsynth *D, int n, float fsamp, float fpipe);
    /**
     * Save the wavetable for this pipe and associated description to file in binary format
     * @param F File pointer for writing
     */
    void save (FILE *F);
    /**
     * Load the wavetable for this pipe from binary file
     * @param F File pointer for reading, set to the beginning of the data section for this pipe
     */
    void load (FILE *F);
    /**
     * Play from the wavetable. Playing means looping through the wavetable (including initial attack)
     * while the pipe is on  (the _sdel bit is set) and exponentially releasing the pipe when
     * the pipe is turned off
     * Technically, a play pointer (_p_p, internally p) and a release point (_p_r) are moved through
     * the wavetable sections, with small variations to create a more realistic sound, and
     * the corresponding audio data is moved into the output buffer designated by
     * _out
     */
    void play ();

    /**
 * @brief Loop length: Find a combination of a number of entire number of cycles bb at pipe base frequency f and number
 * of samples aa so that at sampling frequency fsamp, the pipe frequency f is optimally matched.
 *
 * More precisely, this is an optimization problem where the actual pipe frequency given by
 * fsamp*bb/aa should be as close as possible to the target pipe frequency f. This is to avoid
 * phase jumps and thus cracks when looping indefinitely over the samples. <br /> In detail,
 * the search for an optimal combination aa,bb
 * is implemented iteratively:<br />
 * - start out with 1 cycle (bb=1)<br />
 * - calculate aa from aa=fsamp*bb/f, exactly and to the nearest integer. Examine the delta between the integer and true estimate. <br />
 * - Increase the number of cycles to bb>1, such that 1/bb best matches the delta<br />
 * - Multiply the previous integer sample number aa by the new cycle number bb, and add or subtract on more
 *   sample depending on the sign of the delta. This is the key step: the addition or subtraction of
 *   one sample means that the effective ratio aa/bb as compared to aa is now changed by 1/bb, which optimally
 *   approaches aa towards the nearest integer.<br />
 * - Repeat the approach by considering the remaining fractional part and again multiplying aa, and adding
 *   samples to optimally reduce the fractional part of aa. Report the highest aa value below lmax, or
 *   stop at ppm precision.
 *
 * @param f Frequency of the pipe
 * @param fsamp sampling frequency, in the implementation here can include up to 3x oversampling
 * @param lmax maximum externally imposed loop length, the iteration will stop to avoid aa exceeding lmax
 * @param aa Pointer to the length of the loop for output
 * @param bb Pointer to the number of cycles in the loop fot output
 */
    static void looplen (float f, float fsamp, int lmax, int *aa, int *bb);
    /** Wavetable preparation: Calculate the attack part (initial part, when pipe is turned on)
     * The basic idea is to simulate higher frequency components which are initially produced when
     * the pipe starts playing, before the main harmonics set in. This function
     * @param n The number of samples to be prepared
     * @param p The profile of the attack, the higher p, the shorter the attack peak
     */
    static void attgain (int n, float p);

    float     *_p0;    // wavetable data pointer: attack start
    float     *_p1;    // wavetable data pointer: loop start
    float     *_p2;    // wavetable data pointer: loop end
    /**
     * Attack length (number of sample points for attack length)
     */
    int32_t    _l0{};
    /**
     * Main loop length (stationnary, after attack)
     */
    int32_t    _l1;
    int16_t    _k_s;   // sample step, i.e. periods of pre-sampling minimally required
    int16_t    _k_r;   // release lenght
    float      _m_r;   // release multiplier
    float      _d_r{};   // release detune
    float      _d_p{};   // instability

    Pipewave  *_link;  // link to next pipe in active chain
    uint32_t   _sbit;  // on state bit
    uint32_t   _sdel;  // delayed state
    float     *_out{};   // audio output buffer
    float     *_p_p;   // play pointer
    float      _p_f;   // play pointer fraction
    float      _y_p;   // play interpolation
    float      _z_p;   // play interpolation speed
    float     *_p_r;   // release pointer
    float      _y_r;   // release interpolation
    float      _g_r;   // release gain
    int16_t    _i_r;   // release count

    /**
     * Allocate memory for the static internal variables _arg (time duringy cycle) _att (attack gain)
     * @param fsamp Sampling frequency
     */
    static void initstatic (float fsamp);

    static   Rngen   _rgen; // for random number generation
    static   float  *_arg; // time parameter during waveform generation
    static   float  *_att; // harmonic's attack gain time series
};

/**
 * The class Rankwave defines a rank, which is a set of pipes covering a range of notes (from n0 to n1),
 * voiced in a similar style
 */
class Rankwave
{
public:
    /** Constructor, with midi not range
     *
     * @param n0 Lowest midi note
     * @param n1 highest midi note
     */
    Rankwave (int n0, int n1);
    ~Rankwave (void);

    /** Set midi note to playing<br />
     * Note: the function also initializes the delay system (which avoids abrupt ending of the note once playing is done to
     * avoid click artefacts)
     * @param n The midi note number that should be set to ON (playing)
     */
    void note_on (int n)
    {

        if ((n < _n0) || (n > _n1)) return;
        Pipewave *P = _pipes + (n - _n0);
        P->_sbit = _sbit;
        if (! (P->_sdel || P->_p_p || P->_p_r))
        {
            P->_sdel |= _sbit;
            P->_link = _list;
            _list = P;
        }
    }
    /** Set midi note to off<br />
     * Note: turning off the note also starts the delay which allows the note to die down without click
     * @param n The midi note number that should be set to OFF (silent)
     */
    void note_off (int n)
    {
        if ((n < _n0) || (n > _n1)) return;
        Pipewave *P = _pipes + (n - _n0);
        P->_sdel >>= 4;
        P->_sbit = 0;
    }
    /**
     * Turn all the pipes (for all midi notes) of this rank off
     */
    void all_off ()
    {
        Pipewave *P;
        for (P = _list; P; P = P->_link) P->_sbit = 0;
    }
    /**
     * Lowest midi note in this rank
     * @return The lowest midi note played by this rank
     */
    [[nodiscard]] int  n0 () const { return _n0; }
    /**
     * Highest midi note in this rank
     * @return The highest midi note played by this rank
     */
    [[nodiscard]] int  n1 () const { return _n1; }
    /**
     * Play the ranks that are on
     * @param shift If >0, advance the delay (decay of deactived notes)
     */
    void play (int shift);
    /**
     * Set output parameters
     * @param out Pointer to the output buffer to fill
     * @param del delay, 0 to 31 length of delay
     * @param pan Spatial position of the pipe, for stereo
     */
    void set_param (float *out, int del, int pan);
    /** Generate the wavetables for the pipes
     *
     * @param D Additive synthesizer containing the parameters for wavetable synthesis
     * @param fsamp Sampling frequency
     * @param fbase Tuning base frequency
     * @param scale Tuning scale to be applied
     */
    void gen_waves (Addsynth *D, float fsamp, float fbase, float *scale);
    /**
     * Save the wavetables. This saves the wavetable of each pipe in the rank into a common .ae1 file.
     * @param path Path to folder for saving wavetables (ae1 files)
     * @param D Additive synthesizer parameters, here used for the file name
     * @param fsamp Sampling frequency (checked later when loading from file, must match for wavetable to be used)
     * @param fbase Base (checked later when loading from file, must match for wavetable to be used)
     * @param scale (checked later when loading from file, must match for wavetable to be used)
     * @return 0 on success, 1 on error
     */
    int  save (const char *path, Addsynth *D, float fsamp, float fbase, float *scale);

    /**
     * Load wavetables into this rank. This causes all the pipes in this rank to load their wavetable
     * from the common ae1 file.
     * @param path Path to the ae1 file folder
     * @param D Additive synthesizer params, here used for the file name
     * @param fsamp Sampling frequency; loading will only be performed if the sampling frequency matches
     * @param fbase Base tuning frequency. Loading will onyl be performed if the sampling frequency matches
     * @param scale Tuning scale
     * @return 0 upon success, 1 upon failure, including mismatch in fsamp, fbase or scale
     */
    int  load (const char *path, Addsynth *D, float fsamp, float fbase, float *scale);
    /** Has this rank been modified compared to the wavetable information on disk?
     * @return True if modified (i.e. wavetables calculated), false if corresponding to file information
     */
    [[nodiscard]] bool modif () const { return _modif; }

    /** Used by division logic. The _cmask is the currently applicable mask for the rank. The lowest 7 bits
     * of the mask code for a maximum of 7 keyboards to which the rank can respond. The rank will play a given note
     * if it is played on one of the keyboards to which the rank responds as indicated by the bits in the c-mask. The
     * current _cmask is used to decide which notes should be played as a function of the state of the different keyboards
     * regarding that note in Division::
     */
    int  _cmask;
    /** Handled by division logic. The _nmask is the incoming (new) mask to be imposed on the rank
     * The nmask is typically set via the division to which the rank belongs, through Division::set_rank_mask
     * and cleared through Division::clr_rank_mask. These functions do not update the current mask _cmask just yet, and
     * discrepancies between _cmask and _nmask are used in the Division::update (unsigned char *keys) function to
     * detect recent changes in rank activation, in order to correct the actual list of playing notes for the rank
     */
    int  _nmask;

private:

    Rankwave (const Rankwave&);
    Rankwave& operator=(const Rankwave&);

    int         _n0; // lowest midi note for the rank
    int         _n1; // Highest midi note for the rank
    uint32_t    _sbit; // Bitmask indicating the starting bit for the delayed plaing cycle
    Pipewave   *_list; // LIst of active pipes
    Pipewave   *_pipes; // Overall array of pipes
    bool        _modif; // is rank modified compared
};


#endif

