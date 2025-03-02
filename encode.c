#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "common.h"
#include <unistd.h>

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
    //printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    //printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

uint get_ext_size(EncodeInfo *encInfo)
{
    //Step1: Move the pointer to the extention
    //Step2: Count the size and return the count
    char *ptr = strstr(encInfo->secret_fname,".");
    strcpy(encInfo->extn_secret_file,ptr);
    return strlen(ptr);
}

uint get_file_size(FILE *fptr_secret)
{
    fseek(fptr_secret, 0, SEEK_END);
    return ftell(fptr_secret);
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

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    //Step 1: Check the argv[2] is .bmp or not
    //Step 2: If yes -> Goto Step 3, No -> print error & return e_failure
    //Step 3: Check the argv[3] is .txt or not
    //Step 4: If yes -> Goto Step 5, No -> print error & return e_failure
    //Step 5: Check the argv[4] is .bmp or not
    //Step 6: If yes -> Store the arguments into respective arguments and return e_success, No -> print error & return e_failure
    if(strstr(argv[2],".bmp") && strstr(argv[3],".txt"))
    {
        
        encInfo->src_image_fname = argv[2];
        encInfo->secret_fname = argv[3];
        if(argv[4] == NULL)
        {
            printf("\nINFO: Output file not mentioned. Default file name is added.\n");
            sleep(1);
            encInfo->stego_image_fname = "encoded_data.bmp";
        }
        else
        {
            if(strstr(argv[4],".bmp"))
            {
                encInfo->stego_image_fname = argv[4];
            }
            else
            {
                printf("Error in matching .bmp file.\n");
                return e_failure;
            }
        }
        printf("\nINFO: Read and validation is completed for Encoding.\n");
        sleep(1);
        return e_success;
    }
    else
    {
        if(strstr(argv[2],".bmp") == NULL)
        {
            printf("Error in matching .bmp file.\n");
        }
        else if(strstr(argv[3],".txt") == NULL)
        {
            printf("Error in matching .txt file.\n");
        }
        return e_failure;
    }
    
}

Status check_capacity(EncodeInfo *encInfo)
{
    //Step1: Find the size of source file
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    //printf("%d\n",encInfo->image_capacity);
    //Step2: Find the size of secret file extension
    encInfo->size_ext = get_ext_size(encInfo);
    //printf("%d\n",encInfo->size_ext);
    //Step3: Find the size of secret file
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    //printf("%d\n",encInfo->size_secret_file);
    //Step4: Check whether we have enough space to hide
    //i.e., size_of_src_file > (16(Magic_string) + 32(size of int) + (size_of_ext) * 8 + 32(size of int) + (size_of_data) * 8 + 54(Header) + 1(EOF))
    if(encInfo->image_capacity < (16 + 32 + (encInfo->size_ext * 8) + 32 + (encInfo->size_secret_file * 8) + 54 + 1))
    {
        return e_failure;
    }
    return e_success;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_stego_image)
{
    rewind(fptr_src_image); //To set the source file pointer to 0th position
    //Step1: Read 54 bytes of data from source image
    char str[55];
    int read, write;
    read = fread(str, 54, 1, fptr_src_image);
    if(read != 1)
    {
        printf("Failed to read the data.\n");
        return e_failure;
    }
    //Step2: Write the read data to destination image
    write = fwrite(str, 54, 1, fptr_stego_image);
    if(write != 1)
    {
        printf("Failed to write the data.\n");
        return e_failure;
    }
    return e_success;
}

void encode_byte_to_lsb(char ch, char *arr)
{
    //Step1: Clear LSB bit of arr[i]
    for(int i = 0; i < 8; i++)
    {
        //Step2: Get one bit from ch
        //Step3: Replace the bit
        arr[i] = arr[i] & ~(1);
        arr[i] = (arr[i]) | (((unsigned)(ch & (1<<i))) >> i);
    }
    //Step4: Goto to step1 until i > 7
}

void encode_size_to_lsb(int size, char *arr)
{
    // printf("size = %d\n", size);
    for(int i = 0; i < 32; i++)
    {
        arr[i] = arr[i] & (~(1));
        arr[i] = (arr[i]) | (((unsigned)(size & (1<<i))) >> i);
    }
}

Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char arr[8];
    for(int i = 0; i < size; i++)
    {
        //Step1: Read 8 byte of data from src file and store it in array
        if(fread(arr, 8, 1, fptr_src_image) != 1)
        {
            printf("INFO: Error in reading the source file\n");
            return e_failure;
        }
        //Step2: Call encode_byte_to_lsb(arr, data[0]);
        encode_byte_to_lsb(data[i], arr);
        //Step3: Write the encoded data to output.bmp
        if(fwrite(arr, 8, 1, fptr_stego_image) != 1)
        {
            printf("INFO: Error in writing in the output file\n");
            return e_failure;
        }
        //Step4: Repeat the above process for length of data times
    }
    return e_success;
}

Status encode_magic_string(char *magic, EncodeInfo *encInfo)
{
    //Step1:Call encode_data_to_image
    if(encode_data_to_image(magic, strlen(magic), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("Error in encoding the magic string\n");
        return e_failure;
    }
    return e_success;
}

Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char arr[32];
    //Step1: Read 32 bytes of data from the source file
    if(fread(arr, 32, 1, encInfo->fptr_src_image) != 1)
    {
        return e_failure;
    }
    //Step2: Call encode_size_to_lsb(ext_size, arr);
    encode_size_to_lsb(size, arr);
    //Step3: Write encoded datat to destination file
    if(fwrite(arr, 32, 1, encInfo->fptr_stego_image) != 1)
    {
        return e_failure;
    }
    return e_success;
}

