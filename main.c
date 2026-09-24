#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_FILE_NUMBER_GENERATE 1000 // max file number guardrail used in generate_output_filename 

void print_usage(); // called when --help, -h or metastrip alone is written
int is_valid_subcommand_target(char*, int*, int*);
int is_valid_file(char* filename);
void check_dublicates(int*); // to check if dublicates in the command exist or not
void print_payload(FILE*, int, char, int*, int, int, int);
void metastrip_show(FILE*, int*, int);
void metastrip_strip(FILE*, int*, int);
void generate_output_filename(char*, char*, int);
int check_output_file_exist(char*, char*, char*);

int main(int argc, char* argv[]){
    FILE *file;
    char fileName[256]; // file name gets assigned as per the command
    char output_filename[500]; // used with strip where the copy is stored

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
    else if(((strcmp(argv[1], "-v") == 0) || (strcmp(argv[1], "--version") == 0))){
        if(argc != 2){
            printf("'-v' or '--version' does not take value.\n\n");
            exit(2);
        }
        printf("Metastrip version 1.0.0\n");
        exit(0);
    }
    else if((strcmp(argv[1], "-h") == 0)  || (strcmp(argv[1], "--help") == 0)){
        if(argc == 2) // prevents someone from passing anything to --help, ex- metastrip --help something
            print_usage();
        else
            printf("'%s' does not take any value. See 'metastrip --help'\n\n", argv[1]);
        exit(0);
    }
    else{
        // checking if all the subcommands and flags are valid or not
        if((strcmp("show", argv[1]) == 0 ) || (strcmp("strip", argv[1]) == 0)){ // checking if valid subcommands
            if (argc == 3){
                if (is_valid_subcommand_target(argv[2], valid_targets, &is_all_used) == 1){
                    printf("metastrip: no File Provided\n\n");
                    exit(2);
                }
                else{
                    FILE* check_file = fopen(argv[2], "r"); // if it's a file then checking if it's valid or not
                    is_valid_subcommand_target("all", valid_targets, &is_all_used); // for this condition it is assumed that all is passed as target

                    if (check_file != NULL){
                        fclose(check_file);
                        snprintf(fileName, 256, "%s", argv[2]); // after checking it is assigned to the main fileName                    }

                        if(strcmp("strip", argv[1]) == 0){
                            generate_output_filename(fileName, output_filename, sizeof(output_filename));
                            // printf("output file: %s\n", output_filename); // testing purpose
                        }
                    }
                    else{
                        // printf("Invalid  provided, check 'metastrip --help'.\n\n");
                        printf("!! File Does Not Exist !!\n\n");
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
                        //     for(int i = 0; i <= 21; ++i)
                        //         printf("%d ", valid_targets[i]);
                        //     printf("\n");
                        }
                    }

                }
                else if((strcmp("strip", argv[1]) == 0)){
                    // printf("test\n");
                    if(argc == 4){
                    // printf("test\n");
                        if(is_valid_subcommand_target(argv[2], valid_targets, &is_all_used) != 1){
                            printf("metastrip: wrong usage, please check 'metastrip --help'\n\n");
                            exit(2);
                        }
                        if(is_valid_file(argv[3]) == 1){
                            snprintf(fileName, 256, "%s", argv[3]); // after checking it is assigned to the main fileName
                            generate_output_filename(fileName, output_filename, sizeof(output_filename));
                        }
                        else{
                            printf("!! File Does Not Exist !!\n\n");
                            exit(2);
                        }
                    }
                    else if(argc == 5){ // metastrip strip inputfile.jpg -o outputfile.jpg // no target check here as assumed all
                        // printf("test\n");
                        if(is_valid_file(argv[2]) == 1){
                            snprintf(fileName, 256, "%s", argv[2]); // after checking it is assigned to the main fileName
                        }
                        else{
                            printf("!! File Does Not Exist !!\n\n");
                            exit(2);
                        }

                        if(((strcmp(argv[3], "-o") == 0) )|| (strcmp(argv[3], "--output") == 0)){
                            if(check_output_file_exist(argv[4], argv[2], output_filename) != 1){
                                printf("!! The Output File Provided, Overwriting any existing file is forbidden !!");
                                exit(1);
                            }
                        }
                        else{
                            printf("metastrip: wrong usage, please check 'metastrip --help'\n\n");
                            exit(2);
                        }
                        
                        // printf("check2\n");

                        
                    }
                    else if(argc == 6){
                        // printf("check3\n");
                        if(is_valid_subcommand_target(argv[2], valid_targets, &is_all_used) != 1){
                            printf("metastrip: wrong usage, please check 'metastrip --help'\n\n");
                            exit(2);
                        }
                        if(is_valid_file(argv[3]) == 1){
                            snprintf(fileName, 256, "%s", argv[3]); // after checking it is assigned to the main fileName
                        }
                        else{
                            printf("!! File Does Not Exist !!\n\n");
                            exit(2);
                        }

                        if(((strcmp(argv[4], "-o") == 0) )|| (strcmp(argv[4], "--output") == 0)){
                            if(check_output_file_exist(argv[5], argv[2], output_filename) != 1){
                                printf("!! The Output File Provided, Overwriting any existing file is forbidden !!");
                                exit(1);
                            }
                        }
                        else{
                            printf("metastrip: wrong usage, please check 'metastrip --help'\n\n");
                            exit(2);
                        }

                    }
                    else{
                        printf("metastrip: wrong usage, please check 'metastrip --help'\n\n");
                        exit(2); 
                    }
                }
            }
            else{
                printf("metastrip: no value provided to '%s'.\n", argv[1]);
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
        if (strcmp(argv[1], "show") == 0){
            metastrip_show(file, valid_targets, is_all_used);
        }
        else if(strcmp(argv[1], "strip") == 0){
            printf("test -> strip");
            // metastrip_strip(file, valid_targets, is_all_used);
        }
    }

    return 0;
}

void metastrip_strip(FILE* file, int* valid_targets, int is_all_used){

}

int check_output_file_exist(char* filename, char* input_file_name, char* final_output_file){
    /*
        This function checks if the output file exist or not if it exist than as per the main rule of the program
        nothing can be overwritten, therefore 0 is returned
        else
        (true) 1 is returned
    */
    // [pending future update 23/9/26]--> auto detecting extension currently only jpg is written later in future if more extension
    // added then this function would need input filename as well to autodetect the extension as well

    /*
    two cases if wrong extension in output file provided is wrong compared to the input file
    if user didn't provide extension to outputfile
    if user provided right extension to the file then it's good to go

    all three have to be checked before passing to fopen
    */
    char extension_name_input[20];
    char* input_extension = strrchr(input_file_name, '.'); // since input file name is already checked before coming here so we know it is safe and contains an extension
    
    // extracting the extension name form the input file
    int i = 0;
    while(*input_extension != '\0'){ // there will always be a null character at the end of the filename, assumed 
        extension_name_input[i] = *input_extension;
        ++i;
        if (i == 19) // guard which will never fire but still in case
            break;
        input_extension++;
    }
    extension_name_input[i] = '\0'; // making sure extension name has a null character at the end

    char* output_extension = strrchr(filename, '.');
    if(output_extension == NULL){ // there is no extension in the outputfile we have to concatinate
        strcat(final_output_file, filename);
        strcat(final_output_file, extension_name_input);
    }
    else{ // if not that means extension is provided we have to check if output extension and input extension are same or not
        snprintf(final_output_file, 500, "%s", filename);
        char extension_name_output[20];
        i = 0;
        while(*output_extension != '\0'){ // there will always be a null character at the end of the filename, assumed 
            extension_name_output[i] = *output_extension;
            ++i;
            if (i == 19) // guard which will never fire but still in case
                break;
            output_extension++;
        }
        extension_name_output[i] = '\0'; // making sure extension name has a null character at the end

        if(strcmp(extension_name_input, extension_name_output) != 0){ // this means user provided wrong extension name in output incompatible
            printf("!! Extensions provided for input and output file are both incompatible, please provide same extension !!\n\n");
            exit(2);
        }
    }

    FILE *file = fopen(final_output_file, "rb");
    if (file != NULL){
        fclose(file);
        return 0; // filealready exist
    }
    printf("%s\n", final_output_file); //test
    return 1;
}

void generate_output_filename(char* input_filename, char* final_file, int output_file_size){
    /*
        Assumes input filename is valid and it exists
        this function works when no explicit output files are provided for commands like
        metastrip strip filename.jpg <- here only input file is provided on which operations will be performed but no outputfile to save at
        this function looks for existing file if any stripped_filename.jpg if exist then 
        it adds number up to MAX_FILE_NUMBER_GENERATE
        ex stripped_1_filename.jpg, stripped_50_filename.jpg keeps finding till it hits that file and updates the output_filename
    */

    char output_fileName[500] = "stripped_";
    // incase filename comes as .\test2.jpg so that .\doen't get appended
    if (input_filename[0] == '.' && (input_filename[1] == '/' || input_filename[1] == '\\'))  
        strcat(output_fileName, input_filename+2);
    else
        strcat(output_fileName, input_filename);

    FILE* file = fopen(output_fileName, "rb");

    if(file != NULL){ // this means file exist we have to change the name
        fclose(file); // closing the existing file
        int file_number = 1; // starting from stripped_1_filename.ext
        char str[20];

        while (file_number < MAX_FILE_NUMBER_GENERATE){ // guard rail added to 500 tries if nothing fits then user is asked to delete some files (in case ran as a script)
            snprintf(output_fileName, sizeof(output_fileName), "%s", "stripped_"); // renaming it
            snprintf(str, sizeof(str), "%d", file_number);
            strcat(output_fileName, str); // currently it is stripped_1 <- example
            strcat(output_fileName, "_"); // stripped_1_
            // stripped_1_filename.jpg
            if (input_filename[0] == '.' && (input_filename[1] == '/' || input_filename[1] == '\\'))  
                strcat(output_fileName, input_filename+2);
            else
                strcat(output_fileName, input_filename);

            file = fopen(output_fileName, "rb");
            if(file == NULL)
                break;
            else
                fclose(file);
            
            file_number++;
        }

        if(file_number == MAX_FILE_NUMBER_GENERATE){
            printf("Max file generation limit hit, please delete some existing file or use -o to generate name");
            exit(1);
        }
    }
    
    snprintf(final_file, output_file_size, "%s", output_fileName); // sending the final value to main
    
}

void metastrip_show(FILE* file, int* valid_targets, int is_all_used){
        /*
            This function implements the whole show functionality, since all input checks are already done
            in the main function, here we start reading the bytes from the images and based on requested <targets>
            we find markers and print them
        */

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
                int payload_length;
                // com
                if((byte1 == 0xFF && byte2 == 0xFE) && valid_targets[20] == 1){
                    payload_length = length - 2;
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

                // else if are from app0 - app15
                else if((byte1 == 0xFF && byte2 == 0xE0) && valid_targets[0] == 1) // app0
                    print_payload(file, length, read_char, valid_targets, is_all_used, 0, byte2); 


                else if((byte1 == 0xFF && byte2 == 0xE1) && (valid_targets[1] == 1 || valid_targets[2] == 1 || valid_targets[3] == 1)){ // app1
                
                    if (valid_targets[1] == 1){
                        print_payload(file, length, read_char, valid_targets, is_all_used, 1, byte2); 
                        
                        if(is_all_used == 0){ // 11 sept currenlty pending only app1,exif fixed but not for icc icptc
                            valid_targets[2] = -1;
                            valid_targets[3] = -1;
                        }
                        continue;
                    }

                    if(valid_targets[2] == 1){
                        printf("EXIF: Payload-legnth: %d\n", length-2);
                        long long t = length - 2;
                        char iden_str[6]; // identifier string
                        FILE* new_file = fopen("exif_save.txt", "w"); // new file to save the data
                        for(int i = 0; i < 6; ++i){ // to read exif in payload
                            iden_str[i] = getc(file);
                        }

                        // printf("%s\n", iden_str);

                        if(strcmp(iden_str, "Exif") == 0){
                            t -= 6;
                            char c;
                            int new_line = 0; // after printing every 100 character move to new line so you don't scroll horizontally for long
                            while(t--){
                                if(new_line > 100){
                                    new_line = 0;
                                    putc('\n', new_file);
                                }
                                c = getc(file);
                                // remove this if you wish to see raw data in file. [for v.1.2] i can give --hex option to print raw bytes or print only printable char [future idea] - not currenly inlemented
                                putc(((c >= 32 && c <= 126) ? c : '.'), new_file); // writing to file instead of printing in terminal
                                new_line++;
                            }

                            printf("Written exif (printable character only) to file 'exif_save.txt'\n");

                            fclose(new_file);
                            valid_targets[2] = -1;
                        }
                        else
                            fseek(file, t-6, SEEK_CUR);       
                        continue;                 
                    }                   
                    
                    if(valid_targets[3] == 1){
                        long long t = length - 2;
                        char iden_str[30]; // "http://ns.adobe.com/xap/1.0/" (29 chars) + null terminator

                        for(int i = 0; i < 30; ++i){ // to read xmp identifier in payload
                            iden_str[i] = getc(file);
                        }

                        if(strcmp(iden_str, "http://ns.adobe.com/xap/1.0/") == 0){
                            printf("XMP: Payload-legnth: %d\n", length-2);
                            t -= 30;

                            FILE* new_file = fopen("xmp_save.txt", "w"); // new file to save the data

                            char c;
                            int new_line = 0; // after printing every 100 character move to new line so you don't scroll horizontally for long
                            while(t--){
                                if(new_line > 100){
                                    new_line = 0;
                                    putc('\n', new_file);
                                }
                                c = getc(file);
                                // remove this if you wish to see raw data in file. [for v.1.2] i can give --hex option to print raw bytes or print only printable char [future idea] - not currenly inlemented
                                putc(((c >= 32 && c <= 126) ? c : '.'), new_file); // writing to file instead of printing in terminal
                                new_line++;
                            }
                            printf("Written xmp (printable character only) to file 'xmp_save.txt'\n");

                            fclose(new_file);
                            valid_targets[3] = -1;
                        }
                        else
                            fseek(file, t-30, SEEK_CUR);
                    }
                
                }

                else if((byte1 == 0xFF && byte2 == 0xE2) && (valid_targets[4] == 1 || valid_targets[5] == 1)){ // app2
                    if (valid_targets[4] == 1){
                        print_payload(file, length, read_char, valid_targets, is_all_used, 4, byte2);

                        if(is_all_used == 0)
                            valid_targets[5] = -1;
                        continue;
                    }

                    if(valid_targets[5] == 1){
                        long long t = length - 2;
                        char iden_str[12]; // "ICC_PROFILE" (11 chars) + null terminator

                        for(int i = 0; i < 12; ++i){ // to read icc identifier in payload
                            iden_str[i] = getc(file);
                        }

                        if(strcmp(iden_str, "ICC_PROFILE") == 0){
                            printf("ICC: Payload-legnth: %d\n", length-2);
                            t -= 12;

                            FILE* new_file = fopen("icc_save.txt", "w"); // new file to save the data

                            char c;
                            int new_line = 0;
                            while(t--){
                                if(new_line > 100){
                                    new_line = 0;
                                    putc('\n', new_file);
                                }
                                c = getc(file);
                                putc(((c >= 32 && c <= 126) ? c : '.'), new_file);
                                new_line++;
                            }
                            printf("Written icc (printable character only) to file 'icc_save.txt'\n");

                            fclose(new_file);
                            valid_targets[5] = -1;
                        }
                        else
                            fseek(file, t-12, SEEK_CUR);
                    }
                }

                else if((byte1 == 0xFF && byte2 == 0xE3) && valid_targets[6] == 1) // app3
                    print_payload(file, length, read_char, valid_targets, is_all_used, 6, byte2);

                else if((byte1 == 0xFF && byte2 == 0xE4) && valid_targets[7] == 1) // app4
                    print_payload(file, length, read_char, valid_targets, is_all_used, 7, byte2); 

                else if((byte1 == 0xFF && byte2 == 0xE5) && valid_targets[8] == 1) // app5
                    print_payload(file, length, read_char, valid_targets, is_all_used, 8, byte2);

                else if((byte1 == 0xFF && byte2 == 0xE6) && valid_targets[9] == 1) // app6
                    print_payload(file, length, read_char, valid_targets, is_all_used, 9, byte2);
                    
                else if((byte1 == 0xFF && byte2 == 0xE7) && valid_targets[10] == 1) // app7
                    print_payload(file, length, read_char, valid_targets, is_all_used, 10, byte2);

                else if((byte1 == 0xFF && byte2 == 0xE8) && valid_targets[11] == 1) // app8
                    print_payload(file, length, read_char, valid_targets, is_all_used, 11, byte2);

                else if((byte1 == 0xFF && byte2 == 0xE9) && valid_targets[12] == 1) // app9
                    print_payload(file, length, read_char, valid_targets, is_all_used, 12, byte2);  

                else if((byte1 == 0xFF && byte2 == 0xEA) && valid_targets[13] == 1) // app10
                    print_payload(file, length, read_char, valid_targets, is_all_used, 13, byte2);  

                else if((byte1 == 0xFF && byte2 == 0xEB) && valid_targets[14] == 1) // app11
                    print_payload(file, length, read_char, valid_targets, is_all_used, 14, byte2);  

                else if((byte1 == 0xFF && byte2 == 0xEC) && valid_targets[15] == 1) // app12
                    print_payload(file, length, read_char, valid_targets, is_all_used, 15, byte2); 

                else if((byte1 == 0xFF && byte2 == 0xED) && (valid_targets[16] == 1 || valid_targets[17] == 1)){ // app13
                    if (valid_targets[16] == 1){
                        print_payload(file, length, read_char, valid_targets, is_all_used, 16, byte2);

                        if(is_all_used == 0)
                            valid_targets[17] = -1;
                        continue;
                    }

                    if(valid_targets[17] == 1){
                        long long t = length - 2;
                        char iden_str[14]; // "Photoshop 3.0" (13 chars) + null terminator

                        for(int i = 0; i < 14; ++i){ // to read iptc identifier in payload
                            iden_str[i] = getc(file);
                        }

                        if(strcmp(iden_str, "Photoshop 3.0") == 0){
                            printf("IPTC: Payload-legnth: %d\n", length-2);
                            t -= 14;

                            FILE* new_file = fopen("iptc_save.txt", "w"); // new file to save the data

                            char c;
                            int new_line = 0;
                            while(t--){
                                if(new_line > 100){
                                    new_line = 0;
                                    putc('\n', new_file);
                                }
                                c = getc(file);
                                putc(((c >= 32 && c <= 126) ? c : '.'), new_file);
                                new_line++;
                            }
                            printf("Written iptc (printable character only) to file 'iptc_save.txt'\n");

                            fclose(new_file);
                            valid_targets[17] = -1;
                        }
                        else
                            fseek(file, t-14, SEEK_CUR);
                    }
                }

                else if((byte1 == 0xFF && byte2 == 0xEE) && valid_targets[18] == 1) // app14
                    print_payload(file, length, read_char, valid_targets, is_all_used, 18, byte2); 
             
                else if((byte1 == 0xFF && byte2 == 0xEF) && valid_targets[19] == 1) // app15
                    print_payload(file, length, read_char, valid_targets, is_all_used, 19, byte2);  
                               
                else
                    fseek(file, length-2, SEEK_CUR);
                
            }

            // printf("%d", sizeof(valid_targets)/sizeof(int));
            if(is_all_used == 0){
                for(int i = 0; i < (sizeof(valid_targets)/sizeof(int)); ++i){ 
                    if(valid_targets[i] == 1){
                        switch (i){
                        case 0:
                            printf("APP0 ");
                            break;

                        case 1:
                            printf("APP1 ");
                            break;

                        case 2:
                            printf("Exif ");
                            break;

                        case 3:
                            printf("Xmp ");
                            break;

                        case 4:
                            printf("APP2 ");
                            break;

                        case 5:
                            printf("Icc ");
                            break;

                        case 6:
                            printf("APP3 ");
                            break;

                        case 7:
                            printf("APP4 ");
                            break;

                        case 8:
                            printf("APP5 ");
                            break;

                        case 9:
                            printf("APP6 ");
                            break;

                        case 10:
                            printf("APP7 ");
                            break;

                        case 11:
                            printf("APP8 ");
                            break;

                        case 12:
                            printf("APP9 ");
                            break;

                        case 13:
                            printf("APP10 ");
                            break;

                        case 14:
                            printf("APP11 ");
                            break;

                        case 15:
                            printf("APP12 ");
                            break;

                        case 16:
                            printf("APP13 ");
                            break;

                        case 17:
                            printf("Iptc ");
                            break;

                        case 18:
                            printf("APP14 ");
                            break;

                        case 19:
                            printf("APP15 ");
                            break;

                        case 20:
                            printf("COM ");
                            break;

                        case 21:
                            printf("Trailing ");
                            break;
                    }
                        printf("not found\n");
                    }
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

void print_usage(){
    printf(
        "metastrip - view and remove metadata from JPEG files.\n\n"
        "Usage: metastrip <command> [target] <file> [options]\n\n"
        "Commands:\n"
        "  show                    Display metadata\n"
        "  strip                   Write a copy with metadata removed\n\n"
        "Targets:\n"
        "  all                     All segments (default when no target is given)\n"
        "  app0 ... app15          A specific APP segment\n"
        "  com                     Comment segment\n"
        "  trailing                Data after end of image\n"
        "  exif, xmp, icc, iptc    Match by payload identifier\n\n"
        "Options:\n"
        "  -h, --help              Show this help\n"
        "  -v, --version           Show the installed version\n"
        "  -o, --output <file>     Output filename (strip only)\n"
        "                          Default: stripped_<name>.jpg\n"
        "                          Refuses to overwrite an existing file\n\n"
        "Examples:\n"
        "  metastrip show photo.jpg\n"
        "  metastrip show exif photo.jpg\n"
        "  metastrip strip exif photo.jpg\n"
        "  metastrip strip photo.jpg -o clean.jpg\n"
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

void print_payload(FILE* file, int length, char read_char, int* target_list, int is_all_used, int target, int byte2){
    
    int payload_length = length - 2;
    printf("APP%d\t%d bytes: ", (byte2-0xE0), payload_length);

    if(is_all_used == 0)
        target_list[target] = -1;


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