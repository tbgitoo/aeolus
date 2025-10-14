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


#ifndef AEOLUS_ASECTION_H
#define AEOLUS_ASECTION_H


#include "global.h"


#define PERIOD 64 // Audio processing block size
#define MIXLEN 64 // Mixing buffer length
#define NCHANN 4 // Number of audio channels, for spatial processing
#define NRANKS 32 // Maximum number of organ ranks (sets of pipes)

/**
 * The Diffuser class implements diffusion filters for smoothing reverb effects
 */
class Diffuser
{
public:
    /**
    * Diffuser initialization, including buffer and feed-forward coefficient
    * @param size size of the buffer for this diffuser
    * @param c strength of the feed forward coefficient
    */
    void init (int size, float c);
    /**
     * Delete the buffer
     */
    void fini ();
    /**
     * Get the buffer size associated with this diffuser
     * @return  buffer size
     */
    int  size () { return _size; }
    /**
     * Diffuse amplitude by mixing in past data
     * @param x Current amplitude to process
     * @return processed amplitude
     */
    float process (float x)
    {
        float w;

        w = x - _c * _data [_i];
        x = _data [_i] + _c * w;
        _data [_i] = w;
        if (++_i == _size) _i = 0;
        return x;
    }

private:

    float     *_data; // buffer
    int        _size; // size of the buffer
    int        _i; // present index in processing
    float      _c; // feed forward coefficient in the audio diffusers
};

/**
 * Class for spatialization and smoothing of reverb, working on the audio signal post-synthesis
 * The audio sections are initiated in init_audio of the audio part.
 */
class Asection
{
public:

    /** Constructor
     * @param fsam sampling rate
     */
    explicit Asection (float fsam);
    /**
     * Destructor, free audio buffer
     */
    ~Asection ();

    /**
     * Obtain the write pointer to the buffer of this audio section. This pointer is obtained
     * by the division associated with this audio section during division processing, such that
     * the division can fill the audio buffer of the audio section with the data to then be processed
     * by the audio section
     * @return Write pointer to the beginning of the write buffer area of this audio section
     */
    float *get_wptr () { return _base + _offs0; }

    /**
     * get pointer to audio parameter for direct access (transmitted to model)
     * @return pointer to the audio parameters
     */
    Fparm *get_apar () { return _apar; }

    /** Set the time frame of the audio section (i.e. the "size" of the reprocessing blocks)
     * @param size Length (in seconds) of the audio reprocessing by this audio section
     */
    void set_size (float size);

    /** Process this audio section. The basic signal needs to be already present in the buffer
     * through invocation of the process function of the corresponding division
     * @param vol Overall volume
     * @param W Pointer to output buffer to which the omnidirection signal of this division/section
     *          will be added
     * @param X Pointer to output buffer to which the front-back signal of this division/section
     *          will be added
     * @param Y Pointer to output buffer to which the left-right selta signal of this division/section
     *          will be added
     * @param R Pointer to output buffer to which the reflected signal of this division/section
     *          will be added
     *         */
    void process (float vol, float *W, float *X, float *Y, float *R);

    static float _refl [16];

private:
    /**
     * Type of params: AZIMUTH for Horizontal positioning in the soundfield<br />
     * STWIDTH_ stereo width<br />
     * DIRECT part of directly transmitted "dry" signal<br />
     * REFLECT early reflection<br />
     * REVERB delayed reverb signal<br />
     */
    enum { AZIMUTH, STWIDTH, DIRECT, REFLECT, REVERB };

    int      _offs0;
    int      _offs [16];
    float    _fsam;
    float   *_base;
    float    _sw; // omnidirectional channel
    float    _sx; // front-back delta
    float    _sy; // left-right delta
    Diffuser _dif0; // diffuser for realistic reverb
    Diffuser _dif1; // diffuser for realistic reverb
    Diffuser _dif2; // diffuser for realistic reverb
    Diffuser _dif3; // diffuser for realistic reverb
    Fparm    _apar [5]; // values for the AZIMUTH, STWIDTH, DIRECT, REFLECT, REVERB coefficients with min and max
};


#endif

