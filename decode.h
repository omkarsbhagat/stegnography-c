/* decode.h */

#ifndef DECODE_H
#define DECODE_H

#include "types.h"
#include "common.h"

typedef struct _DecodeInfo
{
    char *stego_image_fname;   // input: stego.bmp
    char *output_fname;        // output: output.txt

    FILE *fptr_stego_image;
    FILE *fptr_output_file;

    char extn_secret_file[10]; // ".txt"
    int  extn_size;            // 4
    long size_secret_file;     // secret.txt size in bytes

    unsigned char image_data[8]; // buffer for 8 bytes while decoding
} DecodeInfo;

/* CLA validation for decode */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Open files for decoding */
Status open_decode_files(DecodeInfo *decInfo);

/* Main decode driver */
Status do_decoding(DecodeInfo *decInfo);

/* Individual decode steps */
Status decode_magic_string(DecodeInfo *decInfo);
Status decode_secret_file_extn_size(DecodeInfo *decInfo);
Status decode_secret_file_extn(DecodeInfo *decInfo);
Status decode_secret_file_size(DecodeInfo *decInfo);
Status decode_secret_file_data(DecodeInfo *decInfo);

#endif
