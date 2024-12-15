//--------------------------------------------------------------------------------
#include "../include/utilities.h"
#include "../uinclude/communication.h"
#include "../uinclude/struct_packet.h"
#include "../uinclude/functions.h"
#include "../include/student_client.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>



#define INT_MAX 1978 // Maximum data_size for packet data. (2048 - 70)


Packet** add_file_request(char* data, char* filename, char* directory, int channel){
    /*
        Creates a packet that requests the creation of a file, as per the "add remote file" function.
    INPUT :
        data : The content to be stored in the file.
        filename: the file's name.
        directory: the directory in which to store the file.
        channel: A socket file descriptor used to communicate with the server
    OUTPUT :
        A list of the packets required to send the data split into sections of less than INT_MAX size.
        If the server failed to receive the file (e.g because the filename already exists), the list contains only an ABORT error packet.
    */
    int datalen = strlen(data);

    int reqpacknum = 1;
    if (datalen > INT_MAX) reqpacknum = (datalen  / INT_MAX ) + 1; // number of packets required to store all data.
     

    Packet* pack0 = empty_packet(); // This is the "test packet". It will be sent with the INT_MAX first characters of the file to see if the file can be created.

    char *buffer = malloc(INT_MAX * sizeof(char));
    pack0->code = 2;
    strcpy(pack0->option1, filename);
    strcpy(pack0->option2, itoa(datalen, 10));
    slice(data, buffer, 0, INT_MAX);
    pack0-> data_size = strlen(buffer);
    strcpy(pack0->data_ptr, buffer);

    char pktbuff[MAX_PACKET_SIZE];
    packet_to_string(pack0, pktbuff);
    send_pkt(pktbuff, channel);

    // WAIT FOR THE SERVER ANSWER
    while (1) {
        if(recv_pkt(pktbuff, channel) == SUCCESS){
            break;
        }
    }

    Packet* status = empty_packet();
    string_to_packet(pktbuff, status);

    if(status->code != 0){ // a.k.a if something went wrong...
        Packet** list = calloc(1, sizeof(Packet));
        pack0->code = COMMAND_FAILS;
        list[0] = error_packet(COMMAND_FAILS); 
        return list;

    }
    else {
        Packet** list = calloc(reqpacknum-1, sizeof(Packet));

        //list[0] = pack0;
        for(int i = 1; i < reqpacknum; i++){
            Packet * out = empty_packet();
            char *buffer = malloc(INT_MAX * sizeof(char));
            out->code = 2;
            strcpy(out->option1, filename);
            strcpy(out->option2, itoa(datalen, 10));
            slice(data, buffer, i*INT_MAX, (i+1)*INT_MAX);
            out-> data_size = strlen(buffer);
            strcpy(out->data_ptr, buffer);
            list[i-1] = out;
        }
        return list; // SHOULD SEND ONLY THE ELEMENTS AFTER INDEX 0. (0 excluded)
    }

}


void received_from_server(Packet** received, char* directory){
    int packnum = 1;

    if(received[0]->code == CMD_PRINT){ // PRINT_N_LINES
        packnum= atoi(received[0]->option1);
        for (int i = 0; i < packnum; i++){
            printf("%s", received[i]->data_ptr);
        }
        printf("\n"); // TO DO maybe here put a printf("\n"); to visually close the file ?
        
    }
    else if(received[0]->code == CMD_GET){ // FETCH
        int fetchfilesize = atoi(received[0]->option2);
        if (fetchfilesize > INT_MAX) {packnum = (fetchfilesize  / INT_MAX ) + 1;} else {packnum = 1;}
        char* file = malloc(sizeof(char) * fetchfilesize);
        for (int i = 0; i < packnum; i++){
            file = cats(file, received[i]->data_ptr);
        }
        write_to_file(cats(directory, received[0]->option1), file);
    }
    else if(received[0]->code == CMD_LIST){ // LS
        packnum= atoi(received[0]->option1);
        char *lstring = malloc(sizeof(char) * packnum * INT_MAX); // Reconstructing the string sent on each packet.
        for (int i = 0; i < packnum; i++){
            lstring = cats(lstring, received[i]->data_ptr); // Create the string by concatenating
        }
        print_ls_format(lstring, strlen(lstring));
    }
    else{
        print_response(received[0]);
    }
}

