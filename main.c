#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void print_usage(); // called when --help, -h or metastrip alone is written

int main(int argc, char* argv[]){
    if(argc == 1){
        print_usage();
        // fprintf(stderr, "Bad arguments");
        exit(2);
    }
    else if((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)){
        if(argc == 2) // prevents someone from passing anything to --help, ex- metastrip --help something
            print_usage();
        else
            printf("%s does not take any value. See 'metastrip --help'\n\n", argv[1]);
        exit(0);
    }
    else{
        // checking if all the subcommands and flags are valid or not

        if((strcmp("show", argv[1]) == 0 )|| (strcmp("strip", argv[1]) == 0)){ // checking if valid subcommands
            if (argc >= 3){ // there should be at least three total args metastrip show filename.txt can add more valuesto show 
                int argNum = 2; // starting after subcommands
                int argTotal = argc;

                if((strcmp("show", argv[1]) == 0)){ 
                    /*
                    we start at position 2 which is value or file name
                    metastrip show <target> filename or metastrip show filename
                                      ^ we are here         or           ^ here  depending on the argument passed
                    we need to extract the value so arc-1, one being removed is the filename       
                    [pending] --hexdump will fail this currently it is not being added       
                    */
                    argTotal = argc-1;
                    char* end;
                    int all_flag = 0; // can't use value 'all' with other values i.e metastrip show app0 

                    // while (argNum < argTotal){ // fix this it values seprated by space should be comma seprated
                    //     if(strncmp(argv[argNum], "app", 3) == 0){ // checking app0 .. app15
                    //         long num = strtol(argv[argNum]+3, &end, 10);

                    //         if (!(*end == '\0' && (num >= 0 && num <= 15) && !(strcmp(argv[argNum], "app") == 0))){ // checks if app is from 0 to 15 and if it's not just app
                    //             printf("Invalid app target, should be app0 ... app15\n");
                    //             exit(2);
                    //         }
                    //     }
                    //     argNum++;
                    // }

                    


                }
                else{ // strip case

                }
            }
            else{
                printf("Nothing specified, nothing added.\n");
                fprintf(stderr, "hint: Maybe you wanted to say 'metastrip show filename.jpg'\n"); // will add a different color later on like git [pending - decoration]
                exit(2);
            }
        }   
        else{
            printf("metastrip: %s is not a metastrip command. See 'metastrip --help'.\n\n", argv[1]);
            exit(2);           
        }     
    }

    
    // no flag's being used till now only print_usage used for normal metastrip, not for --help -h pending

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
            fclose(file);
            exit(1);
        }

        fclose(file);

    }
    else{
        printf("!! File Does not Exist !!\n");
        exit(1);
    }


    return 0;
}


void print_usage(){
    printf(
        "metastrip - view and remove metadata from JPEG files.\n\n"
        "Usage: metastrip <command> [target] <file> [options]\n\n"
        "Commands:\n"
        "  show                    Display metadata\n"
        "  strip                   Write a copy with metadata removed\n\n"
        "Targets:\n"
        "  all                     All segments (default for show)\n"
        "  app0 ... app15          A specific APP segment\n"
        "  com                     Comment segment\n"
        "  trailing                Data after end of image\n"
        "  exif, xmp, icc, iptc    Match by payload identifier\n\n"
        "Options:\n"
        "  -h, --help              Show this help\n"
        "      --hexdump           Print raw bytes as hex (show only)\n"
        "  -o <file>               Output filename (strip only)\n"
        "                          Default: <name>_stripped.jpg\n\n"
        "Examples:\n"
        "  metastrip photo.jpg\n"
        "  metastrip show exif photo.jpg\n"
        "  metastrip show app3 photo.jpg --hexdump\n"
        "  metastrip strip exif photo.jpg\n"
        "  metastrip strip all photo.jpg -o clean.jpg\n\n"

        "Output goes to stdout and can be redirected:\n"
        "  metastrip show exif photo.jpg > meta.txt\n"
        "The original file is never modified.\n"
    );
}

