#ifndef DECODE_H
#define DECODE_H
#include<stdio.h>
#include "types.h" // Contains user defined types

/*
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

typedef struct _DecodeInfo
{
    //Magic string
    // int magic_string_size;
    char magic_string[5];

    /* decode file Image info */
    char *enc_image_fname; // To store the encoded image name
    FILE *fptr_enc_image;  // To store the address of the encoded image
    

    /*Extracted File Info */
    char extr_fname[20];     // To store the extracted file name
    FILE *fptr_extr;        // To store the extracted file address
    int extr_extn_size;
    char extn_extr_file[5]; // To store the extracted file extension
    char extr_data[100];    // To store the extracted data
    long size_extr_file;    // To store the size of the secret data

 

} DecodeInfo;

/* Encoding function prototype */

/* Read and validate Encode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the encoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_files_for_decoding(DecodeInfo *decInfo);

/* Store Magic String */
Status decode_magic_string( DecodeInfo*decInfo);

/*decode extension size*/
Status decode_secret_file_extn_size( DecodeInfo *decInfo);

/* decode secret file extenstion */
Status decode_secret_file_extn( DecodeInfo *decInfo);

/* decode secret file size */
Status decode_secret_file_size( DecodeInfo *decInfo);

/* decode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo);

/* decode a byte into LSB of image data array */
Status decode_byte_from_lsb(unsigned char *data, char *image_buffer);

// decode a size to lsb
Status decode_size_from_lsb(uint *size, char *imageBuffer);



#endif
