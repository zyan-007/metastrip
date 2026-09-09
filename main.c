#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void print_usage(); // called when --help, -h or metastrip alone is written
int is_valid_subcommand_target(char*, int*, int*);
int is_valid_file(char* filename);
void check_dublicates(int*); // to check if dublicates in the command exist or not
void print_payload(FILE*, int, char);

int main(int argc, char* argv[]){
    FILE *file;
    char fileName[256]; // file name gets assigned as per the command

    int is_all_used = 0; // this will be used during printing so that missing commands are not printed
    /*
        metastrip show trialing file.jpg
        if trailing doesnot exit then it will be printed targets not trailing data: none
        
        but if metastrip show all file.jpg is used
        all available ones only should be printed nothing like app13: none should not be printed that's why is_all_used 
    */
    
    /*
        All valid targets sit in this
        starting from 0 which is app0 to app19, index 20 is for com, index 21 is for trailing
        
        index -> 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19
                   ^ this is for app1 if it is set to 1 that means no need to check for exif and xmp as they are included so skip 2 and 3
    if index 1 is 0  ^ exif is checked her at index 2 
                       ^ index 3 is check for xmp
                         ^ index 4 is where app2 starts
                           ^ index 5 is for icc if app2 is checked then index 5 is skipped else it is checked
                same for iptc which lies in index index 17 and app13 lies at index 16, if index 16 is 1 then 17 is skipped else it it checked

        hope you understand this one maybe in future as the project grow i change this whole indexing part
        but right now this is the best thing i could come up with.
    */
    int valid_targets[22] = {0};

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
            if (argc == 3){
                if (is_valid_subcommand_target(argv[2], valid_targets, &is_all_used) == 1){
                    printf("metastrip: no File Provided\n\n");
                    exit(2);
                }
                else{
                    FILE* check_file = fopen(argv[2], "r"); // if it's a file then checking if it's valid or not
                    is_valid_subcommand_target("all", valid_targets, &is_all_used); // for this condition it is assumed that all is passed as target

                    // comment this out later only for testing purpose
                    for(int i = 0; i <= 21; ++i)
                        printf("%d ", valid_targets[i]);
                    printf("\n");

                    if (check_file != NULL){ // [pending check] if it's a fill name this should be equivalent to all it should print everything
                        fclose(check_file);
                        snprintf(fileName, 256, "%s", argv[2]); // after checking it is assigned to the main fileName
                        // printf("check");
                    }
                    else{
                        printf("Invalid target provided, check 'metastrip --help'.\n\n");
                        exit(2);
                    }
                    
                }
            }
            else if (argc > 3){ // there should be at least three total args metastrip show filename.txt can add more valuesto show 
                int argNum = 2; // starting after subcommands
                int argTotal = argc;

                if((strcmp("show", argv[1]) == 0)){ 
                                     
                    if(argc != 4){
                        printf("metastrip: wrong usage, please check 'metastrip --help'\n\n");
                        exit(2);
                    }
                    else{
                        if(is_valid_subcommand_target(argv[2], valid_targets, &is_all_used) != 1){
                            printf("metastrip: wrong targets provided, please check 'metastrip --help'\n\n");
                            exit(2);
                        }
                        else{
                            // printf("check flow\n");

                            if((is_valid_file(argv[3]) == 1)){
                                snprintf(fileName, 256, "%s", argv[3]); // after checking it is assigned to the main fileName
                            }

                            // comment this out later only for testing purpose
                            for(int i = 0; i <= 21; ++i)
                                printf("%d ", valid_targets[i]);
                            printf("\n");
                        }
                    }

                }
                else{ // strip case [pending]

                }
            }
            else{
                printf("metastrip: no value provided to '%s'.\n", argv[1]);
                // later can add hint like in git [optional pending]
                exit(2);
            }
        }   
        else{
            printf("metastrip: %s is not a metastrip command. See 'metastrip --help'.\n\n", argv[1]);
            exit(2);           
        }     
    }

    
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

                if(byte1 == 0xFF && byte2 == 0xDA && valid_targets[21] == 1){ // once we hit this marker we check for any trailing data
                    // looking for trailing data

                    /* once trailing data discovered it will be printed and 
                     main array updated to -1, if not then it stays at 1, which will be later used to tell the user
                     later if it is not present, does not print if all tag is used though
                    */
                    if (is_all_used == 0) //prevents printing of trailing data: none incase all is used unless explicitly mentioned
                        valid_targets[21] = -1; 

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

                int read_char;
                int payload_length = length-2; 
                // com
                if((byte1 == 0xFF && byte2 == 0xFE)){
                    printf("COM\t%d bytes: ", payload_length);

                    if (is_all_used == 0)
                        valid_targets[20] = -1;


                    while(payload_length > 0){
                        read_char = getc(file);
                        printf("%c", ((read_char >= 32 && read_char <= 126) ? read_char : '.')); // non printable character are printed as .
                        payload_length--;
                    }

                    printf("\n");
                }
                // printing APP slots 
                else if((byte1 == 0xFF && (byte2 >= 0xE0 && byte2 <= 0xEF))){ 
                    
                    // This has a bug com is printed as app30 even if com is not explictly written in the command it should 
                    // ignore but isn't [important fix, major PENDING]
                    
                    if(byte2 == 0xE0 && valid_targets[0] == 1){ // app0
                        printf("APP0\t%d bytes: ", payload_length);

                        if(is_all_used == 0)
                            valid_targets[1] = -1;

                        print_payload(file, payload_length, read_char);
                            
                    }
                    
                }
                else{
                    fseek(file, length-2, SEEK_CUR);
                }
            }

        }
        else{
            printf("!! Image might not be jpg or might be corrupted !!\n");
            fclose(file);
            exit(1);
        }

        fclose(file);

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

