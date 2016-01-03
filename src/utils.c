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

#include "utils.h"

/**
* Ask and you shall receive
* @param cinfo libJPEG decompress object
* @param coefficients libJPEG virtual pointer
* @param component which component (Y, U or V)
* @param block_row
* @param block_column
* @param dct_row
* @param dct_column
* @return the coefficient requested or NULL
*/
short *getCoefficient(j_decompress_ptr cinfo, jvirt_barray_ptr *coefficients,
        int component, int block_row,
        int block_column, short dct_row,
        short dct_column) {

    // Make sure nothing insane is going on
    if (component >= cinfo->num_components || component < 0) return NULL;
    if (block_row >= cinfo->comp_info[component].height_in_blocks || block_row < 0) return NULL;
    if (block_column >= cinfo->comp_info[component].width_in_blocks || block_column < 0) return NULL;
    if (dct_row >= MAX_N || dct_row < 0) return NULL;
    if (dct_column >= MAX_N || dct_column < 0) return NULL;

    // calculate where our junk is
    jpeg_component_info *compptr = cinfo->comp_info + component;
    JBLOCKARRAY buffer = (cinfo->mem->access_virt_barray)(
            (j_common_ptr) & cinfo, coefficients[component],
            block_row, (JDIMENSION) 1, FALSE);
    JCOEFPTR blockptr = buffer[0][block_column];
    // give me the money!
    return blockptr + dct_row * MAX_N + dct_column;
}

/**
* Create an array of shorts
* @param N how man shorties do you want?
* @return the number of shorties you asked for
*/
short *makeShortArray(int N) {
    short *shortArray = calloc(N, sizeof(short));

    if(shortArray == NULL) {
        fprintf(stderr, "FAILED TO GET MEMORY");
    }   
    // give me some shorties
    return shortArray;
}

/**
* Create an array of short pointers
* @param N how man shorty pointers do you want?
* @return the number of shorty pointers you asked for
*/
short **makeShortPtrArray(int N) {
    short **shortArray = (short**)malloc(N*sizeof(short *));

    if(shortArray == NULL) {
        fprintf(stderr, "FAILED TO GET MEMORY");
    }   
    // give me those shorties rooms
    return shortArray;
}

/**
* How much can you hold?
* @param extractedMessage the decoded message file
* @param stegObject the stego object file
* @param prngKey your key
*/
void printCapacity(char *extractedMessage, char *stegObject, int prngKey) {
    DCTStream ds;
    FILE *msgFile;
    int coef_cnt = 0;
    int msgFileSize;
    int stegFileSize;

    if(EXIT_FAILURE == initDCTStream(&ds, stegObject, NULL, 0, 0, prngKey, 1)){
        return;
    }
    
    if (NULL == (msgFile = fopen(extractedMessage, "rb"))) {
        fprintf(stderr, "can't open %s\n", extractedMessage);
        return;
    }
    
    //  Get the stego message filesize
    fseek(msgFile, 0L, SEEK_END); 
    msgFileSize = ftell(msgFile);
    fseek(msgFile, 0L, SEEK_SET);
    
    //  Get the stego object filesize
    fseek(ds.inJPEGFile, 0L, SEEK_END); 
    stegFileSize = ftell(ds.inJPEGFile);
    fseek(ds.inJPEGFile, 0L, SEEK_SET);
    
    fprintf(stderr, "Encoded message file size to resulting compressed jpeg file size = %f %%\n",
            100 * (double) msgFileSize / stegFileSize);
    
    while (getNextCoefficient(&ds) != NULL) {
        coef_cnt++;
    }

    fprintf(stderr, "Encoded message file size to available dct size = %f%%\n",
            100 * (double) msgFileSize / (sizeof(short) * coef_cnt));
    
    fclose(msgFile);
    closeDCTStream(&ds);
}

/**
* You heard the man, start giving me informative information about this image!
* @param srcinfo libJPEG decompress struct
*/
void printImageInfo(j_decompress_ptr srcinfo) {
    int compnum;// which component?... All of them!

    fprintf(stderr, "\ncomponents: %d  image width: %d  image height: %d\n\n",
            srcinfo->num_components, srcinfo->image_width, srcinfo->image_height);

    for (compnum = 0; compnum < srcinfo->num_components; compnum++) {
        fprintf(stderr,
                " component: %d  width in blocks: %d  height in blocks: %d\n",
                compnum, srcinfo->comp_info[compnum].width_in_blocks,
                srcinfo->comp_info[compnum].height_in_blocks);
        fprintf(stderr, "\n");
    }
}

/**
* The noise, show it to me
* @param cover jpeg cover file
* @param stegObject jpeg stego file
* @param prngKey your key
*/
void printPSNR(char *cover, char *stegObject, int prngKey) {
    
    DCTStream ds1, ds2;
    initDCTStream(&ds1, cover, NULL, 0, 1, prngKey, 1);
    initDCTStream(&ds2, stegObject, NULL, 0, 1, prngKey, 1);
    
    double MSE, PSNR;
    short *coefficient1, *coefficient2;
    int squareSum = 0;
    
    while (1) {
        coefficient1 = getNextCoefficient(&ds1);
        coefficient2 = getNextCoefficient(&ds2);
        
        if (coefficient1 == NULL || coefficient2 == NULL) {
            break;
        }
        
        squareSum += pow((int)(*coefficient1 - *coefficient2), 2);
    }
   
    MSE = (double) squareSum / ds1.prng_max;
    PSNR = (double) 10 * log(pow(255, 2) / MSE);

    fprintf(stderr, "MSE = %f\n", MSE);
    fprintf(stderr, "PSNR = %f\n", PSNR);
    closeDCTStream(&ds1);
    closeDCTStream(&ds2);
}

/**
* Show me the numbers!
* @param cover the cover image
* @param stegObject stego object
* @param extractedMessage decoded message file
* @param prngKey what was your key?
*/
void printStats(char *cover, char *stegObject, char *extractedMessage, int prngKey)
{
    printCapacity(extractedMessage, stegObject, prngKey);
    printPSNR(cover, stegObject, prngKey);
}

/**
* Create an int arry for me
* @param x this many high
* @param y this many wide
* @return high x wide ints
*/
unsigned int **makeMeInt(int x, int y){
    unsigned int **intArray = (unsigned int**)malloc(x*sizeof(unsigned int*));

    if(intArray == NULL) {
        fprintf(stderr, "FAILED TO GET MEMORY");
    }   
    int z;

    for(z=0;z<y;z++) {
        intArray[z] = (unsigned int*)calloc(y, y*sizeof(unsigned int));

        if(intArray[z] == NULL) {
            fprintf(stderr, "FAILED TO GET MEMORY");
        }   
    }
    return intArray;
}
