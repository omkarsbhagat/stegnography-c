/* decode.c */
#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

/* Open decode files */
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }

    decInfo->fptr_output_file = fopen(decInfo->output_fname, "w");
    if (decInfo->fptr_output_file == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->output_fname);
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    return e_success;
}

/* Validate decode args: ./a.out -d stego.bmp output.txt */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if (argv[2] == NULL || argv[3] == NULL)
    {
        printf("ERROR: Missing arguments for decoding\n");
        return e_failure;
    }

    if (strstr(argv[2], ".bmp") == NULL)
    {
        printf("ERROR: Input must be a .bmp file\n");
        return e_failure;
    }

    if (strstr(argv[3], ".txt") == NULL)
    {
        printf("ERROR: Output file should be .txt\n");
        return e_failure;
    }

    decInfo->stego_image_fname = argv[2];
    decInfo->output_fname = argv[3];

    return e_success;
}

/* decode one byte from 8 image bytes*/
Status decode_byte_from_lsb(unsigned char *image_buffer, unsigned char *data)
{
    unsigned char ch = 0;
    for (int i = 0; i < 8; i++)
    {
        unsigned char bit = (image_buffer[i] & 0x01);
        ch |= (bit << i);  
    }
    *data = ch;
    return e_success;
}

/* decode 'size' bytes and write into buffer 'data'  */
Status decode_data_from_image(char *data, int size, DecodeInfo *decInfo)
{
    for (int i = 0; i < size; i++)
    {
        size_t r = fread(decInfo->image_data, 8, 1, decInfo->fptr_stego_image);
        if (r != 1)
            return e_failure;

        unsigned char ch;
        if (decode_byte_from_lsb(decInfo->image_data, &ch) != e_success)
            return e_failure;

        data[i] = (char)ch;
    }
    return e_success;
}

/* Decode and verify magic string  */
Status decode_magic_string(DecodeInfo *decInfo)
{
    int magic_len = strlen(MAGIC_STRING);
    char buf[64]; 
    if (decode_data_from_image(buf, magic_len, decInfo) != e_success)
        return e_failure;

    buf[magic_len] = '\0';

    if (strncmp(buf, MAGIC_STRING, magic_len) != 0)
        return e_failure;

    return e_success;
}

/* Decode extension size */
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    unsigned char tmp[32]; 
    long size_val = 0;
    for (int i = 0; i < 32; i++)
    {
        int r = fgetc(decInfo->fptr_stego_image);
        if (r == EOF) return e_failure;
        unsigned char bit = (unsigned char)(r & 0x01);
        size_val |= ((long)bit << i);
    }

    decInfo->extn_size = (int)size_val;
    return e_success;
}

/* Decode extension string of length extn_size */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    if (decInfo->extn_size <= 0 || decInfo->extn_size >= (int)sizeof(decInfo->extn_secret_file))
        return e_failure;

    /* For each extension character we need to read 8 image bytes and reconstruct char */
    for (int k = 0; k < decInfo->extn_size; k++)
    {
        unsigned char ch = 0;
        for (int i = 0; i < 8; i++)
        {
            int r = fgetc(decInfo->fptr_stego_image);
            if (r == EOF) return e_failure;
            unsigned char bit = (unsigned char)(r & 0x01);
            ch |= (bit << i);
        }
        decInfo->extn_secret_file[k] = (char)ch;
    }
    decInfo->extn_secret_file[decInfo->extn_size] = '\0';
    return e_success;
}

/* Decode secret file size*/
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    long size_val = 0;
    for (int i = 0; i < 32; i++)
    {
        int r = fgetc(decInfo->fptr_stego_image);
        if (r == EOF) return e_failure;
        unsigned char bit = (unsigned char)(r & 0x01);
        size_val |= ((long)bit << i);
    }
    decInfo->size_secret_file = size_val;
    return e_success;
}

/* Decode the actual secret file data  and write into output file */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    if (decInfo->size_secret_file <= 0)
        return e_failure;

    /* Read each secret byte and write to output file as a character */
    for (long k = 0; k < decInfo->size_secret_file; k++)
    {
        unsigned char ch = 0;
        for (int i = 0; i < 8; i++)
        {
            int r = fgetc(decInfo->fptr_stego_image);
            if (r == EOF) return e_failure;
            unsigned char bit = (unsigned char)(r & 0x01);
            ch |= (bit << i);
        }
        /* Write decoded byte as ASCII/text into output file */
        if (fputc(ch, decInfo->fptr_output_file) == EOF)
            return e_failure;
    }
    return e_success;
}

/* Main decoding driver */
Status do_decoding(DecodeInfo *decInfo)
{
    
    if (open_decode_files(decInfo) != e_success)
    {
        printf("ERROR: Unable to open decode files\n");
        return e_failure;
    }
    printf("INFO: Files opened successfully\n");

    
    if (fseek(decInfo->fptr_stego_image, 54, SEEK_SET) != 0)
    {
        printf("ERROR: fseek failed\n");
        return e_failure;
    }
    printf("INFO: BMP header skipped\n");

    /* Decode magic string */
    if (decode_magic_string(decInfo) != e_success)
    {
        printf("ERROR: Magic string mismatch or not found\n");
        return e_failure;
    }
    printf("INFO: Magic string verified successfully\n");

    /* Decode extension size */
    if (decode_secret_file_extn_size(decInfo) != e_success)
    {
        printf("ERROR: Failed to decode extension size\n");
        return e_failure;
    }
    printf("INFO: Extension size = %d\n", decInfo->extn_size);

    /* Decode extension */
    if (decode_secret_file_extn(decInfo) != e_success)
    {
        printf("ERROR: Failed to decode extension\n");
        return e_failure;
    }
    printf("INFO: Extension decoded = %s\n", decInfo->extn_secret_file);

    /* Decode secret file size */
    if (decode_secret_file_size(decInfo) != e_success)
    {
        printf("ERROR: Failed to decode secret file size\n");
        return e_failure;
    }
    printf("INFO: Secret file size decoded = %ld bytes\n", decInfo->size_secret_file);

    /* Decode secret file data and write to output file */
    if (decode_secret_file_data(decInfo) != e_success)
    {
        printf("ERROR: Failed to decode secret file data\n");
        return e_failure;
    }
    printf("INFO: Secret file data written successfully\n");

    /* Close files */
    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output_file);

    return e_success;
}
