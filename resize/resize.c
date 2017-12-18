#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./resize n infile outfile\n");
        return 1;
    }
    
    // BMP resizing factor
    int n = atoi(argv[1]);
    
    if (n < 1)
    {
        printf("BMP resizing factor must be a postive integer");
        return 1;
    }

    // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];

    // open input file 
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || 
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }
    
    // variables for scanlines
    int original_biWidth = bi.biWidth;
    int original_biHeight = bi.biHeight;
    int original_padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    
    // variables for resized image
    bi.biWidth = bi.biWidth * n;
    bi.biHeight = bi.biHeight * n;
    int resized_padding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    
    // new size parameters for resized image
    bi.biSizeImage = ((sizeof(RGBTRIPLE) * bi.biWidth) + resized_padding) * abs(bi.biHeight);
    bf.bfSize = bi.biSizeImage + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);

    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(original_biHeight); i < biHeight; i++)
    {
        for (int rows = 0; rows < n; rows++)
        {
            // iterate over pixels in scanline
            for (int j = 0; j < original_biWidth; j++)
            {
               for (int columns = 0; columns < n; columns++)
                {
                    // temporary storage
                    RGBTRIPLE triple;
        
                    // read RGB triple from infile
                    fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
        
                    // write RGB triple to outfile
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                    
                    if (columns != (n-1))
                    {
                        fseek(inptr, -sizeof(RGBTRIPLE), SEEK_CUR);
                    }
                }
            }
    
            // skip over padding, if any
            fseek(inptr, original_padding, SEEK_CUR);
    
            // then add it back (to demonstrate how)
            for (int k = 0; k < resized_padding; k++)
            {
                fputc(0x00, outptr);
            }
            
            if (rows != (n-1))
            {
                fseek(inptr, (-sizeof(RGBTRIPLE) * original_biWidth) - original_padding , SEEK_CUR);
            }
        }
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}