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

#ifndef UTILS_H
#define UTILS_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <jpeglib.h>    //  punks dependant on size_t

#include "DCTStream.h"

#define MAX_N 8         //  we shall only consider 8x8 DCT blocks
#define MAX_CHANNEL 3   //  we shall assume there is only Y, U, and V

short *getCoefficient(j_decompress_ptr, jvirt_barray_ptr *, int, int, int, short, short);
short *makeShortArray(int);
short **makeShortPtrArray(int);

void printCapacity(char *, char *, int);
void printImageInfo(j_decompress_ptr);
void printPSNR(char *, char *, int);
void printStats(char *, char *, char *, int);

unsigned int **makeMeInt(int, int);

#endif  //UTILS_H
