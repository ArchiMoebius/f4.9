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

#include "DCTStream.h"
#include "utils.h"


/**
* Welp, give me another!
* @param ds dctstream object
* @return the next coefficient in the 'stream' / sequence
*/
short *getNextCoefficient(DCTStream *ds) {
    
    if (ds->index == 0 && ds->start == 1) {//  to skip or not to skip
        ds->start = 0;
        ds->index = (ds->index + ds->prng_prime) % ds->prng_max;
        return getCoefficient(&ds->srcinfo, ds->coef_arrays, 0, 0, 0, 0, 0);
    }
    
    while (ds->index != 0) {//  lets get working
        //  decode the index
        long block_index = ds->index / (MAX_N * MAX_N);
        int dct_index = ds->index % (MAX_N * MAX_N);

        //  skip the DC coefficients?
        if (ds->includeDC == 0 && dct_index == 0) {
            //  update the index
            ds->index = (ds->index + ds->prng_prime) % ds->prng_max;
            continue;
        }

        int dct_row = dct_index / MAX_N;
        int dct_column = dct_index % MAX_N;
        int component = 0, width, height;

        while (block_index >= 0) {
            width = ds->srcinfo.comp_info[component].width_in_blocks;
            height = ds->srcinfo.comp_info[component].height_in_blocks;

            if (block_index >= width * height) {
                block_index -= width * height;
            }
            //  valid index into blocks
            else {
                break;
            }
            component++;
        }

        int block_row = block_index / width;
        int block_column = block_index % width;

        short *coefficient = getCoefficient(&ds->srcinfo, ds->coef_arrays,
                component, block_row, block_column,
                dct_row, dct_column);

        //  update the index
        ds->index = (ds->index + ds->prng_prime) % ds->prng_max;

        //  skip zero coefficients?
        if (ds->includeZero == 0 && *coefficient == 0) {
            continue;
        }
        else {
            return coefficient;
        }
    }
    
    //  if we reach 0th index again then all coefficients permuted
    return NULL;
}

/**
* setup the stream pased in
* @param ds dctstream object
* @param srcJPEG cover file
* @param dstJPEG stego object
* @param dcterms use the DC terms?
* @param zeros use the zero terms?
* @param prime_key the prime key
* @param quality quality of the output image (disabled currently)
* @return did it work?
*/
short initDCTStream(DCTStream *ds, char* srcJPEG, char* dstJPEG, short dcterms, short zeros, int prime_key, int quality) {
    if(srcJPEG == NULL)//  you don't know what you are doing...
        return EXIT_FAILURE;

    //  Error handling
    ds->srcinfo.err = jpeg_std_error(&ds->jpegSrcErr);

    //  Create jpeg compression objects
    jpeg_create_decompress(&ds->srcinfo);

    //  Open source file
    if (NULL == (ds->inJPEGFile = fopen(srcJPEG, "rb"))) {
        fprintf(stderr, "\ninitDCTStream: Can't open %s\n", srcJPEG);
        return EXIT_FAILURE;
    }

    //  are we encoding or decoding?
    if(dstJPEG != NULL) {

        //  Open destination file
        if (NULL == (ds->outJPEGFile = fopen(dstJPEG, "wb"))) {
            fprintf(stderr, "\ninitDCTStream: Can't open %s\n", dstJPEG);
            return EXIT_FAILURE;
        }
        ds->dstinfo.err = jpeg_std_error(&ds->jpegDstErr);
        jpeg_create_compress(&ds->dstinfo);
    } else {
        ds->outJPEGFile = NULL;
    }

    jpeg_stdio_src(&ds->srcinfo, ds->inJPEGFile);
    jpeg_read_header(&ds->srcinfo, TRUE);
    ds->coef_arrays = jpeg_read_coefficients(&ds->srcinfo);

    if(dstJPEG != NULL) { //  are we encoding or decoding?
        jpeg_copy_critical_parameters(&ds->srcinfo, &ds->dstinfo);
        jpeg_stdio_dest(&ds->dstinfo, ds->outJPEGFile);

        if(quality < 0 || quality > 100) {
            fprintf(stderr, "\ninitDCTStream: Invalid quality value, skipping quality setting and utilizing the default\n");
        }else{
            //jpeg_set_quality(&ds->dstinfo, quality, FALSE);
            //libjpeg doesn't handle the quality setting correctly, everything goes to hell on the output image...
        }
    }

    ds->includeDC = dcterms;
    ds->includeZero = zeros;
    
    ds->start = 1;//  it has begun
    ds->index = 0;
    ds->prng_prime = prime_key;
    ds->prng_max = 0;
    
    int component;
    for (component = 0; component < ds->srcinfo.num_components; component++) {
        int width = ds->srcinfo.comp_info[component].width_in_blocks;
        int height = ds->srcinfo.comp_info[component].height_in_blocks;
        
        ds->prng_max += (long)MAX_N * MAX_N * width * height;
    }
    
    if (ds->prng_max % ds->prng_prime == 0) {//  how weird, it wasn't relative, make it so
        ds->prng_max--;
    }

    if(DEBUG){
        fprintf(stderr, "Total number of coefficients: %lu\n", ds->prng_max);
    }
    
    return EXIT_SUCCESS;
}

/**
* Shutdown time
* @param ds dctstream object
*/
void closeDCTStream(DCTStream *ds) {
    if(ds->outJPEGFile != NULL) {
        jpeg_write_coefficients(&ds->dstinfo, ds->coef_arrays);

        //  Cleanup output resources
        jpeg_finish_compress(&ds->dstinfo);
        jpeg_destroy_compress(&ds->dstinfo);
        
    }

    //  Cleanup input resources
    jpeg_finish_decompress(&ds->srcinfo);
    jpeg_destroy_decompress(&ds->srcinfo);

    if(ds->outJPEGFile != NULL)
        fclose(ds->outJPEGFile);

    if(ds->inJPEGFile != NULL)
        fclose(ds->inJPEGFile);
}

/**
* Print all of the coefficients out
* @param ds dctstream object
*/
void printCoefficients(DCTStream *ds){
    printf("\nCOEFFICIENTS:\n");
    short *coefficient;

    do {
        if(NULL != (coefficient = getNextCoefficient(ds))) {
            printf("%i ", *coefficient);
        }
    } while(coefficient != NULL);
}
