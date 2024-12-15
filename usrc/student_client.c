//--------------------------------------------------------------------------------
#include "../include/utilities.h"
#include "../uinclude/communication.h"
#include "../uinclude/struct_packet.h"
#include "../uinclude/functions.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>


/* help message for commandline options */
const char * const client_help_options = "\
 client options are:\n\
\n\
 -interactive :\n\
	interactive mode ('better' interface messages)\n\
\n\
 -analyze filename :\n\
	reads commands from filename.\n\
\n\
 -directory string :\n\
	   specifies the directory to be used to store files. If this\n\
	   directory is non empty when the client starts, then existing\n\
	   files are assumed to be part of the local drive.\n\
\n\
";

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
    pack0-> data_ptr = buffer;

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
            out->data_ptr = buffer;
            list[i-1] = out;
        }
        return list; // SHOULD SEND ONLY THE ELEMENTS AFTER INDEX 0. (0 excluded)
    }

}

int student_client_old(int channel, int argc, char *argv[]) {
    /*
        Implements a client-side program that interacts with a server.
    INPUT :
        argc : The number of command-line arguments
        *argv[] : An array of strings containing the command-line arguments
    OUTPUT :
        0 : Sucess
        -1 : Bad packet format
    */

    // Ignore SIGPIPE signals
    signal(SIGPIPE, SIG_IGN);

    // Variables for command-line options
    int analyze_flag = 0;
    int interactive_flag = 0;
    char analyze_file[256] = {0};
    char directory[256] = {0};  // Directory MUST ENDS WITH '/'

    // Check if the user is asking for help
    /*
    if(argc == 2 && strcmp(argv[1], "help") == 0 ){
        printf("\n%s", client_help_options);
    };
    */

    // Step 1: Parse command-line arguments, figuring out which mode to activate
    for (int i = 3; i < argc; i++) {

        if (strcmp(argv[i], "-analyze") == 0) {
            if (analyze_flag || i + 1 >= argc) {
                // If the program encounters -analyze, it expects a value (commands.txt) immediately after it.
                // If i + 1 >= argc,there are no more arguments after argv[i]
                fprintf(stderr, "Error: Invalid or duplicate -analyze option\n");  // In case the user is messing with us
                return -1;
            }
            analyze_flag = 1;
            strncpy(analyze_file, argv[++i], sizeof(analyze_file) - 1);
        }
        else if (strcmp(argv[i], "-interactive") == 0) {
            if (interactive_flag) {
                fprintf(stderr, "Error: Duplicate -interactive option\n");
                return -1;
            }
            interactive_flag = 1;
        }
        else if (strcmp(argv[i], "-directory") == 0) {
        // DIRECTORY STRING MUST END WITH '/'
            if (directory[0] || i + 1 >= argc) { // BY DEFAULT directory is './'
                fprintf(stderr, "Error: Invalid or duplicate -directory option\n");
                return -1;
            }
            strncpy(directory, argv[++i], sizeof(directory) - 1);
        }
        else {
            fprintf(stderr, "Error: Unknown option %s\n", argv[i]);  // User isn't making any sense
            return -1;
        }
    }

    // Step 2: Handle -analyze option
    if (analyze_flag) {
        printf("Executing commands from file: %s\n", analyze_file);

        // Open the file
        FILE *file = fopen(analyze_file, "r"); 

        char line[256];  // maximum length of the line
        //char *packet = NULL;   <-- USELESS NOW

        // Take each line of the file
        while (fgets(line, sizeof(line), file)) {
	        // Process the command
            printf("Processing command: %s\n", line);

            // Convert the command into a packet
            Packet* package = malloc(sizeof(Packet)); // Allocate memory for the packet

            if (package == NULL) {
                printf("Memory allocation failed. Exiting.\n");
                continue;  // Exit if memory allocation fails
            }

            // Check if the command can be converted to a packet
            if (CmdlinetoPacket(line, package) == -1) {
                printf("Error: Command not recognized. Please try again.\n");
                free(package);  // Free the memory to avoid memory leak
                continue;  // Skip to the next loop iteration
            }

            // Print the packet for debugging (optional)
            // print_packet(package);

            // Converting the packet into a string for sending
            char* package_string = malloc((70 + package->data_size)*sizeof(char));
            packet_to_string(package, package_string);

            // Sending the packet to the server
            send_pkt(package_string, channel);

            // Receiving the response from the server
            // Total packet size must not exceed 2048 bytes, including the header.

            
            char* answer_string = malloc((2048)*sizeof(char));
            
            int response_code = recv_pkt(answer_string, channel);

            treat_response_code(response_code);
    
            if(response_code == SUCCESS){
                // Convert the command into a packet
                Packet* answer_package = malloc(sizeof(Packet));  // Allocate memory for the packet

                if (answer_package == NULL) {
                    printf("Memory allocation failed. Exiting.\n");
                    continue;  // Exit if memory allocation fails
                }

                // Check if the command can be converted to a packet
                if (CmdlinetoPacket(answer_string, answer_package) == -1) {
                    printf("Error: Command not recognized. Please try again.\n");
                    free(answer_package);  // Free the memory to avoid memory leak
                    continue;  // Skip to the next loop iteration
                }
                print_response(answer_package);
            }

            // Free the allocated memory after processing the command
            free(package);
            free(answer_string);
            free(package_string);
        }
        // Close the file
        fclose(file);
    }


    // Step 3: Handle -interactive option
    if (interactive_flag) {
        printf("Entering interactive mode...\n");
        
    // Continuous loop for interactive mode
    while (1) {
        char command[256];
        printf("> ");  // Command prompt simulator XD

        // Read a command from the command line
        if (fgets(command, sizeof(command), stdin) == NULL) {
            printf("Error reading input.\n");
            continue;
        }
        command[strcspn(command, "\n")] = 0;  // Remove the newline character

        // Check for an exit command
        if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0) {
            printf("Exiting interactive mode.\n");
            break;// Ahah! Break not continue (yes, i made the mistake)
        }

        // Process the command
        printf("Processing command: %s\n", command);

        // Convert the command into a packet
        Packet* package = malloc(sizeof(Packet));  // Allocate memory for the packet
        if (package == NULL) {
            printf("Memory allocation failed. Exiting.\n");
            continue;  // Exit if memory allocation fails
        }

        // Check if the command can be converted to a packet
        if (CmdlinetoPacket(command, package) == -1) {
            printf("Error: Command not recognized. Please try again.\n");
            free(package);  // Free the memory to avoid memory leak
            continue;  // Skip to the next loop iteration
        }

        // Print the packet for debugging (optional)
        // print_packet(package);


        // Converting the packet into a string for sending
        char package_string[2048];  // Maximum number of bytes is 2048
        packet_to_string(package, package_string);

        // Sending the packet to the server
        send_pkt(package_string, channel);

        // Receiving the response from the server
        // Total packet size must not exceed 2048 bytes, including the header
        char* answer_string = malloc((2048)*sizeof(char));

        int response_code = recv_pkt(answer_string, channel);

        treat_response_code(response_code);
    
        if(response_code == SUCCESS){
            // Convert the command into a packet
            Packet* answer_package = malloc(sizeof(Packet)); // Allocate memory for the packet

            if (answer_package == NULL) {
                printf("Memory allocation failed. Exiting.\n");
                continue; // Exit if memory allocation fails
            }

            // Check if the command can be converted to a packet
            if (CmdlinetoPacket(answer_string, answer_package) == -1) {
                printf("Error: Command not recognized. Please try again.\n");
                free(answer_package);  // Free the memory to avoid memory leak
                continue;  // Skip to the next loop iteration
            }
            print_response(answer_package);
            }

        // Free the allocated memory after processing the command
        free(package);
        free(answer_string);
        }
    }

    // Step 4: Handle -directory option
    if (directory[0]) {
        printf("Using directory: %s\n", directory);
        // TODO: Check existing files and process as needed
    }

    // TODO: Populate the packet structure based on file input
    // TODO: Send the packet using send_pkt()
    return 0; // Success
}