void print_ls_format(const char *stri, int n) {
        /*
        Prints the received string in the desired ls format.
    INPUT :
        stri: the string to print
        n: the string's length.
    */
    
    int i = 0;

    while (i < n) {
        // Afficher le nom jusqu'à la virgule
        while (i < n && stri[i] != ',') {
            putchar(stri[i]);
            i++;
        }

        // Afficher une tabulation après le nom
        putchar('\t');
        i++; // Passer la virgule

        // Afficher la taille jusqu'à la prochaine virgule ou fin de chaîne
        while (i < n && stri[i] != ',') {
            putchar(stri[i]);
            i++;
        }

        // Passer la virgule (ou fin de chaîne)
        i++;

        // Afficher un saut de ligne après chaque couple
        putchar('\n');
    }
}

int wait_for_response(int channel){
    // WIP: This one should create a list of the packets received by the server after request.
    // Something goes wrong. It seems to get into a while true and never gets out of it. 
    
    char pktbuff[INT_MAX]; // 2048 + 1 to store '\0'
    //pktbuff[INT_MAX] = '\0';

    while (1){
        
        int error_listener = recv_pkt(pktbuff, channel);
        // CHECK IF THE FIRST PACKET IS RECEIVED
        
        if(error_listener == SUCCESS){ // Waits until it receives a packet.

            // DECLARE this variable only if needed. (Inside the "if packet is received")
            Packet* pkt = empty_packet();
            
            // CONVERT the string packet received to a struct packet
            int error_code_conversion = string_to_packet(pktbuff, pkt); 
            //strcpy(pktbuff, ""); // ???

            // HANDLING THE ERROR DUE TO THE CONVERSION IF THERE IS ONE
            if(error_code_conversion == BAD_PACKET_FORMAT) return BAD_PACKET_FORMAT;
            if(error_code_conversion == QUOTA_EXCEEDED) return QUOTA_EXCEEDED;

            // ADD the part here where we only receive an single packet error from the server
            // it should check the content of the packet received, then check if it's an error answer from the server
            // and then return from this function, given that we don't need to create a whole list of packet to read.
             
             
            // DETERMINE the number of packets to still receive
            int packnum = 1; // Initialize
            if(pkt->code == CMD_PRINT || pkt->code == CMD_LIST){ // Retrieves the number of packets that it should receive.
                packnum = atoi(pkt->option1);
            }
            else if(pkt-> code == CMD_GET){ // Retrieves the number of packets that it should receive.
                int fetchfilesize = atoi(pkt->option2);
                if (fetchfilesize > INT_MAX) {packnum = (fetchfilesize  / INT_MAX ) + 1;}
            }

           
            // GENERATE the list of packets which will be received.
            Packet ** PacketList = calloc(packnum, sizeof(Packet));
            printf("\n\nPRINT PAACKNUM %d \n\n", packnum);
            
            PacketList[0] = pkt;
            int i = 1;
            while(i < packnum){ // Waits until it receives all desired packets. This might be where things go wrong.
                if(recv_pkt(pktbuff, channel) == SUCCESS){
                    Packet * empty_packet_received = empty_packet();
                    string_to_packet(pktbuff, empty_packet_received);
                    PacketList[i] = empty_packet_received;
                    //strcpy(pktbuff, ""); // ???
                    print_string(PacketList[i]->data_ptr, PacketList[i]->data_size);
                    i++;
                }
            }
            printf("\n\nALL THE PACKETS HAVE BEEN RECEIVED\n\n");

            received_from_server(PacketList, CLIENT_DIRECTORY); // Calls the function that reads the list and executes the desired action (print lines, get file, etc).
            
            printf("ERROR F ?\n");
            
            break;
            
        }
        
        
        
        else if(error_listener == SUCCESS){
            printf("BUG\n");
            // TO DO
            return SUCCESS;
        } 
        
        printf("FINI E ?\n");
        break;
    
    }

    printf("FINI C ?\n");
    
    return SUCCESS;
}

