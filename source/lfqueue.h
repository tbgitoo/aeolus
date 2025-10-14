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
// Thomas Braschler / Mathis Braschler (2025): Addes some more comments
// ----------------------------------------------------------------------------


#ifndef AEOLUS_LFQUEUE_H
#define AEOLUS_LFQUEUE_H


#include <cstdint>

/**
 * Circular buffer of storing uint8 (byte-wise) messages
 */
class Lfq_u8
{
public:
    /** Constructor
     *
     * @param size Buffer size
     */
    explicit Lfq_u8 (int size);
    /**
     * Destructor
     */
    ~Lfq_u8 ();
    /**
     * Available bytes for writing. This is the total buffer minus the bytes written but not yet read
     * @return
     */
    int       write_avail () const { return _size - _nwr + _nrd; }
    /** Commit the last newly written bytes
     * @param n Number of bytes to commit
     */
    void      write_commit (int n) { _nwr += n; }
    /**
     * Write a byte to the circular buffer
     * @param i Position ahead of the first byte available for writing (provide 0 to use the first
     * available byte)
     * @param v Byte value to store
     */
    void      write (int i, uint8_t v) { _data [(_nwr + i) & _mask] = v; }
    /** How many bytes are available for reading
     * @return The number of bytes that have been written to the buffer but not yet read
     */
    int       read_avail () const { return _nwr - _nrd; }
    /**
     * Commit the bytes read. This means that the write pointer advances over the commited bytes, and they
     * are considered free for modification
     * @param n The number of bytes to be marked as read
     */
    void      read_commit (int n) { _nrd += n; }
    /**
     * Read the i-th byte ahead of the read pointer
     * @param i get the i-th byte ahead of the read pointer. Provide i=0 go get the byte a the current
     * read pointer, and i positive to get bytes ahead of the read pointer. i should not be larger than
     * read_avail()-1, if read_avail() is zero, no byte should be read (that is, the reading operation
     * will work, but the resulting value is indetermined)
     * @return i-th byte
     */
    uint8_t   read (int i) { return _data [(_nrd + i) & _mask]; }

private:

    uint8_t * _data; // The internal buffer
    int       _size; // The size of the internal buffer. Must be a power of 2
    int       _mask; // Binary mask, used to ensure that only the valid buffer range is accessed
    int       _nwr; // current write position in the buffer array
    int       _nrd; // current read position in the buffer array

};

/**
 * Circular buffer of storing uint16 (2-byte) messages
 */
class Lfq_u16
{
public:
    /** Constructor
     * @param size Buffer size
     */
    explicit Lfq_u16 (int size);
    /**
     * Destructor
     */
    ~Lfq_u16 ();
    /**
     * Available 2-byte slots for writing. This is the total buffer minus entries written but not yet read
     * @return Number of currently available slots for writing uint16 entries
     */
    int       write_avail () const { return _size - _nwr + _nrd; }
    /** Commit the last newly written uint16 entries
     * @param n Number of uint16 entries to commit
     */
    void      write_commit (int n) { _nwr += n; }
    /**
     * Write a uint16 entry to the circular buffer
     * @param i Position ahead of the first uint16 entry available for writing (provide 0 to use the first
     * available uint16 slot)
     * @param v Byte value to store
     */
    void      write (int i, uint16_t v) { _data [(_nwr + i) & _mask] = v; }
    /** How many bytes are available for reading
     * @return The number of bytes that have been written to the buffer but not yet read
     */
    int       read_avail () const { return _nwr - _nrd; }
    /**
     * Commit the uint16 tokens read. This means that the write pointer advances over the commited uint16,
     * and they
     * are considered free for modification
     * @param n The number of bytes to be marked as read
     */
    void      read_commit (int n) { _nrd += n; }
    /**
     * Read the i-th byte ahead of the read pointer
     * @param i get the i-th byte ahead of the read pointer. Provide i=0 go get the byte a the current
     * read pointer, and i positive to get bytes ahead of the read pointer. i should not be larger than
     * read_avail()-1, if read_avail() is zero, no byte should be read (that is, the reading operation
     * will work, but the resulting value is indetermined)
     * @return i-th byte
     */
    uint16_t  read (int i) { return _data [(_nrd + i) & _mask]; }

private:

    uint16_t *_data;
    int       _size;
    int       _mask;
    int       _nwr;
    int       _nrd;
};

/**
 * Circular buffer for storing uint32 (entities of 4 bytes each) messages
 */
class Lfq_u32
{
public:
    /** Constructor
     *
     * @param size number of uint32 blocks in the buffer
     */
    explicit Lfq_u32 (int size);
    /**
     * Destructor
     */
    ~Lfq_u32 ();

    /**
     * Number of presently available slots for storing uint32 values into the buffer
     * @return The number of available buffer slots
     */

    int       write_avail () const { return _size - _nwr + _nrd; }
    /**
     * Commit the last n uint32 values written to the buffer
     * @param n The number of uint32 values to commit
     */
    void      write_commit (int n) { _nwr += n; }
    /**
     * Write a uint32 value to the buffer
     * @param i Slots ahead of the current write pointer (the first slot available is at i=0, that is
     *           at the write pointer itself)
     * @param v The value to store
     */
    void      write (int i, uint32_t v) { _data [(_nwr + i) & _mask] = v; }
    /**
     * Get the number of uint32 values that are at present available for reading (i.e. that
     * have been written and commited to the buffer, but for which no read-commit was issued yet)
     * @return The number of uint32 values available for reading
     */
    int       read_avail () const { return _nwr - _nrd; }
    /**
     * Commit the reading of n uint32 values. The committed values are considered definitly read out
     * and are thus available for overwrite, do not read them once you issued the read commit.
     * @param n Number of uint32 values to commit as having been read
     */
    void      read_commit (int n) { _nrd += n; }
    /**
     * Read the i-th uint32 value ahead of the read pointer
     * @param i Advance relative to read pointer (0 = get the value at the read pointer itself).
     * Should be positive, otherwise, values already liberated for potential write operations are read
     * @return The value stored at the i-th slot relative to the read pointer
     */
    uint32_t  read (int i) { return _data [(_nrd + i) & _mask]; }

private:

    uint32_t *_data; // The data array of the circular buffer
    int       _size; // Length of the data array, must be a power of 2
    int       _mask; // Binary mask used to circularize the access and avoid access outside of the buffer length,
                      // the value of this will be size-1
    int       _nwr; // Write position
    int       _nrd; // Read position
};


#endif

