#include <stdio.h>
#include "encode.h"
#include "types.h"
#include<string.h>
#include "decode.h" 
int main(int argc,char *argv[])
{
    //val cla
    if (argc < 2)
{
    printf("ERROR: No operation specified.\n");
    printf("USAGE:\n");
    printf("./a.out -e <input.bmp> <secret.txt> <stego.bmp>\n");
    printf("./a.out -d <stego.bmp> <output.txt>\n");
    return 1;
}
    EncodeInfo E1;
    OperationType choice = check_operation_type(argv);

    if (choice == e_encode)
    {
      
    printf("Encoding is selected\n");

    if (read_and_validate_encode_args(argv, &E1) == e_success)
    {
        printf("INFO : Read and Validate is success\n");

         /* Start encoding */
            if (do_encoding(&E1) == e_success)
            {
                printf("INFO : Encoding completed successfully\n");
            }
            else
            {
                printf("ERROR : Encoding failed\n");
                return 1;
            }
        }
    
    else
    {
        printf("ERROR : Read and Validate failed\n");
        return 1;   // stop program
    }
}

    
    else if (choice == e_decode)
    {
        DecodeInfo D1;
        printf("Decoding selected\n");

        if (read_and_validate_decode_args(argv, &D1) != e_success)
        {
            printf("ERROR: Invalid decode arguments\n");
            return 1;
        }

        if (do_decoding(&D1) == e_success)
            printf("INFO: Decoding successful\n");
        else
            printf("ERROR: Decoding failed\n");
    }

 
    
    
    
else if (choice == e_unsupported)
    {
        printf("Invalid option\n");
       
        printf("For Encoding : ./a.out -e beautiful.bmp secret.txt stego.bmp\n");
        printf("For Decoding : ./a.out -d stego.bmp output.txt\n");

    }

    return 0;

}



OperationType check_operation_type(char *argv[])
{
    // Check if user entered: -e
    if (strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }
    // Check if user entered: -d
    else if (strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}