Status encode_secret_file_extn(const char *str,EncodeInfo *encInfo)
{
    int size = strlen(str);
    //Step1: Encode the extension
    if(encode_data_to_image(encInfo->extn_secret_file, size, encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("Error in encoding the file extension\n");
        return e_failure;
    }
    return e_success;
}

Status encode_secret_file_size(int size, EncodeInfo *encInfo)
{
    char a[32] = {0};
    //Step1: Read 32 bytes of data from the source file
    if(fread(a, 32, 1, encInfo->fptr_src_image) != 1)
    {
        return e_failure;
    }
    //Step2: Call encode_size_to_lsb(ext_size, arr);
    encode_size_to_lsb(size,a);
    //Step3: Write encoded datat to destination file
    if(fwrite(a, 32, 1, encInfo->fptr_stego_image) != 1)
    {
        return e_failure;
    }
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    rewind(encInfo->fptr_secret);
    int size = encInfo->size_secret_file;
    char arr[size];
    if(fread(arr, size, 1, encInfo->fptr_secret) != 1)
    {
        printf("Error in reading the secret file data\n");
        return e_failure;
    }
    if(encode_data_to_image(arr, size, encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("Error in encoding the secret data\n");
        return e_failure;
    }
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char arr[1];
    while(fread(arr, 1, 1, fptr_src_image) > 0)
    {
        if(fwrite(arr, 1, 1, fptr_stego_image) != 1)
        {
            printf("INFO: Error in writing the remaining data\n");
            return e_failure;
        }
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    //Step1: Open the files
    if(open_files(encInfo) == e_failure)
    {
        return e_failure;
    }
    else
    {
        printf("\nINFO: All files are opened successfully.\n");
        sleep(1);
    }

    //STEP2:Check capacity to encode
    //STEP3:Check returned e_success or e_faiure
    //STEP4:If e_success -> GOTO STEP5, else -> print error and return e_failure
    if(check_capacity(encInfo) == e_failure)
    {
        printf("INFO: Insufficient source size\n");
        return e_failure;
    }
    else
    {
        printf("\nINFO: Capacity of all files is checked.\n");
        sleep(1);
    }

    //STEP5:Call copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image)
    //STEP6:Check returned e_success or e_faiure
    //STEP7:If e_success -> GOTO STEP8, else -> print error and return e_failure
    if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_failure)
    {
        printf("INFO: Failed to copy the .bmp header\n");
        return e_failure;
    }
    else
    {
        printf("\nINFO: bmp header copied successfully.\n");
        sleep(1);
    }
    
    //STEP8: Encode the magic string
    //STEP9: Check returned e_success or e_failure
    //STEP10: If e_success -> GOTO STEP11, else -> Print error and return e_failure
    if(encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        return e_failure;
    }
    else
    {
        printf("\nINFO: Magic String encoded successfully.\n");
        sleep(1);
    }

    //STEP11: Call encode_secret_file_extn_size(ext_size,/*File pointer*/)
    //STEP12: Check returned e_success or e_failure
    //STEP13: If e_success -> GOTO STEP14, else -> Print error and return e_failure
    if(encode_secret_file_extn_size(encInfo->size_ext, encInfo) == e_failure)
    {
        printf("Failed in encoding the secret file extension size\n");
        return e_failure;
    } 
    else
    {
        printf("\nINFO: Encode file extension size completed.\n");
        sleep(1);
    }

    //STEP14: Call encode_secret_file_extn(extn,/*File pointers*/)
    //STEP15: Check returned e_success or e_failure
    //STEP16: If e_success -> GOTO STEP17, else -> Print error and return e_failure
    if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo) == e_failure)
    {
        printf("Failed in encoding the secret file extension\n");
        return e_failure;
    }
    else
    {
        printf("\nINFO: Encode file extension is completed.\n");
        sleep(1);
    }

    //STEP17: Call encode_secret_file_size(long size,encInfo)
    //STEP18: Check returned e_success or e_failure
    //STEP19: If e_success -> GOTO STEP20, else -> Print error and return e_failure
    if(encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure)
    {
        printf("Failed in encoding the secret file size\n");
        return e_failure;
    }
    else
    {
        printf("\nINFO: Encode size of file completed.\n");
        sleep(1);
    }


    //STEP20: Call encode_secret_file_data(encInfo)
    //STEP21: Check returned e_success or e_failure
    //STEP22: If e_success -> GOTO STEP23, else -> Print error and return e_failure
    if(encode_secret_file_data(encInfo) == e_failure)
    {
        printf("Failed in encoding the secret file data\n");
        return e_failure;
    }
    else
    {
        printf("\nINFO: Encode file data completed.\n");
    }

    //STEP23: Copy the remaining the bytes of data
    //STEP24: Check returned e_success or e_failure
    //STEP25: If e_success -> return e_success, else -> Print error and return e_failure
    if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("Failed in encoding the remaining data\n");
        return e_failure;
    }
    else
    {
        printf("\nINFO: Remaining data copied successfully.\n");
        sleep(1);
    }
    return e_success;
}