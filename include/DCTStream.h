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

#ifndef DCTSTREAM_H
#define	DCTSTREAM_H

#include <stdio.h>
#include <stdint.h>

#include <jpeglib.h>

/*! \struct DCTStream
 * A wrapper for libJPEG objects and our F 4.9 implementation
 */
typedef struct DCTStream {
    FILE *inJPEGFile;// the source data comes from here
    FILE *outJPEGFile;// the destination if one goes here

    int prng_prime;// the key used for the PRNG

    jvirt_barray_ptr *coef_arrays; // the array of coefficients which comes from the source data

    long index;// where are we at in the coefficients
    long prng_max;// the total

    short includeDC;// shall we include DC terms in the coefficients array
    short includeZero;// shall we include zero terms in the coefficients array
    short start;// have we been started yet - a simple flag

    struct jpeg_compress_struct dstinfo;// where the data is going
    struct jpeg_decompress_struct srcinfo;// where the data is from
    struct jpeg_error_mgr jpegDstErr, jpegSrcErr;// simple error wrappers fro libJPEG use
} DCTStream;

short *getNextCoefficient(DCTStream*);
short initDCTStream(DCTStream *, char*, char*, short, short, int, int);

void closeDCTStream(DCTStream*);
void printCoefficients(DCTStream *);

#endif	/* DCTSTREAM_H */
