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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "f5.h"
#include "utils.h"

void usage(char* fname) {
    printf("Usage: %s [action (e,d,s)] -i filepath -c filepath [-o output] -q value -k value -p value\n\n", fname);
    printf("q = quality\nc = cover\ni = input\n");
    printf("o = output\nk = KN_Scheme\np = prime key value\n\n");
    printf("-e: Encode\n");
    printf("\tThe only options which are valid with the encode switch are: -i -c -k -q and -p\n");
    printf("\tExample: %s -e -i ../data/secretInfo.txt -c ../data/cover.jpeg\n\n", fname);
    printf("-d: Decode\n");
    printf("\tThe only options which are valid with the decode switch are: -i -o -k -q and -p\n");
    printf("\tExample: %s -d -i ../data/stegoimage.jpeg -o decoded_data.txt\n\n", fname);
    printf("-s: Stats\n");
    printf("\tThe only options which are valid with the stats switch are: -i -c -o\n");
    printf("\tExample: %s -s -i ../data/stegoimage.jpeg -c ../data/cover.jpeg -o decoded_data.txt\n\n", fname);
    exit(-1);
}

int main(int argc, char **argv) {

    printf("F 4.9  Copyright (C) 2013\n");
    printf("This is free software, and you are welcome to redistribute it\n");
    printf("You should have received a copy of the GNU General Public License\n");
    printf("along with this program. GPL version 3 or later,\nIf not, see <http://www.gnu.org/licenses/>.\n\n");
    
    if(argc < 2) {
        usage(argv[0]);
    }

    char c;
    char* cover = NULL;
    char* inFile = NULL;
    char* outFile = NULL;
    char* outputJPEGName = NULL;

    int meK = 3;
    int prngKey = 31;
    int quality = 100;
    int ret = EXIT_FAILURE;

    enum {
        encode, decode, stats
    } operation;

    while ((c = getopt (argc, argv, "edsi:q:k:p:i:s:c:o:")) != -1) {

        switch (c) {
            case 'c':
                cover = optarg;
                break;
            case 'd':
                operation = decode;
                break;
            case 'e':
                operation = encode;
                break;
            case 'i':
                inFile = optarg;
                break;
            case 'k':
                meK = atoi(optarg);
                break;
            case 'o':
                outFile = optarg;
                break;
            case 'p':
                prngKey = atoi(optarg);
                break;
            case 'q':
                quality = atoi(optarg);
                break;
            case 's':
                operation = stats;
                break;
            case '?':

                if (isprint (optopt)) {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                    usage(argv[0]);
                }else{
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                    usage(argv[0]);
                }
            default:
                usage(argv[0]);
        }
    }
 
    switch(operation) {
        case decode:

            fprintf(stderr, "\nDecoding\n");

            if(EXIT_FAILURE == stegoF5(outFile, inFile, NULL, (stegoOperation) decodeData, prngKey, quality, meK)) {
                printf("\nSomething went wrong, please check your usage and try again!\n");
            } else {
                ret = EXIT_SUCCESS;
            }
            break;

        case encode:

            fprintf(stderr, "\nEncoding\n");

            if(outFile != NULL) {
                fprintf(stderr, "\nYou can't use the output flag when encoding!\n");
                exit(EXIT_FAILURE);
            }

            if(cover != NULL && inFile != NULL && outFile == NULL) {
                int len = strlen(cover);
                int maxLen = len + 500;//support for directories with long names
                outputJPEGName = calloc(maxLen, 1);
                strncat(outputJPEGName, cover, len);
                snprintf(outputJPEGName+len, maxLen, "_%i_q_%i_key_%i_k.jpeg", quality, prngKey,  meK);
                printf("\nValues:\n\tHide: %s inside of: %s with %i quality and %i as the key with %i as the K-N schema\n", inFile, cover, quality, prngKey, meK);
                printf("\nSaving files as: %s\n", outputJPEGName);
            }

            if(EXIT_FAILURE == stegoF5(inFile, cover, outputJPEGName, (stegoOperation) encodeData, prngKey, quality, meK)) {
                printf("\nSomething went wrong, please check your usage and try again!\n");
            } else {
                ret = EXIT_SUCCESS;
            }

            free(outputJPEGName);
            break;

        case stats:

            printStats(inFile, cover, outFile, prngKey);
            break;

        default:
            break;
    }

    return ret;
}
