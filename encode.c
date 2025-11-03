#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
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

uint get_file_size(FILE *fptr)
{
    //  To Find the size of secret file data
    fseek(fptr, 0, SEEK_END);
    return ftell(fptr);
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{

    char *temp_ext;
    temp_ext = strstr(argv[2], ".bmp"); // to store bmp address
    // printf("%s\n", argv[2]);
    // printf("%s\n", temp_ext);
    // printf("%ld\n", temp_ext - argv[2]);
    // printf("%ld\n", strlen(argv[2]));
    if (((temp_ext - argv[2]) >= 1 && ((strlen(argv[2])) - 4) == temp_ext - argv[2])) // check if it returns null (second condition for .bmp...)
        encInfo->src_image_fname = argv[2];
    else
        return e_failure;

    // to validate extension of secret file
    char secret_f_ext[4][5] = {".txt", ".h", ".c", ".sh"};
    int flag = 0;
    temp_ext = strstr(argv[3], ".");
    // printf("%s\n", argv[3]);
    // printf("%s\n", temp_ext);
    // printf("%ld\n", temp_ext - argv[3]);
    if ((temp_ext - argv[3]) > 1)
    {
        encInfo->secret_fname = argv[3];
        for (int i = 0; i < 4; i++)
        {
            if (strcmp(temp_ext, secret_f_ext[i]) == 0)
            {
                strcpy(encInfo->extn_secret_file, temp_ext); // exten_scret_file is an array of char so we cant copy directl
                flag = 1;
               
            }
        }
        if (flag == 0)
            return e_failure;
    }
    else
        return e_failure;

    // to check output file name

    if (argv[4] == NULL)
        encInfo->stego_image_fname = "default.bmp";
    else
    {
        temp_ext = strstr(argv[4], ".bmp"); // to store bmp address
        // printf("%s\n", argv[4]);
        // printf("%s\n", temp_ext);
        // printf("%ld\n", temp_ext - argv[4]);
        // // printf("%ld\n", strlen(argv[4]));
        // if(temp_ext==NULL)
        // printf("Error : Encoded file should have .bmp extension\n");
        if (((temp_ext - argv[4]) >= 1) && (((strlen(argv[4])) - 4) == temp_ext - argv[4])) // check if it returns null second condition for extension like .bmp......
            encInfo->stego_image_fname = argv[4];
        else
            return e_failure;
    }
    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "❌ ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "❌ ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, " ❌ ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    char *extn = strstr(encInfo->secret_fname, encInfo->extn_secret_file);  // to check the extensionsize 
    int extn_size = strlen(extn);
    int total_bytes = 54 + (strlen(MAGIC_STRING) * 8) + 32 + (extn_size * 8) + 32 + (encInfo->size_secret_file * 8);
    //  printf("%d",total_bytes);
    if (encInfo->image_capacity > total_bytes)
        return e_success;
    else
        return e_failure;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char buffer[54];
    rewind(fptr_src_image);
    // printf("%ld and %ld\n",ftell(fptr_src_image),ftell(fptr_dest_image));
    fread(buffer, 54, 1, fptr_src_image);
    fwrite(buffer, 54, 1, fptr_dest_image);
    if (ftell(fptr_src_image) == ftell(fptr_dest_image))
    {
        return e_success;
    }
    else
        return e_failure;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    printf("magic start from :%ld\n",ftell(encInfo->fptr_src_image));
    char buffer[8];
    for (int i = 0; i < strlen(magic_string); i++) // loop runs till size of magic string
    {
        fread(buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(magic_string[i], buffer); // at a time single character send for encoding  , one character encoded with 8 bytes of secret data
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_stego_image))
    {
        return e_success;
    }
    else
        return e_failure;
}
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char buffer[32];

    fread(buffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(size, buffer);
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);

    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_stego_image))
        return e_success;
    else
        return e_failure;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buffer[8];
    for (int i = 0; i < strlen(file_extn); i++)
    {

        fread(buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(encInfo->extn_secret_file[i], buffer);
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_stego_image))
        return e_success;
    else
        return e_failure;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buffer[32];

    fread(buffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(file_size, buffer);
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);

    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_stego_image))
        return e_success;
    else
        return e_failure;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char buffer[8];
    char temp;
    rewind(encInfo->fptr_secret);
    for (int i = 0; i < encInfo->size_secret_file; i++)
    {
        fread(&temp, 1, 1, encInfo->fptr_secret);
        // printf("%c",temp);
        fread(buffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(temp, buffer);
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_stego_image))
        return e_success;
    else
        return e_failure;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch, 1, 1, fptr_src) == 1)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }
    if (ftell(fptr_src) == ftell(fptr_dest))
        return e_success;
    else
        return e_failure;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] = ((image_buffer[i] & (~1)) | (data >> i) & 1);
    }
}

Status encode_size_to_lsb(int size, char *imageBuffer)
{
    for (int i = 0; i < 32; i++) // because extension size integer data so 32
    {
        imageBuffer[i] = (imageBuffer[i] & (~1)) | ((size >> i) & 1);
    }
}

Status do_encoding(EncodeInfo *encInfo)
{
    printf("\n==================== ENCODING PROCESS STARTED ====================\n");

    if (open_files(encInfo) == e_success)
    {
        printf("✅ Files opened successfully.\n");

        if (check_capacity(encInfo) == e_success)
        {
            printf("✅ Image capacity verified.\n");

            if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
            {
                printf("✅ BMP header copied successfully.\n");

                if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    printf("✅ Magic string encoded successfully.\n");

                    if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo) == e_success)
                    {
                        printf("✅ Secret file extension size encoded successfully.\n");

                        if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
                        {
                            printf("✅ Secret file extension encoded successfully.\n");

                            if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
                            {
                                printf("✅ Secret file size encoded successfully.\n");

                                if (encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("✅ Secret file data encoded successfully.\n");

                                    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                                    {
                                        printf("✅ Remaining image data copied successfully.\n");
                                        printf("\n🔒 Encoding completed successfully.\n");
                                        printf("=================================================================\n");
                                        return e_success;
                                    }
                                    else
                                        printf("❌ Error: Unable to copy remaining image data.\n");
                                }
                                else
                                    printf("❌ Error: Unable to encode secret file data.\n");
                            }
                            else
                                printf("❌ Error: Unable to encode secret file size.\n");
                        }
                        else
                            printf("❌ Error: Unable to encode secret file extension.\n");
                    }
                    else
                        printf("❌ Error: Unable to encode secret file extension size.\n");
                }
                else
                    printf("❌ Error: Unable to encode magic string.\n");
            }
            else
                printf("❌ Error: Unable to copy BMP header.\n");
        }
        else
            printf("⚠️  Warning: Image does not have enough capacity for encoding.\n");
    }
    else
        printf("❌ Error: Unable to open required files.\n");

    printf("=================================================================\n");
    return e_failure;
}
