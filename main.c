#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "decode.h"

OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("❌ Error: No arguments provided.\n");
        printf("\nUsage:\n");
        printf("  ▶️  To Encode: ./a.out -e <source.bmp> <secret.txt> [output.bmp]\n");
        printf("  ▶️  To Decode: ./a.out -d <stego.bmp> [output.txt]\n");
        printf("\nℹ️  Example:\n");
        printf("  ./a.out -e beautiful.bmp secret.txt encoded.bmp\n");
        printf("  ./a.out -d encoded.bmp\n");
        printf("\nDescription:\n");
        printf("  -e : Perform encoding (hide secret file inside image)\n");
        printf("  -d : Perform decoding (extract secret file from image)\n");
        printf("=================================================================\n");
        return 0;
    }

    EncodeInfo enc_info;
    DecodeInfo dec_info;

    if (check_operation_type(argv[1]) == e_encode)
    {
        if (argc < 4)
        {
            printf("❌ Error: Insufficient arguments for encoding.\n");
            printf("Usage: ./a.out -e <source.bmp> <secret.txt> [output.bmp]\n");
            return 0;
        }

        if (read_and_validate_encode_args(argv, &enc_info) == e_success)
        {
            do_encoding(&enc_info);
        }
        else
        {
            printf("❌ Error: Invalid arguments for encoding.\n");
            return 0;
        }
    }
    else if (check_operation_type(argv[1]) == e_decode)
    {
        if (argc < 3)
        {
            printf("❌ Error: Insufficient arguments for decoding.\n");
            printf("Usage: ./a.out -d <stego.bmp> [output.txt]\n");
            return 0;
        }

        if (read_and_validate_decode_args(argv, &dec_info) == e_success)
        {
            do_decoding(&dec_info);
        }
        else
        {
            printf("❌ Error: Invalid arguments for decoding.\n");
            return 0;
        }
    }
    else
    {
        printf("⚠️  Error: Unsupported operation '%s'\n", argv[1]);
        printf("Use -e for encoding or -d for decoding.\n");
        printf("Example:\n  ./a.out -e image.bmp secret.txt output.bmp\n");
        printf("           ./a.out -d encoded.bmp\n");
    }

    return 0;
}

OperationType check_operation_type(char *symbol)
{
    if (strcmp(symbol, "-e") == 0)
        return e_encode;
    else if (strcmp(symbol, "-d") == 0)
        return e_decode;
    else
        return e_unsupported;
}
