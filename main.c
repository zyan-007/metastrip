#include <stdio.h>
#include <string.h>

int main(){
    char fileName[256];
    FILE *file;
    
    // file name input from the user
    fgets(fileName, 256, stdin);
    fileName[strcspn(fileName, "\n")] = '\0';

    file = fopen(fileName, "rb");

    if(file != NULL){
        int byte1 = getc(file); // storing first byte should be FF
        int byte2 = getc(file); // storing second byte should be D8

        if (byte1 == 0xFF && byte2 == 0xD8){ // jpg images start form FF D8
            printf("Segment\tPayload-length\n");

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
                unsigned int length = (high << 8) | low;
                // printf("%ld\n" , ftell(file)-4);
                if (length < 2){
                    fprintf(stderr, "Segment length is incorrect\n");
                    break;
                }

                // printing APP slots and COM
                if((byte1 == 0xFF && (byte2 >= 0xE0 && byte2 <= 0xEF)) || (byte1 == 0xFF && byte2 == 0xFE)){ 
                    int read_char;
                    int payload_length = length-2; 
                    
                    if(byte2 == 0xFE){
                        printf("COM\t%d bytes: ", payload_length);

                        while(payload_length > 0){
                            read_char = getc(file);
                            printf("%c", ((read_char >= 32 && read_char <= 126) ? read_char : '.')); // non printable character are printed as .
                            payload_length--;
                        }
                    }
                    else{
                        printf("APP%d\t%d bytes: ", (byte2-0XE0), payload_length); 

                        int total_count = ((payload_length) < 32) ? (payload_length) : 32; // 32 bytes is a guardrail if the length of the bytes is less than 32 than printing till there
                        int count = total_count;
                        while(count > 0){
                            read_char = getc(file);
                            printf("%c", ((read_char >= 32 && read_char <= 126) ? read_char : '.')); // non printable character are printed as .
                            count--;
                        }
                        fseek(file, payload_length-(total_count), SEEK_CUR);
                        printf("\n");
                    }
                    
                }
                else{
                    fseek(file, length-2, SEEK_CUR);
                }
                // printf("-- %d --\n", length-2);
            }

        }
        else{
            printf("!! Image might not be jpg or might be corrupted !!\n");
        }

        fclose(file);

    }
    else{
        printf("!! File Does not Exist !!\n");
    }


    return 0;
}