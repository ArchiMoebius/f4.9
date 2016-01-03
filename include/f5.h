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

#ifndef F5_H
#define	F5_H

#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include <jpeglib.h>

#include "BitStream.h"
#include "DCTStream.h"


/*! Creates a type name for stegoOperation function pointers rock! */ 
typedef int (*stegoOperation)(BitStream *bs, DCTStream *ds, short **buffer);

int getPosition(short **buffer, short *bits);
int stegoF5(char *, char *, char* , stegoOperation, int, int, int);

short decodeData(BitStream *, DCTStream *, short **buffer);
short encodeData(BitStream *, DCTStream *, short **buffer);

static void setMESchema(int);

#endif	/* F5_H */
