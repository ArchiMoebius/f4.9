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

#include "BitStream.h"

/**
* We are done with it, clean up time!
* @param bs bitstream object
* @return was it already clean?
*/
int closeBitStream(BitStream* bs) {
    int status = bs->init = 0;

    if(bs->fp != 0) {
        status = fclose(bs->fp);
        bs->fp = 0;
    }

    return status;
}

/**
* If you want another and there is any left, you get another bit, only one!
* @param bs bitstream object
* @return did everything work?
*/
int getNextBit(BitStream* bs) {
    static int bytesRead = 0;

    if (bs->init != 1)
        return EOF;

    if(bs->mode == BIT_STREAM_WRITE){
        fprintf(stderr, "\nI'm not doing anything, nothing! Until you fix me!\n");
        return EOF;
    }

    int aByte = 0;

    if(bs->bitReadIndex < 0) { // time to start anew
        bs->bitReadIndex = 7;

        if(bytesRead >= bs->headerSizeBits/8) { // have we finished reading the header filesize in?
            aByte = fgetc(bs->fp);

            if(aByte != EOF) { // another byte for us!
                bs->cChar = (unsigned char) aByte;
                bs->bitsRead+=8;
            }
        } else { // read the filesize in bits of the stego message
            bs->cChar = (unsigned char)(bs->bitCount&0xFF);
            bs->bitCount >>=8;
            bytesRead++;
        }
    }

    if(aByte == EOF) {
        bytesRead = 0;
    }

    if (aByte != EOF) {
        int val = (bs->cChar >> bs->bitReadIndex) & 0x1;
        bs->bitReadIndex--;
        return val; // and here is your bit, take it, take it now!
    }
    return aByte; // yup, still working
}

/**
* Open and start sucking or spitting bits out
* @param bs a bitstream 'object'
* @param stegoFile which file shall we beat the bits out of?
* @param mode To read or not to read, shall I write?
* @return did everything workout? (were we able to open the file?)
*/
int initBitStream(BitStream* bs, char* stegoFile, int mode){
    bs->bitCount = 0;// how many bits have there been?;
    bs->headerSizeBits = sizeof(bs->bitCount)*8;// encoded filesize header, in bits;
    bs->bitReadIndex = -1;// which bit were we on again?;
    bs->bitWriteIndex = 0;// which bit was written last?;
    bs->bitsRead = 0;// how many have we seen;
    bs->bitsWritten = 0;// how many have been pushed out?;
    bs->cChar = 0;// A byte! A whole byte!;
    bs->init = 0;// let the games begin!;
    bs->sizeInBits = 0;// the whole thing, I mean - All of it!;
    bs->mode = mode;// reading or writing in binary?;

    if (0 != openBitsStream(bs, stegoFile)) {
        fprintf(stderr, "\ninitBitStream: Can't open %s\n", stegoFile);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/**
* Welp, the file exists, lets get to it
* @param bs bitstream object
* @param filename which file again?
* @return did everything workout?
*/
int openBitsStream(BitStream* bs, const char* filename) {
    if (bs->init == 1)
        return EXIT_FAILURE;

    if(bs->mode == BIT_STREAM_READ) {
        if ((bs->fp = fopen(filename, "rb")) == NULL) {
            fprintf(stderr, "\nopenBitsStream: Can't open %s\n", filename);
            return EXIT_FAILURE;
        }
    } else if (bs->mode == BIT_STREAM_WRITE) {
        if ((bs->fp = fopen(filename, "wb")) == NULL) {
            fprintf(stderr, "\nopenBitsStream: Can't open %s\n", filename);
            return EXIT_FAILURE;
        }
    } else {
        fprintf(stderr, "\nAre you crazy!\n");
        return EXIT_FAILURE;
    }

    fseek(bs->fp, 0L, SEEK_END); //  Get the stego message filesize
    bs->sizeInBits = bs->bitCount = ftell(bs->fp)*8;// how many bits are there here?;
    fseek(bs->fp, 0L, SEEK_SET);

    bs->init = 1;// we are ready to roll!
    return EXIT_SUCCESS;
}

/**
* Place byte, a whole byte into our file!
* @param bs bitstream object
* @param byte the byte to write
* @return did it work?
*/
int writeByte(BitStream* bs, unsigned char byte) {
    static int bytesRead = 0;// how many bytes have there been?;

    if (bs->init != 1)
        return EOF;

    if(bs->mode == BIT_STREAM_READ){
        fprintf(stderr, "\nI'm not doing anything, nothing! Until you fix me!\n");
        return EOF;
    }

    if(bytesRead >= bs->headerSizeBits/8) {// write the un-stego'd message
        return fputc(byte, bs->fp); // returns EOF on error
    } else {// read the stego message filesize in
        bs->sizeInBits |= (byte << (bytesRead*8));
        bytesRead++;
    }

    return !EOF;
}

/**
* One by one they come, one by one they go. bits to bytes, bits to bytes.
* @param bs bitstream object
* @param bit a shiny little bit!
* @return did it work?
*/
int writeNextBit(BitStream* bs, unsigned short bit) {
    if (bs->init != 1)
        return EOF;

    if(bs->mode == BIT_STREAM_READ){
        fprintf(stderr, "\nI'm not doing anything, nothing! Until you fix me!\n");
        return EOF;
    }

    if(bs->bitsWritten > bs->headerSizeBits) {// skip the stego message filesize
        if(bs->bitsWritten >= bs->sizeInBits + bs->headerSizeBits) {// only write our stego message filesize
            return EOF;
        }
    }

    bs->bitsWritten++;

    static unsigned char byte = '\0';

    byte <<= 1;

    if(bit) {
        byte |= 0x1;
    }

    bs->bitWriteIndex++;

    if(bs->bitWriteIndex > 7) {// time to write a byte!
        writeByte(bs, byte);
        byte = '\0';
        bs->bitWriteIndex = 0;
    }

    return !EOF;
}
