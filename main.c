#include <stdio.h>
#include <string.h>

int main(){
    char fileName[256];
    FILE *file;
    
    // file name input from the user
    fgets(fileName, 256, stdin);
    fileName[strcspn(fileName, "\n")] = '\0';

    file = fopen(fileName, "rb+");

    if(file != NULL){
        int byte1 = getc(file); // storing first byte should be FF
        int byte2 = getc(file); // storing second byte should be D8

        if (byte1 == 0xFF && byte2 == 0xD8){ // jpg images start form FF D8
            while(1){
                byte1 = getc(file);
                byte2 = getc(file);

                if(byte1 == 0xFF && byte2 == 0xDA) break; // reading till FF DA before compressed data only        
                if(byte1 == EOF || byte2 == EOF) break; // guardrail to prevent a bad file to break the program       

                // for reading length
                int high = getc(file); 
                int low = getc(file);

                /*
                let's say length is 16 in decimal
                high will be 0x00
                low will be 0x10
                two bytes need to be combined to be passed to fseek
                so combine these two bytes into one (high << 8) | low is done
                */
                int length = (high << 8) | low;
                // printf("%ld\n" , ftell(file)-4);

                // printing APP slots and COM
                if((byte1 == 0xFF && (byte2 >= 0xE0 && byte2 <= 0xEF)) || (byte1 == 0xFF && byte2 == 0xFE)){ 
                    printf("%02X %02X\t", byte1, byte2);

                    printf("%02X %02X\n", high, low);
                }

                fseek(file, length-2, SEEK_CUR);
                // printf("-- %d --\n", length-2);
            }

        }
        else{
            printf("!! Image might not be jpg or might be corrupted !!\n");
        }
    }
    else{
        printf("!! File Does not Exist !!\n");
    }

    fclose(file);

    return 0;
}