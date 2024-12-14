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

#define INT_MAX 2048 - 70 // Maximum data_size for packet data.

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
        // COPY STUDENT CLIENT CODE
        recv_pkt(pktbuff, channel);

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

int student_client_old(int argc, char *argv[]) {
    /*
        Implements a client-side program that interacts with a server.
    INPUT :
        argc : The number of command-line arguments
        *argv[] : An array of strings containing the command-line arguments
    OUTPUT :
        0 : Sucess
        -1 : Bad packet format
    */
    int channel = connect_to_server(argv[1], atoi(argv[2]));

    // Ignore SIGPIPE signals
    signal(SIGPIPE, SIG_IGN);

    // Variables for command-line options
    int analyze_flag = 0;
    int interactive_flag = 0;
    char analyze_file[256] = {0};
    char directory[256] = {0};  // Directory MUST ENDS WITH '/'

    // Check if the user is asking for help
    if(argc == 2 && strcmp(argv[1], "help") == 0 ){
        printf("\n%s", client_help_options);
    };

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

            switch (response_code) {
                case BAD_PACKET_FORMAT:
                    printf("\nBad packet format\n");
                    break;
                case FILE_NOT_FOUND:
                    printf("\nFile not found\n");
                    break;
                case FILE_ALREADY_EXISTS:
                    printf("\nFile already exists\n");
                    break;
                case COMMAND_FAILS:
                    printf("\nCommand fails (for other server-side failures)\n");
                    break;
                case QUOTA_EXCEEDED:
                    printf("\nQuota exceeded\n");
                    break;
                case SYNTAX_ERROR:
                    printf("\nSyntax error in command line\n");
                    break;
                case BAD_SERVER_RESPONSE:
                    printf("\nBad response from server\n");
                    break;
                case CONNECTION_CLOSED:
                    printf("\nConnection closed\n");
                    break;
                case SUCCESS:
                    printf("\nSuccessfully received the server's response\n");
                    break;
                default:
                    printf("\nUNKNOWN ERROR\n");
            }
    
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

        switch (response_code) {
            case BAD_PACKET_FORMAT:
                printf("\nBad packet format\n");
                break;
            case FILE_NOT_FOUND:
                printf("\nFile not found\n");
                break;
            case FILE_ALREADY_EXISTS:
                printf("\nFile already exists\n");
                break;
            case COMMAND_FAILS:
                printf("\nCommand fails (for other server-side failures)\n");
                break;
            case QUOTA_EXCEEDED:
                printf("\nQuota exceeded\n");
                break;
            case SYNTAX_ERROR:
                printf("\nSyntax error in command line\n");
                break;
            case BAD_SERVER_RESPONSE:
                printf("\nBad response from server\n");
                break;
            case CONNECTION_CLOSED:
                printf("\nConnection closed\n");
                break;
            case SUCCESS:
                printf("\nSuccessfully received the server's response\n");
                break;
            default:
                printf("\nUNKNOWN ERROR\n");
            }
    
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



int student_client(int channel, int argc, char *argv[]) {
    /*
        Implements a client-side program that interacts with a servernthrough a socket channel.
    INPUT :
        channel : A socket file descriptor used to communicate with the server
        argc : The number of command-line arguments
        *argv[] : An array of strings containing the command-line arguments
    OUTPUT :
        CANNOT_READ : to exit
        
    */

    // Writing to a closed socket causes a SIGPIPE signal, which makes the program exits. 
    // The following line inhibits this default behaviour.
    // Thus, writing to a cloned socket will simply return -1 with the EPIPE
    // error in errno and will therefore not cause an exit. 
    // (see the line with "EPIPE" in send_pkt in usrc/communication.c).
    signal(SIGPIPE, SIG_IGN);

    // Buffer to receive the command line
    char cmdline[128];
    // Buffer to build the packet to send (max size: 81)
    char sendbuf[2048];

    // Print info to terminal
    printf("(^C to exit)\n\n");
    // Infinite loop -> use ^C to exit the program 
    while (1) {
        // Get the command from user, exit if it fails
        printf("Enter a command > ");
        if(! fgets(cmdline, 128, stdin)){
            printf("Cannot read command line\n");
            // Return CANNOT_READ to exit
            return CANNOT_READ;
        }

        printf("PRINT CLIENT COMMAND :\n\t");
        print_string(cmdline, strlen(cmdline) - 1);

        char cmd_to_packet_string[MAX_PACKET_SIZE];
        int error_code = convert_cmd_string_to_packet_string(cmdline, cmd_to_packet_string);

        printf("\nString Send :\n\t");
        print_string(cmd_to_packet_string, 70);  // HEADER ONLY

        // Attempt to send the packet
        int res = send_pkt(cmd_to_packet_string, channel);
        printf("RES : %d\n", res);
        // Returns 1 to restart if somme communication error occured
        // If (!res) return 1;
    }
}
