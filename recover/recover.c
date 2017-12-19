#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t BYTE;

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./recover infile\n");
        return 1;
    }
    
    // a variable that stores consequent numbers of images retrived from the lost memory
    int image_number = 0;
    
    // open a card file
    char *infile = argv[1];
    FILE *card = fopen(infile, "r");
    
    if (card == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }
    
    // open an output image file
    FILE *image_file = NULL;
    
    // set a buffer array for the individual block of 512 bytes
    BYTE buffer[512] = {};
    
    while (1) 
    {
        // iterate over each byte in each block until the end of the file is reached
        for (int i = 0; i < 512; i++)
        {
            // close the file at the end
            if (feof(card))
            {
                fclose(card);
                
                // closing previously opened output file pointer
                if (image_file != NULL)
                {
                    fclose(image_file);
                }
                
                return 0;
            }
            
            // read one byte with each iteration
            fread(&buffer[i], sizeof(BYTE), 1, card);
        }
        
        // check if this file is a JPEG
        if (buffer[0] == 0xff && buffer[1] == 0xd8 && buffer[2] == 0xff && (buffer[3] == 0xe0 || buffer[3] == 0xe1))
        {
            // close the file and push it into the folder
            if (image_file != NULL)
            {
                fclose(image_file);
            }
            
            // create a char array for a pointer below
            char filename[8];
            
            sprintf(filename, "%03i.jpg", image_number);
            image_number++;
            
            // open the output file pointer
            if ((image_file = fopen(filename, "w")) == NULL)
            {
                printf("Could not write an image\n");
                return 3;
            }

            fwrite(&buffer[0], 512 * sizeof(BYTE), 1, image_file);
            
        }
        else if (image_file != NULL) 
        {
            fwrite(&buffer[0], 512 * sizeof(BYTE), 1, image_file);
        }
        
    }
    
    // close the card file
    fclose(card);
    
    return 0;
}