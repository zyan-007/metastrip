#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    if(argc != 2){ // currently only 1 argument along with program name taken will update later
        fprintf(stderr, "Bad arguments");
        exit(2);
    }

    char fileName[256];
    snprintf(fileName, 256, "%s", argv[1]);
    FILE *file;
    
    // file name input from the user
    // fgets(fileName, 256, stdin); // now sending file name from terminal itself no need to take input
    // fileName[strcspn(fileName, "\n")] = '\0';

    file = fopen(fileName, "rb");

    if(file != NULL){
        int byte1 = getc(file); // storing first byte should be FF
        int byte2 = getc(file); // storing second byte should be D8

        if (byte1 == 0xFF && byte2 == 0xD8){ // jpg images start form FF D8
            printf("Segment\tPayload-length\n");

            while(1){
                byte1 = getc(file);
                byte2 = getc(file);

                if(byte1 == 0xFF && byte2 == 0xDA){ // once we hit this marker we check for any trailing data
                    // looking for trailing data
                    fseek(file, -1, SEEK_END); // start reading from the end
                    long long int filesize = ftell(file)+1;
                    int flag = 0; // only works if corrupted data or markers are bad

                    while(1){
                        byte2 = getc(file); // reading bytes from the end of the file
                        if(byte2 == 0xD9){
                            fseek(file, -2, SEEK_CUR);

                            if((byte2 = getc(file)) == 0xFF){
                                break;
                            }
                            fseek(file, 1, SEEK_CUR); // in case FF not found to prevent a condition like FF D9 D9 E3
                        }

                        // guard rail
                        if(ftell(file) <= 1){
                            flag = 1;
                            break;
                        }

                        fseek(file, -2, SEEK_CUR);
                    }

                    if(flag == 1){ 
                        fprintf(stderr, "Bad file"); // only fires up when there are bad markers
                        break;
                    };

                    if(filesize-(ftell(file)+1) != 0)
                        printf("Trailing data: %lld bytes\n", filesize-(ftell(file)+1)); // printing the size of the trailing data
                    else
                        printf("Trailing data: none\n");
                    break;
                }    
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

                        printf("\n");
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