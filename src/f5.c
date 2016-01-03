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
#include "DCTStream.h"
#include "f5.h"
#include "utils.h"

int F5_K = 0; // Passed in at runtime, used for matrix encoding
int F5_N = 0; // Passed in at runtime, used for matrix encoding

/**
* calculate the location in the buffer which should be updated to match the message bits
* @param buffer an array of coefficients
* @param bits message bits
* @return the position in the buffer which should be decreased in magnitude or -1 if it shouldn't be changed
*/
int getPosition(short *(*buffer), short *bits) {
                                /// TODO: architecture independent bit manipulation 
    short c, k;                 // counter variable;
    short p;                    // the position in the buffer which should be updated, if at all
    short *streambuffer = NULL; // information bits buffer
    short *syndrome;            // the actual syndrome

    streambuffer = makeShortArray(F5_N);
    syndrome = makeShortArray(F5_K);
    p = 0;

    if(DEBUG) {
        printf("\n\tBuffer:\t{");
        for (k = 0; k < F5_N; k++) {
            printf("%i ", *buffer[k]);
        }
        printf("}\n");
    }

    for (k = 0; k < F5_N; k++) { // compute the information bits;
        streambuffer[k] = (*buffer[k] > 0) ? abs(*buffer[k]) % 2 : (abs(*buffer[k]) + 1) % 2;
    }

    if(DEBUG) {
        printf("\n\tValue\t{");
        for (k = 0; k < F5_N; k++) {
            printf("%d ", streambuffer[k]);
        }
        printf("}\n\t");
    }

    for (k = 0; k < F5_K; k++) { // calculate the syndrome;
        syndrome[k] = 0;
        for (c = 1; c <= F5_N; c++) {
            if (c & (1 << k)) {
                //fprintf(stderr, "%i ", c-1);
                syndrome[k] ^= streambuffer[c-1];
            }
        }
    }

    for (k = F5_K-1; k >= 0; k--) { // calculate the position;
       p <<= 1; 
       p |= syndrome[k]^bits[k];
    }

    if(DEBUG) {
        printf("\nSYNDROM: ");
        for (k = 0; k < F5_N; k++) {
            printf("%i ", syndrome[k]);
        }
        printf("\n");

        printf(" == %i\n\t", p);
    }

    free(streambuffer);
    free(syndrome);

    return p - 1;
}

/**
* Setup our matrix encoding scheme
* @param stegoFile message file
* @param srcJPEG the cover
* @param dstJPEG the stego object
* @param operate shall we encode or decode
* @param prngKey the prime value to setup the permutative straddling
* @param quality setting to utilize for the output image (disabled currently)
* @param meK matrix encoding schema K value
* @return success or failure - did we encode/decode?
*/
int stegoF5(char *stegoFile, char *srcJPEG, char* dstJPEG,
            stegoOperation operate, int prngKey, int quality, int meK) {

    BitStream bs;// wrapper for bit manipulation;
    DCTStream ds;// wrapper for image data extraction - LibJPEG wrapper;

    if(EXIT_FAILURE == initDCTStream(&ds, srcJPEG, dstJPEG, 0, 0, prngKey, quality)){
        return EXIT_FAILURE;
    }

    if(meK < 1 || meK > 9) {
        fprintf(stderr, "\nstegoF5: Invalid meK value\n");
        closeDCTStream(&ds);
        return EXIT_FAILURE;
    } else {
        setMESchema(meK);
    }
    
    if(dstJPEG != NULL) {
        if(EXIT_FAILURE == initBitStream(&bs, stegoFile, BIT_STREAM_READ)){
            closeDCTStream(&ds);
            return EXIT_FAILURE;
        }
    } else {
        if(EXIT_FAILURE == initBitStream(&bs, stegoFile, BIT_STREAM_WRITE)){
            closeDCTStream(&ds);
            return EXIT_FAILURE;
        }
    }

    printImageInfo(&ds.srcinfo);

    int m = 0;                                  // counter variable;
    short *coefficient;                         // simple pointer;
    short **buffer = makeShortPtrArray(F5_N);   // our array of coefficients;

    while(1) {

        if (NULL == (coefficient = getNextCoefficient(&ds))) {
            printf("\nReached the end of the image...damn!\n");
            break;
        }

        buffer[m++] = coefficient;

        if (m == F5_N) {// is the buffer full?;

            if (0 > operate(&bs, &ds, buffer)) {// operate on the data! (encode or decode);
                printf("\nbye world!\n");
                break;
            }
            m = 0;
        }
    }
    free(buffer);// take the garbage out!;

    if(dstJPEG != NULL) {
        fprintf(stderr, "\nSTATS:\n\tTotalSize: %i\n\tBitsWritten: %i\n", bs.sizeInBits, bs.bitsRead);

        if(bs.sizeInBits == bs.bitsRead) {
            fprintf(stderr, "\nThe whole file was encoded!\n");
        } else {
            fprintf(stderr, "\nThe whole file was not encoded!\n");
        }
    } else {
        fprintf(stderr, "\nSTATS:\n\tTotalSize: %i\n\tBitsWritten: %i\n", bs.sizeInBits, bs.bitsWritten - bs.headerSizeBits);

        if(bs.sizeInBits == (bs.bitsWritten - bs.headerSizeBits)) {
            fprintf(stderr, "\nThe whole file was decoded!\n");
        } else {
            fprintf(stderr, "\nThe whole file was not decoded!\n");
        }
    }

    closeBitStream(&bs);
    closeDCTStream(&ds);

    return EXIT_SUCCESS;// we win, you lose!;
}

