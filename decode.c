#include <stdio.h>
#include <string.h>
#include "common.h"
#include "decode.h"
#include <unistd.h>

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    //Step1: Check whether argv[2] is of .bmp or not
    //Step2: If yes -> return e_success, else print error and return e_failure
    if(strstr(argv[2],".bmp"))
    {
        decInfo->src_fname = argv[2];
    }
    else
    {
        printf("Error in matching the .bmp file\n");
        return e_failure;
    }

    if(argv[3] == NULL)
    {
        printf("\nINFO: Output file not mentioned. Default file name is added.\n");
        sleep(1);
        decInfo->sec_fname = "decoded_data";
    }
    else
    {
        decInfo->sec_fname = argv[3];
    }
    return e_success;
}

Status open_src_file(DecodeInfo *decInfo)
{
    decInfo->src_fptr = fopen(decInfo->src_fname,"r");
    if(decInfo->src_fptr == NULL)
    {
        fprintf(stderr,"Error in opening the file %s\n", decInfo->src_fname);
        return e_failure;
    }
    return e_success;
}

void clear_image_buffer(char *image_buffer)
{
    for(int i = 0; i < 32; i++)
    {
        image_buffer[i] = 0;
    }
}

void decode_byte_from_lsb(char *data, int size, DecodeInfo *decInfo)
{
    for(int j = 0; j < size; j++)
    {
        decInfo->arr[j] = decInfo->arr[j] & 0;
        for(int i = 0; i < 8; i++)
        {
            decInfo->arr[j] = decInfo->arr[j] | ((data[i] & 1)<<i);
        }
        data += 8;
    }
}

Status decode_magic_string(char *magic, DecodeInfo *decInfo)
{
    //Skipping the bmp header
    fseek(decInfo->src_fptr, 54, SEEK_SET);
    if(fread(decInfo->image_data, 16, 1, decInfo->src_fptr) != 1)
    {
        printf("Error in reading the magic string\n");
        return e_failure;
    }
    //Getting the magic string
    decode_byte_from_lsb(decInfo->image_data, 2, decInfo);
    if(strcmp(decInfo->arr, magic) != 0)
    {
        printf("Error the magic string not found\n");
        return e_failure;
    }
    //printf("Magic string found is %s\n",decInfo->arr);
    return e_success;
}

int decode_size_from_lsb(char *image_buff)
{
    uint data = 0;
    for(int i = 0; i < 32; i++)
    {
        data = data | ((image_buff[i] & 1) << i);
    }
    return data;
}

Status decode_ext_size(DecodeInfo *decInfo)
{
    clear_image_buffer(decInfo->image_data);
    //Read 32 byte of data to decode the ext_size
    if(fread(decInfo->image_data, (sizeof(int) * 8), 1, decInfo->src_fptr) != 1)
    {
        printf("Error in decoding the extension size\n");
        return e_failure;
    }
    decInfo->ext_size = decode_size_from_lsb(decInfo->image_data);
    return e_success;
}

Status decode_extension(DecodeInfo *decInfo)
{
    clear_image_buffer(decInfo->image_data);
    if(fread(decInfo->image_data, (decInfo->ext_size * 8), 1, decInfo->src_fptr) != 1)
    {
        printf("Error in decoding the extension\n");
        return e_failure;
    }
    decode_byte_from_lsb(decInfo->image_data, decInfo->ext_size, decInfo);
    strcpy(decInfo->ext, decInfo->arr);
    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    clear_image_buffer(decInfo->image_data);
    if(fread(decInfo->image_data, 32, 1, decInfo->src_fptr) != 1)
    {
        printf("Error in decoding the secret file size\n");
        return e_failure;
    }
    decInfo->sec_file_size = decode_size_from_lsb(decInfo->image_data);
    return e_success;
}

Status decode_seccret_file_data(DecodeInfo *decInfo)
{
    clear_image_buffer(decInfo->image_data);
    char file_name[30];
    sprintf(file_name, "%s%s", decInfo->sec_fname, decInfo->ext);
    decInfo->sec_fptr = fopen(file_name, "w");
    for(int i =0 ; i < decInfo->sec_file_size; i++)
    {
        if(fread(decInfo->image_data, 8, 1, decInfo->src_fptr) != 1)
        {
            printf("Error in decoding the secret data\n");
            return e_failure;
        }
        decode_byte_from_lsb(decInfo->image_data, decInfo->sec_file_size, decInfo);
        const char *arr = decInfo->arr;
        fprintf(decInfo->sec_fptr, "%s", arr);
    }
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    //Step1: Open the source file
    if(open_src_file(decInfo) == e_failure)
    {
        return e_failure;
    }
    else
    {
        printf("\nINFO: All files are opened successfully.\n");
        sleep(1);
    }

    //Step2: Call decode_magic_string()
    //Step3: Check returned e_failure or e_success
    //Step4: If e_success -> GOTO Step5, else print error and return e_failure
    if(decode_magic_string(MAGIC_STRING, decInfo) == e_failure)
    {
        printf("Failed to decode magic string\n");
        return e_failure;
    }
    else
    {
        printf("\nINFO: Magic String decoded successfully.\n");
        sleep(1);
    }
    
    //Step5: Call decode_ext_size()
    //Step6: Check returned e_failure or e_success
    //Step7: If e_success -> GOTO Step8, else print error and return e_failure
    if(decode_ext_size(decInfo) == e_failure)
    {
        printf("Failed to decode the extension size\n");
        return e_failure;
    }
    else
    {
        printf("\nINFO: Decode file extension size completed.\n");
        sleep(1);
    }

    //Step8: Call decode_extension()
    //Step9: Check returned e_failure or e_success
    //Step10: If e_success -> GOTO Step11, else print error and return e_failure
    if(decode_extension(decInfo) == e_failure)
    {
        printf("Failed to decode the extension\n");
        return e_failure;
    }
    else
    {
        printf("\nINFO: Decode file extension is completed.\n");
        sleep(1);
    }

    //Step11: Call decode_file_size()
    //Step12: Check returned e_failure or e_success
    //Step13: If e_success -> GOTO Step14, else print error and return e_failure
    if(decode_secret_file_size(decInfo) == e_failure)
    {
        printf("Failed to decode the secret file size\n");
        return e_failure;
    }
    else
    {
        printf("\nINFO: Decode size of file completed.\n");
        sleep(1);
    }

    //Step14: Call decode_file_size()
    //Step15: Check returned e_failure or e_success
    //Step16: If e_success -> GOTO Step17, else print error and return e_failure
    
    if(decode_seccret_file_data(decInfo) == e_failure)
    {
        printf("Failed to decode the secret data\n");
        return e_failure;
    }
    else
    {
        printf("\nINFO: Decode file data completed.\n");
        sleep(1);
    }

    return e_success;
}