int is_valid_subcommand_target(char* target, int* target_list, int* is_all_available){
    /*
    1.this function will check if the targets are valid or not should be only these
    app0 - app15, com, trailing , all , exif, xmp, icc, iptc

    2. checks if all is not used with any other target
        ex: metastrip show all,app0,app1 filename.jpg is invalid command

    3. checks for dublicates
        ex: metastrip show app0,app1,app0 filename.jpg is invalid command because of dublicate targets
    */

    char* end = strtok(target, ",");
    if (end == NULL) // if empty string
        return -1;

    int flag = 0; // check if valid all or not, all cannot be used with any other target
    int count = 0;
    int index = 0; // this is to send to check_dublicate and mark which index of target list to reach


    while(end != NULL){
        count++;

        if(count > 1){
            if(flag == 1 || (strcmp(end, "all") == 0)){
                printf("  Invalid use of all target, it can only be used alone\n");
                printf("\ti.e metastrip <command> all <file> [options]\n\n");
                exit(2);
            }
        }

        if (strcmp(end, "all") == 0){
            flag = 1;
            for(int i = 0; i <= 21; ++i)
                *(target_list+i) += 1;

            end = strtok(NULL, ",");
            *is_all_available = 1;
            continue;
        }

        if(strcmp(end, "app") == 0)
            return -1;
        else if(strncmp(end, "app", 3) == 0){ // checking if app0 to app 15
            char* app_end;
            long n = strtol(end+3, &app_end, 10); //converting any number that is after app to long
            if(*app_end == '\0' && n >= 0 && n <= 15){
                
                // updating the app segment in valid target array
                if (n <= 1){ // app0 to app1 covered here
                    index = n;
                    // printf("check");
                }
                    
                else if (n == 2) // app2 at index 4, skipping index 2, 3 for exif and xmp
                    index = 4;
                    
                else if (n >= 3 && n <= 13) // app3 to 13 
                    index = n+3; // 3 offset is adjusted according to exif, xmp, icc saved before with their respective segments
                
                else if (n == 14)
                    index = 18;
                
                else if (n == 15)
                    index = 19;

                *(target_list+index) += 1; // updating value by 1
                
                check_dublicates(target_list); //just check if dublicate app target are given

                end = strtok(NULL, ",");
                continue;
            }
            else 
                return -1;
        }
        // below checking all valid tags
        else if(strcmp(end, "com") == 0) {
            
            *(target_list+20) += 1; // com is at index 20
            check_dublicates(target_list); //just check if dublicatecom target are given


            end = strtok(NULL, ",");
            continue;
        }
        else if(strcmp(end, "xmp") == 0){
            *(target_list+3) += 1; // xmp is at index 3
            check_dublicates(target_list); //just check if dublicate xmp target are given

            end = strtok(NULL, ",");
            continue;

        }
        else if(strcmp(end, "exif") == 0){
            *(target_list+2) += 1; // exif is at index 2
            check_dublicates(target_list); //just check if exif xmp target are given

            end = strtok(NULL, ",");
            continue;  
        }
        else if(strcmp(end, "icc") == 0){
            *(target_list+5) += 1; // com is at index 5
            check_dublicates(target_list); //just check if dublicate icc target are given

            end = strtok(NULL, ",");
            continue;
        }
        else if(strcmp(end, "iptc") == 0){
            *(target_list+17) += 1; // com is at index 17
            check_dublicates(target_list); //just check if dublicate iptc target are given

            end = strtok(NULL, ",");
            continue;
        }
        else if(strcmp(end, "trailing") == 0){
            *(target_list+21) += 1; // com is at index 21 last index
            check_dublicates(target_list); //just check if dublicate trailing target are given

            end = strtok(NULL, ",");
            continue;
        }
        else{
            return -1;
        }
    }

    return 1;
}

int is_valid_file(char* filename){

    FILE* file = fopen(filename, "r");

    if(file == NULL){
        printf("!! File Does Not Exist !!\n\n");
        exit(1);
    }
    fclose(file);

    return 1;
}

void check_dublicates(int* target_list){

    for(int i = 0; i <= 21; ++i){
        if(*(target_list+i) > 1){
            printf("!! Dublicate targets are not allowed !!\n\n");
            exit(2);
        }
    }
}

void print_payload(FILE* file, int payload_length, char read_char){
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