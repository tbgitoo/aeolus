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


#ifndef AEOLUS_REVERB_H
#define AEOLUS_REVERB_H

/**
 * Delay line. This class implements a delay line with frequency-dependent decay times. Comments in part based
 * on interaction with claude.ai
 */
class Delelm
{
private:

    friend class Reverb; // The delay line element is part of the reverb effect
    /**
     * Init the delay line
     * @param size Size of the circular buffer
     * @param fb Feedback strength
     */
    void init (int size, float fb);
    /**
     * Free the buffer
     */
    void fini ();
    /**
     * Set attenuation time to 60db for the mid frequency range
     * @param tmf The time for 60db decay in the mid frequency range
     */
    void set_t60mf (float tmf);
    /**
     * Set attenuation time to 60db for the low frequency range, along with filter strength
     * @param tlo The time for 60db decay int he low frequency range
     * @param _wlo The low frequency filter strength
     */
    void set_t60lo (float tlo, float _wlo);
    /**
      * Set attenuation time to 60db for the high frequency range, along with filter strength
      * @param tlo The time for 60db decay int he high frequency range
      * @param _wlo The high frequency filter strength
      */
    void set_t60hi (float thi, float chi);
    /**
     * Print the parameters of this delay line to the android log, with tag "AeolusSynthesizer Reverb"
     */
    void print ();
    /**
     * Process a sample with the aid of the delay line. This updates the delay line as well
     * @param x The sample to process
     * @return The sample + delay line effects
     */
    float process (float x);

    int        _i; // position in the circular buffer
    int        _size; // size (number of samples) in the circular buffer
    float     *_line; // circular buffr of the delay line
    float      _fb; // feedback strength
    float      _gmf; // gain for mid-frequency signal in the delay feedback
    float      _glo; // gain for the low-frequency signal in the delay feedback
    float      _wlo; // Weight of the present signal for integration into the low frequency signal.
    float      _whi; // Weight of the preent signal for intergration into the high frequency signal
    float      _slo; // current state (amplitude) of the low frequency signal
    float      _shi; // current state (amplitude) fo the high frequency signal
};


/**
 * Reverb processor
 *
 * The reverb processor is applied to the global signal produced by all pipes in all divisions
 * and is used to obtain a reverb effect applying globally to the organ.
 *
 * This implementation uses a Freeverb-style algorithm with multiple parallel comb filters
 * and allpass filters to create a realistic reverb effect. The parallel comb and all-pass
 *
 * Comments in part generated using claude.ai
 */
class Reverb
{
public:
    
    void init (float rate);
    void fini ();
    /** Process reverb effects
       * @param n Number of samples to process
       * @param gain Volume gain (linear)
       * @param W Pointer to output buffer with the omnidirection signal, will be modified by the reverb
       * @param X Pointer to output buffer with the front-back signal, will be modified by the reverb
       * @param Y Pointer to output buffer with the left-right delta signal, will be modified by the reverb
       * @param R Pointer to output buffer with the reflected signal. This is used to feed the reverb, and
       *          the processed signal is then added to the other output buffers
       */
    void process (int n, float gain, float *R, float *W, float *X, float *Y, float *Z);
    /** Set new overall delay, maximum limited by _size/sampling rate
     *
     * @param del Delay in s
     */
    void set_delay (float del);
    /**
     * Decay time for the overall line
     * @param tmf Decay time medium frequencies
     */
    void set_t60mf (float tmf);
    /**
     * Set the low frequency decay time and cutoff frequency
     * @param tlo Decay time for the low frequency part
     * @param flo Cutoff frequency for the low frequency part
     */
    void set_t60lo (float tlo, float flo);
    /**
     * Set the high frequency decay time and cutoff frequency
     * @param thi High frequency decay time
     * @param fhi High frequency cutoff
     */
    void set_t60hi (float thi, float fhi);

private:
    /**
     * Print the parameters of the reverb
     */
    void print ();
    /**
     * The common reverb delay line
     */
    float  *_line;
    /**
     * Sihze of the common reverb delay line
     */
    int     _size;
    /**
     * Actual reverb delay in use, <= _size
     */
    int     _idel;
    /**
     * Current position in the common delay line
     */
    int     _i;
    /**
     * Individual delay element for comb filtering
     */
    Delelm  _delm [16];
    /**
     * Sampling rate
     */
    float   _rate;
    /**
     * Overall gain of the reverb
     */
    float   _gain;
    /**
     * Decay time medium frequency
     */
    float   _tmf;
    float   _tlo; // decay time low frequency
    float   _thi; // decay time high freqeuncy
    float   _flo; // cutoff frequency low frequency
    float   _fhi; // cutoff freqeuncy high frequency
    float   _x0, _x1, _x2, _x3, _x4, _x5, _x6, _x7; // comb filter signals, freeverb style
    float   _z; // internally follows the reflected signal with minimal lowpass

    static int   _sizes [16]; // predefined buffer sizes for the delay lines
    static float _feedb [16]; // predefined feedback strengths for the delay lines
};


#endif