/**
* Put the data from the bs into the ds based upon the coefficients found within the buffer
* @param bs BitStream reference
* @param ds DCTStream reference
* @param buffer a grouping of coefficients
* @return are we done yet?
*/
short decodeData(BitStream *bs, DCTStream *ds, short **buffer){
    int c, k;                                   // counter variable;
    short *syndrome = makeShortArray(F5_K);     // juicy data! (our message will be here);
    unsigned short p = 0;                       // counter variable;
    unsigned short streambuffer[F5_N];          // information bits buffer;

    if(DEBUG) {
        printf("\n\tBuffer:\t{");

        for (p = 0; p < F5_N; p++) {
            printf("%i ", *buffer[p]);
        }
        printf("}\n");
    }

    for (p = 0; p < F5_N; p++) {// calculate the information bits;
        streambuffer[p] = (*buffer[p] > 0) ? abs(*buffer[p]) % 2 : (abs(*buffer[p]) + 1) % 2;
    }

    if(DEBUG) {
        printf("\n\tValue\t{");

        for (p = 0; p < F5_N; p++) {
            printf("%d ", streambuffer[p]);
        }
        printf("}\n\t");
    }

    for (k = 0; k < F5_K; k++) {// calculate the syndrome;
        syndrome[k] = 0;

        for (c = 1; c <= F5_N; c++) {

            if (c & (1 << k)) {

                if(DEBUG){
                        fprintf(stderr, "%i ", c-1);
                }
                syndrome[k] ^= streambuffer[c-1];
            }
        }
    }

    if(DEBUG) {
        printf("\nM --> ");
    }

    for (k = 0; k < F5_K; k++) {// rip our data out of the syndrome and write it out!;

        if(DEBUG) {
                printf("%i", syndrome[k]);
        }

        if(EOF == writeNextBit(bs, syndrome[k]))
            return EOF;
    }

    if(DEBUG) {
        printf("\n");
    }
    free(syndrome);// keep our rooms clean;

    return !EOF;
}

/**
* Setup our matrix encoding scheme
* @param meK the value of N = (2 ^ k) - 1
*/
static void setMESchema(int meK){
    F5_K = meK;                 // set the global variable;
    F5_N = (int)pow(2, F5_K)-1; // counter variable;
}

/**
* Put the data from the bs into the ds based upon the coefficients found within the buffer
* @param bs BitStream reference
* @param ds DCTStream reference
* @param buffer grouping of coefficients
* @return The number of bits which were encoded!
*/
short encodeData(BitStream *bs, DCTStream *ds, short **buffer) {
    int i, m;                           // counter variable;
    short bit, numBitsEncoded = 0;      // what bit is next;
    short *bits = makeShortArray(F5_K); // the array of bits;
    short *coefficient;                 // which coefficient;

    if(DEBUG) {
        printf("\n\tEncoding message bits: ");
    }
    for (i = 0; i < F5_K; i++) {
        bit = getNextBit(bs);

        if(DEBUG) {
                printf(" bit : %d", bit);
        }

        //dont encode anything
        if (bit == EOF && i == 0) {
            return EOF;
        }
        //pad with zeros
        if (bit == EOF) {
            bits[i] = 0;
            printf("\nALL DATA HIDDEN\n");
        }
        else {
            bits[i] = bit;
            numBitsEncoded++;
        }
    }
    if(DEBUG) {
        printf("\n");
    }

    int position = getPosition(buffer, bits);

    testPosition: //jumps here if coefficient causes shrinkage

    if(DEBUG) {
        printf("\tPosition: %i\n", position);
    }

    if (position != -1) {

        if (*(buffer[position]) > 0) {// decrease the magnitude;
            *buffer[position] = (*buffer[position])-1;
        } else {
            *buffer[position] = (*buffer[position])+1;
        }

        if((*buffer[position]) == 0) {// test for shrinkage;
            if(DEBUG) {
                printf("\tshrinkage!\n");
            }

            coefficient = getNextCoefficient(ds);
            if (coefficient == NULL) {
                printf("Could not embed all data\n");
                return -1;
            }

            for (m = position+1; m < F5_N; m++) {// shift all to the left;
                buffer[m-1] = buffer[m];
            }
            buffer[F5_N-1] = coefficient;// our new coefficient to test with;
            position = getPosition(buffer, bits);

            goto testPosition;
        }
    } else {
        if(DEBUG) {
                printf("\nPERFECT MATCH\n");
        }
    }
    
    if(DEBUG) {
        printf("\nFINAL buffer: ");
        for (m = 0; m < F5_N; m++) {
            printf("%i ", *(buffer[m]));
        }
        printf("\n");
    }

    if(DEBUG) {
        getPosition(buffer, bits);
        printf("\n--------------FINISHED-------------\n");
    }

    free(bits);

    return numBitsEncoded;
}
