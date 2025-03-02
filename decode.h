#ifndef DECODE_H
#define DECODE_H 
#include "types.h"
#define MAX_LENGTH 4

typedef struct _DecodeInfo
{
    // Secret file extension and source image data information
    uint ext_size;
    char ext[MAX_LENGTH];   // Assuming MAX_LENGTH is the max size for the extension
    char image_data[32];    // Fixed-size array for image data

    // Source file and secret file information
    char *src_fname;        // Pointer to source file name
    uint sec_file_size;
    char *sec_fname;        // Pointer to secret file name

    // Files and common data
    FILE *src_fptr;         // File pointer for source image
    FILE *sec_fptr;         // File pointer for secret file
    char arr[50];           // Common array, possibly for decoded data

} DecodeInfo;

//To clear image buffer
void clear_image_buffer(char *image_buffer);

//Read and validate the source file passed through CLA
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

//Open the source file
Status open_src_file(DecodeInfo *decInfo);

//Start decoding
Status do_decoding(DecodeInfo *decInfo);

//Decode the magic string
Status decode_magic_string(char *magic_string, DecodeInfo *decInfo);

//Decode the extension size
Status decode_ext_size(DecodeInfo *decInfo);

//Decode the extension
Status decode_extension(DecodeInfo *decInfo);

//Decoding the data
void decode_byte_from_lsb(char *data, int size, DecodeInfo *decInfo);

//Decoding the size
int decode_size_from_lsb(char *image_buffer);

//Decode the secret file size
Status decode_secret_file_size(DecodeInfo *decInfo);

//Decode the secret file data
Status decode_seccret_file_data(DecodeInfo *decInfo);

#endif