void received_from_server(Packet** received, char* directory){
    int packnum;

    if(received[0]->code == CMD_PRINT){ // PRINT_N_LINES
        packnum= atoi(received[0]->option1);
        for (int i = 0; i < packnum; i++){
            printf("%s", received[i]->data_ptr);
        }
    }
    else if(received[0]->code == CMD_GET){ // FETCH
        int fetchfilesize = atoi(received[0]->option2);
        if (fetchfilesize > INT_MAX) {packnum = (fetchfilesize  / INT_MAX ) + 1;} else {packnum = 1;}
        char* file = malloc(sizeof(char) * fetchfilesize);
        for (int i = 0; i < packnum; i++){
            file = cats(file, received[i]->data_ptr);
        }
        write_to_file(received[0]->option1, file, directory);
    }
    else if(received[0]->code == CMD_LIST){ // LS
        packnum= atoi(received[0]->option1);
        for (int i =0; i < packnum; i++){
            for(int j = 0; j < received[i]->data_size; j++){
                bool tab = true;
                if( (received[i]->data_ptr)[j] == ',' ){
                    if (tab) {
                        printf("\t");
                        } else {
                        printf("\n");
                    }
                } 
                else{
                    printf("\n");
                    printf("%c", (received[i]->data_ptr)[j]);
                }
            }
        }
    }
    else{
        print_response(received[0]);
    }
}

