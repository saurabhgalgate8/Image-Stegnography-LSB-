#include <stdio.h>
#include <string.h>
#include "decode.h"
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // Validation of encoded bmp file
    char *temp_ext;
    temp_ext = strstr(argv[2], ".bmp"); // to store bmp address
    // printf("%s\n", argv[2]);
    // printf("%s\n", temp_ext);
    // printf("%ld\n", temp_ext - argv[2]);
    // printf("%ld\n", strlen(argv[2]));
    if (((temp_ext - argv[2]) >= 1 && ((strlen(argv[2])) - 4) == temp_ext - argv[2])) // check if it returns null (second condition for .bmp...)
        decInfo->enc_image_fname = argv[2];
    else
        return e_failure;

    // check extracted output file name present or not
    if (argv[3] == NULL)
        strcpy(decInfo->extr_fname, "Decoded_file");
    else
    {
      strcpy(decInfo->extr_fname , strtok(argv[3],"'.'"));  
        // char *ext = strchr(argv[3], '.'); //!=NULL);
        // printf("extn :%s\n", ext);
        // if (ext == NULL)
        //     strcpy(decInfo->extr_fname, argv[3]);
        // else
        // {
        //     argv[3][ext - argv[3]] = '\0';
        //     strcpy(decInfo->extr_fname, argv[3]);
        // }
    }

    return e_success;
}
Status Create_output_file(DecodeInfo *decInfo)
{
    // Secret file
    decInfo->fptr_extr = fopen(decInfo->extr_fname, "w");
    // Do Error handling
    if (decInfo->fptr_extr == NULL)
    {
        perror("fopen");
        fprintf(stderr, "❌ ERROR: Unable to open file %s\n", decInfo->extr_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

/* Get File pointers for i/p and o/p files */
Status open_files_for_decoding(DecodeInfo *decInfo)
{

    // encoded  Image file open
    decInfo->fptr_enc_image = fopen(decInfo->enc_image_fname, "r");

    if (decInfo->fptr_enc_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, " ❌ ERROR: Unable to open file %s\n", decInfo->enc_image_fname);
        return e_failure;
    }
    else
        return e_success;
}

/* Store Magic String */
Status decode_magic_string(DecodeInfo *decInfo)
{
    char buffer[8];
    unsigned char ch;
    fseek(decInfo->fptr_enc_image, 54, SEEK_SET); // to set cursor at hidden data)to skip header)
    // printf("%ld",ftell(decInfo->fptr_enc_image));
    int i = 0;
    for (i = 0; i < 2; i++)
    {
        //  ch=0;
        fread(buffer, 8, 1, decInfo->fptr_enc_image);
        decode_byte_from_lsb(&ch, buffer);
        // printf("char =%c\n",ch);
        decInfo->magic_string[i] = ch;
    }
    decInfo->magic_string[i] = '\0';
    // printf("magic string :%s\n", decInfo->magic_string);
    if (strcmp(decInfo->magic_string, "#*") == 0)
        return e_success;
    else
    {
        printf("❌ Error :Magic string is not found");
        return e_failure;
    }
}

/*Encode extension size*/
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];
    int size;
    fread(buffer, 32, 1, decInfo->fptr_enc_image);
    decode_size_from_lsb(&size, buffer);
    // printf("exten size :%d\n",size);
    decInfo->extr_extn_size = size;
    return e_success;
}

/* decode secret file extenstion */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;
    int i;
    for (i = 0; i < decInfo->extr_extn_size; i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_enc_image);
        decode_byte_from_lsb(&ch, buffer);
        // printf("ext:%c\n",ch);
        decInfo->extn_extr_file[i] = ch;
    }

    //  ADDING extension

    decInfo->extn_extr_file[i] = '\0'; // to terminate exten string
    // printf("extension :%s\n",decInfo->extn_extr_file);    //


    strcat(decInfo->extr_fname, decInfo->extn_extr_file); // extension added
    // printf("file _name :%s\n",decInfo->extr_fname);

    if (Create_output_file(decInfo) == e_success) // to crate output file with correct_extension
        return e_success;
    else
        return e_failure;
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];
    int size;
    fread(buffer, 32, 1, decInfo->fptr_enc_image);
    decode_size_from_lsb(&size, buffer);
    decInfo->size_extr_file = size;

    // printf("size of secret file :%ld\n",decInfo->size_extr_file);
    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;
    for (int i = 0; i < decInfo->size_extr_file; i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_enc_image);
        decode_byte_from_lsb(&ch, buffer);
        fwrite(&ch, 1, 1, decInfo->fptr_extr);
    }
    return e_success;
}

Status decode_byte_from_lsb(unsigned char *data, char *image_buffer)
{

    for (int i = 0; i < 8; i++)
    {
        // *data = (*data | ((image_buffer[i] & 1) << i));
        *data = ((*data & (~(1 << i))) | ((image_buffer[i] & 1) << i));
        //  printf("c :%d\n",*data);
    }
    //    printf("char = %c\n",*data);
}

// decode a size to lsb
Status decode_size_from_lsb(uint *size, char *imageBuffer)
{

    for (int i = 0; i < 32; i++)
    {
        *size = ((*size & (~(1 << i))) | ((imageBuffer[i] & 1) << i));
    }
}

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo)
{
    printf("\n==================== DECODING PROCESS STARTED ====================\n");

    if (open_files_for_decoding(decInfo) == e_success)
    {
        printf("✅ Files opened successfully.\n");

        if (decode_magic_string(decInfo) == e_success)
        {
            printf("✅ Magic string decoded successfully.\n");

            if (decode_secret_file_extn_size(decInfo) == e_success)
            {
                printf("✅ Secret file extension size decoded successfully.\n");

                if (decode_secret_file_extn(decInfo) == e_success)
                {
                    printf("✅ Secret file extension decoded successfully.\n");

                    if (decode_secret_file_size(decInfo) == e_success)
                    {
                        printf("✅ Secret file size decoded successfully.\n");

                        if (decode_secret_file_data(decInfo) == e_success)
                        {
                            printf("\n🔓 Decoding completed successfully!\n");
                            printf("📄 Extracted secret file: %s\n", decInfo->extr_fname);
                            printf("=================================================================\n");
                            return e_success;
                        }
                        else
                            printf("❌ Error: Failed to decode secret file data.\n");
                    }
                    else
                        printf("❌ Error: Failed to decode secret file size.\n");
                }
                else
                    printf("❌ Error: Failed to decode secret file extension.\n");
            }
            else
                printf("❌ Error: Failed to decode secret file extension size.\n");
        }
        else
            printf("❌ Error: Failed to decode magic string.\n");
    }
    else
        printf("❌ Error: Unable to open files for decoding.\n");

    printf("=================================================================\n");
    return e_failure;
}
