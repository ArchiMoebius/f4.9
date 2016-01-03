/*
    (F 4.9) a simple program which implements the F5 algorithm
    Copyright (C) 2013

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BITSTREAM_H
#define	BITSTREAM_H

#include <stdio.h>
#include <stdlib.h>

#define BIT_STREAM_READ 1
#define BIT_STREAM_WRITE 0

/*! \struct BitStream
 * A wrapper for byte level manipulation
 */
typedef struct BitStream {
    int bitCount; /**< make sure we only write bitCount bits */
    int bitReadIndex; /**< where in the byte are we*/
    int bitWriteIndex; /**< where in the byte are we*/
    int bitsRead; /**< for encode*/
    int bitsWritten;///<for decode
    int init;///<flag keeping straight whether or not we have been setup
    int mode;///<reading or writing?
    int sizeInBits;///<total stego message filesize

    short headerSizeBits;///<stego message header

    unsigned char cChar;///<one byte at a time

    FILE* fp;// the file we are eating bytes from
} BitStream;

int closeBitStream(BitStream*);
int getNextBit(BitStream*);
int initBitStream(BitStream*, char*, int);
int openBitsStream(BitStream*, const char*);
int writeByte(BitStream*, unsigned char);
int writeNextBit(BitStream*, unsigned short);

#endif	/* BITSTREAM_H */