void wait_for_response(int channel){
    // WIP: This one should create a list of the packets received by the server after request.
    // Something goes wrong. It seems to get into a while true and never gets out of it. 
    while (1){

        // CHECK IF THE FIRST PACKET IS RECEIVED
        if(recv_pkt(pktbuff, channel) == SUCCESS){ // Waits until it receives a packet.

            // DECLARE this variable only if needed. (Inside the "if packet is received")
            char pktbuff[2048];
            Packet* pkt = empty_packet();

            // CONVERT the string packet received to a struct packet
            int error_code_conversion = string_to_packet(pktbuff, pkt); // TO DO HANDLE THE ERROR DUE TO THE CONVERSION IF THERE IS ONE
            strcpy(pktbuff, ""); // ???

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
            Packet ** PacketList = malloc(packnum * sizeof(Packet));
            PacketList[0] = pkt;
            int i = 1;
            while(i < packnum){ // Waits until it receives all desired packets. This might be where things go wrong.
                if(recv_pkt(pktbuff, channel) == SUCCESS){

                    // NOT sure about filling the packetList directly like that "string_to_packet(pktbuff, PacketList[i]);"
                    // maybe try to malloc an empty packet and then fill it with the content of the string, and finally put it inside the list.
                    string_to_packet(pktbuff, PacketList[i]); 
                    strcpy(pktbuff, ""); // ???
                    i++;
                }
            }

            received_from_server(PacketList, CLIENT_DIRECTORY); // Calls the function that reads the list and executes the desired action (print lines, get file, etc).
            break;
            
        }
    }
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

    // Writing to a closed socket causes a SIGPIPE signal, which makes the program exits. 
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
    char sendbuf[2048];

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
            strncpy(directory, argv[++i], sizeof(directory) - 1);
        }
        else {
            fprintf(stderr, "Error: Unknown option %s\n", argv[i]);  // User isn't making any sense
            return SYNTAX_ERROR;
        }
    }

    // Step 2: Handle -analyze option
    if (analyze_flag) {

        FILE *file = fopen(analyze_file, "r");


        char line[256];  // maximum length of the line
        // Print info to terminal

        // Infinite loop -> use ^C to exit the program
        while (fgets(line, 256, file)) { // FINITE TIMES

            // GENERATE Packet Command Line using a string format
            char cmd_to_packet_string[MAX_PACKET_SIZE];
            int error_code = convert_cmd_string_to_packet_string(line, cmd_to_packet_string);

            // TO DO MANAGE ERROR CODE

            // SEND the packet
            int res = send_pkt(cmd_to_packet_string, channel);

            wait_for_response(channel);
        }

    }

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

            // SEND the packet
            int res = send_pkt(cmd_to_packet_string, channel);

            // WAIT for a response from the server! (the function doesn't work yet)
            wait_for_response(channel);
        }
    }

    return SUCCESS; // USELESS
}