int student_client(int channel, int argc, char *argv[]) {
    /*
        Implements a client-side program that interacts with a servernthrough a socket channel.
    INPUT :
        channel : A socket file descriptor used to communicate with the server
        argc : The number of command-line arguments
        argv : An array of strings containing the command-line arguments
    OUTPUT :
        CANNOT_READ : to exit
        
    */

    // Writing to a closed socket causes a SIfGPIPE signal, which makes the program exits. 
    // The following line inhibits this default behaviour.
    // Thus, writing to a cloned socket will simply return -1 with the EPIPE
    // error in errno and will therefore not cause an exit. 
    // (see the line with "EPIPE" in send_pkt in usrc/communication.c).
    signal(SIGPIPE, SIG_IGN);



    // Variables for command-line options
    int analyze_flag = 0;
    int interactive_flag = 0;
    char analyze_file[256] = {0};
    char directory[256] = {0};  // Directory MUST ENDS WITH '/'

    // Step 1: Parse command-line arguments, figuring out which mode to activate

    // Buffer to receive the command line
    char cmdline[128];
    // Buffer to build the packet to send (max size: 81)
    char sendbuf[MAX_PACKET_SIZE];

    for (int i = 3; i < argc; i++) {

        if (strcmp(argv[i], "-analyze") == 0) {
            if (analyze_flag || i + 1 >= argc) {
                // If the program encounters -analyze, it expects a value (commands.txt) immediately after it.
                // If i + 1 >= argc,there are no more arguments after argv[i]
                fprintf(stderr, "Error: Invalid or duplicate -analyze option\n");  // In case the user is messing with us
                return SYNTAX_ERROR;
            }
            analyze_flag = 1;
            strncpy(analyze_file, argv[++i], sizeof(analyze_file) - 1);
        }
        else if (strcmp(argv[i], "-interactive") == 0) {
            if (interactive_flag) {
                fprintf(stderr, "Error: Duplicate -interactive option\n");
                return SYNTAX_ERROR;
            }
            interactive_flag = 1;
        }
        else if (strcmp(argv[i], "-directory") == 0) {// DIRECTORY STRING MUST END WITH '/'
            if (directory[0] || i + 1 >= argc) { // BY DEFAULT directory is './'
                fprintf(stderr, "Error: Invalid or duplicate -directory option\n");
                return SYNTAX_ERROR;
            }
            set_client_directory(argv[++i]); // DEFINE the new directory to work with
            force_client_directory_format(); // ADD '/' at the end of directory given.
        }
        else {
            fprintf(stderr, "Error: Unknown option %s\n", argv[i]);  // User isn't making any sense
            return SYNTAX_ERROR;
        }
    }


    // Step 2: Handle -analyze option
    if (analyze_flag) {

        if (CLIENT_DIRECTORY == NULL) { // DEFAULT VALUE FOR CLIENT_DIRECTORY
            set_client_directory("./");
        }

        FILE *file = fopen(cats(CLIENT_DIRECTORY, analyze_file), "r");
        if (file == NULL) {
            printf("Error opening file");
            return FILE_NOT_FOUND;
        }

        char line[256];  // maximum length of the line
        // Print info to terminal

        // Infinite loop -> use ^C to exit the program
        while (fgets(line, 255, file)) { // FINITE TIMES (256 - 1 for '\0')
            
            // GENERATE Packet Command Line using a string format
            char cmd_to_packet_string[MAX_PACKET_SIZE];
            int error_code = convert_cmd_string_to_packet_string(line, cmd_to_packet_string);

            // MANAGING ERROR CODE
            if( error_code == CMD_QUIT ){
                printf("Quitting Client");
                return CMD_QUIT;
                //break;//quitting client
            }
            
            if( error_code == CMD_RESTART){
                printf("Restarting Client");
                return CMD_RESTART;
            }

            // SEND the packet
            int res = send_pkt(cmd_to_packet_string, channel);

            printf("\n\n\nENTER WAIT FOR RESPONSE\n\n\n");
            wait_for_response(channel);
            printf("FINI A ?\n");

        }
    }

    printf("FINI D ?\n");
    // Step 3: Handle -interactive option
    if (interactive_flag) {
        while (1) { // INFINITE TIMES
            char cmdline[256];

            // Get the command from user, exit if it fails
            printf("> ");  // Command prompt simulator XD
            if(! fgets(cmdline, 128, stdin)){
                printf("Cannot read command line\n");
                // Return CANNOT_READ to exit
                return CANNOT_READ;
            }

            // GENERATE Packet Command Line using a string format
            char cmd_to_packet_string[MAX_PACKET_SIZE];
            int error_code = convert_cmd_string_to_packet_string(cmdline, cmd_to_packet_string);

            // TO DO MANAGE ERROR CODE (HELP, QUIT, LEAVE etc...)
            if( error_code == CMD_QUIT ){
                printf("Quitting Client");
                return CMD_QUIT;
                //break;//quitting client
            }
            
            if( error_code == CMD_RESTART){
                printf("Restarting Client");
                return CMD_RESTART;
            }

            // SEND the packet
            if(error_code == SUCCESS ){
                int res = send_pkt(cmd_to_packet_string, channel);
            }

            // WAIT for a response from the server! (the function doesn't work yet)
            wait_for_response(channel);
            printf("C'est la qu'on sort ?\n");
        }
    }
    printf("FINI B ?\n");
    return SUCCESS; // USELESS
}
