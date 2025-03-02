/*
Name: Dheer Rudani
Date: 11/11/24
Discription: LSB Image Steganography
*/
#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "decode.h"
#include <unistd.h>

int main(int argc, char *argv[]) 
{
    //Step1: Collect the details sent through CLA
    EncodeInfo encInfo;
    uint img_size;
    if(argc < 2)
        {
            printf("INFO: Please pass valid arguments.\n");
            printf("\nINFO: Encoding - Minimum 4 arguments.\nUsage:- ./a.out -e source_image_file secret_data_file [Destination_image_file]\n");
            printf("\nINFO: Decoding - Minimum 3 arguments.\nUsage:- ./a.out -d source_image_file [Destination_image_file]\n");
            return e_failure;
        }
    //Step 2: Check whether the operation is encoding or decoding
    OperationType op = check_operation_type(argv[1],argc);

    //Step 3: Check the op type is e_encode
    //Step 4: If yes -> start encode, else Goto Step 5
    if(op == e_encode)
    {
        //Create structure variable
        EncodeInfo encInfo;
        printf("INFO: Selected Encoding, Encoding Started.\n");
        sleep(1);
        //Step 1: Read and validate arguments       
        Status test = read_and_validate_encode_args(argv, &encInfo);
        if(test == e_failure)
        {
            printf("Encoding failed!\n");
            return e_failure;
        }
        else
        {
            if(do_encoding(&encInfo) == e_failure)
            {
                printf("Encoding failed!\n");
                return e_failure;
            }
            printf("\n----------------------------------------\n");
            printf("INFO: Encoding completed successfully.\n");
            printf("----------------------------------------\n");
        }
    }
    
    //Step 5: Check the op type is e_decode
    //Step 6: If yes -> start decode, else Goto Step 7
    else if(op == e_decode)
    {
        //Create a structure variable
        DecodeInfo decInfo;
        printf("INFO: Selected Decoding, Decoding Started.\n");
        sleep(1);
        Status check = read_and_validate_decode_args(argv, &decInfo);
        if(check == e_failure)
        {
            printf("Decoding failed!\n");
            return e_failure;
        }
        else
        {
            if(do_decoding(&decInfo) == e_failure)
            {
                printf("Decoding failed!\n");
                return e_failure;
            }
            printf("\n----------------------------------------\n");
            printf("INFO: Decoding completed successfully.\n");
            printf("----------------------------------------\n");
        }
    }

    //Step 7: print Error! message and then STOP the process
    else
    {
        //printf("Error! Invalid input...\nEnter a correct operation type\n");
        return e_failure;
    }
    return e_success;
}

OperationType check_operation_type(char *argv, int argc)
{
    //Step 1: Compare argv with -e
    //Step 2: If yes -> return e_encode, else Goto Step 3
    if(!strcmp(argv, "-e"))
    {
        if(argc < 4)
        {
            printf("INFO: For Encoding Please pass minimum 4 arguments like ./a.out -e source_image_file secret_data_file [Destination_image_file]\n");
            return e_unsupported;
        } 
        return e_encode;
    }

    //Step 3: Compare argv with -d
    //Step 4: If yes -> return e_decode, else Goto Step 5
    else if(!strcmp(argv, "-d"))
    {
        if(argc < 3)
        {
            printf("INFO: For Decoding Please pass minimum 3 arguments like ./a.out -d source_image_file [Destination_image_file]\n");
            return e_unsupported;
        } 
        return e_decode;
    }

    //Step 5: return e_unsupported
    else
    {
        return e_unsupported;
    }
    
}