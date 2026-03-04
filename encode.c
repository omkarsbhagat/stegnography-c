#include <stdio.h>
#include "encode.h"
#include "types.h"
#include<string.h>
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}


Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo){
 
    if (strstr(argv[2], ".bmp") != NULL){
        encInfo->src_image_fname=argv[2];
    }
    else{
        return e_failure;
    }
    
    if(strstr(argv[3],".txt") !=NULL){
        encInfo->secret_fname=argv[3];
    }
     else{
        return e_failure;
    }
    if (argv[4] != NULL)
{
    encInfo->stego_image_fname = argv[4];
}
else
{
    encInfo->stego_image_fname = "default.bmp";
}
return e_success;

}

uint get_file_size(FILE *fptr)
{
    uint size;

    fseek(fptr, 0, SEEK_END);
    size = ftell(fptr);

    return size;
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity =get_image_size_for_bmp(encInfo->fptr_src_image);

    encInfo->size_secret_file =get_file_size(encInfo->fptr_secret);

    if (encInfo->image_capacity >(16 + 32 + 32 + 32 + (encInfo->size_secret_file * 8)))
    { return e_success;

    }
    else{
        return e_failure;
    }
}
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    // fread from source
    char str[54];

    fseek(fptr_src_image, 0, SEEK_SET);
    fread(str, 54, 1, fptr_src_image);

    // write to destination
    fwrite(str, 54, 1, fptr_dest_image);

    return e_success;
}
Status encode_byte_to_lsb(char data, char *image_buffer){
       for (int i = 0; i < 8; i++)
    {
        unsigned char bit = (data >> i) & 1;
        image_buffer[i] = (image_buffer[i] & 0xFE) | bit;   
        
    }
    return e_success;
}
Status encode_size_to_lsb(int size,EncodeInfo *encInfo){
    char str[32];
    fread(str,32, 1,encInfo->fptr_src_image);
     for (int i = 0; i < 32; i++)
    {
         
        str[i] = (str[i] & 0xFE) | ((size >> i) & 1);
    }
     fwrite(str,32, 1,encInfo->fptr_stego_image);
    return e_success;

}
Status encode_data_to_image(char *data, int size, EncodeInfo *encInfo){
    for (int i = 0; i < size; i++){
    fread(encInfo->image_data,8, 1,encInfo->fptr_src_image);
    encode_byte_to_lsb(data[i], encInfo->image_data);
     fwrite(encInfo->image_data,8, 1,encInfo->fptr_stego_image);

}
return e_success;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo){
    encode_data_to_image(MAGIC_STRING, strlen(MAGIC_STRING),encInfo);
    return e_success;
}
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo){
    encode_size_to_lsb(strlen(encInfo->extn_secret_file),encInfo);
    return e_success;

}

Status encode_secret_file_extn(char *file_extn, EncodeInfo *encInfo){
    encode_data_to_image(file_extn,strlen(file_extn),encInfo);
    return e_success;
}
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo){
 encode_size_to_lsb(file_size,encInfo);
    return e_success;

}
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char str[encInfo->size_secret_file];

    fseek(encInfo->fptr_secret, 0, SEEK_SET);

    fread(str, 1, encInfo->size_secret_file, encInfo->fptr_secret);

    encode_data_to_image(str, encInfo->size_secret_file, encInfo);

    return e_success;
}

Status copy_remaining_img_data( EncodeInfo *encInfo){
int rem_bytes=54+encInfo->image_capacity-ftell(encInfo->fptr_src_image);
char str[rem_bytes];
fread(str,rem_bytes,1,encInfo->fptr_src_image);
fwrite(str,rem_bytes,1,encInfo->fptr_stego_image);
return e_success;

}

Status do_encoding(EncodeInfo *encInfo)
{
    // 1. Open files
    if (open_files(encInfo) == e_success)
    {
        printf("Files are opened successfully\n");

        if(check_capacity(encInfo)==e_success){
            printf("Check capacity is success\n");
        
            if (copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
            {
                printf("Copied BMP header successfully\n");

                if(encode_magic_string(MAGIC_STRING,encInfo)==e_success){
                printf("Encoded magic string successfully.\n");
                
                strcpy(encInfo->extn_secret_file, strstr(encInfo->secret_fname,"."));
                printf("%s",encInfo->extn_secret_file);
       
                    if( encode_secret_file_extn_size( strlen(encInfo->extn_secret_file), encInfo)==e_success){
                     printf("Encode file extension size successfull.\n");
                        if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo)==e_success){
                            printf("Encode file extension  successfull.\n");
                            
                            if(encode_secret_file_size(encInfo->size_secret_file,encInfo)==e_success){
                             printf("Encode file size  successfull.\n");
                                if(encode_secret_file_data(encInfo)==e_success){
                                    printf("Encode secret file data successfull.\n");
                                    if(copy_remaining_img_data(encInfo)==e_success){
                                        printf("Copy remaining data successfull.\n");
                                    }
                                    else{
                                        printf("Copy remaining data unsuccessfull.\n");
                                    }
                                 }
            
                                else{
                                printf("Encode secret file data unsuccessfull.\n");
                                return e_failure;
                                }

                            }
                            
                            else{
                            printf("Encode file size unsuccessful.\n");
                            return e_failure;
                                }

                        }
            
                        else{
                        printf("Encode file ext unsuccessful.\n");
                            return e_failure;
                            }
            

                    }
                    else{
                        printf("Encode file ext size unsuccessful.\n");
                        return e_failure;
                    }


                 }

                else{
                    printf("Failed to encode magic string.\n");
                    return e_failure;
                }
            }
            else
            {
                printf("Failed to copy bmp header\n");
                return e_failure;
            }

         }
        else{
             printf("Check capacity is failure\n");
                return e_failure;
            }
        }
    
    else
    {
        printf("ERROR : Unable to open files\n");
        return e_failure;
    }



    return e_success;
}