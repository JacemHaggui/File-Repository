//--------------------------------------------------------------------------------
#include "../include/utilities.h"
#include "../uinclude/communication.h"
#include "../uinclude/struct_packet.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>

// Define constants for packet
#define HEADER_SIZE 70
#define MAX_PACKET_SIZE 2048
#define MAX_DATA_SIZE (MAX_PACKET_SIZE - HEADER_SIZE)
// Defining error codes
#define BAD_PACKET_FORMAT    -1
#define FILE_NOT_FOUND       -2
#define FILE_ALREADY_EXISTS  -3
#define COMMAND_FAILS        -4
#define QUOTA_EXCEEDED       -5
#define SYNTAX_ERROR         -6
#define BAD_SERVER_RESPONSE  -7
#define CONNECTION_CLOSED    -8
#define SUCCESS              0


/**  help message for commandline options */
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

//------------------------------------------------------------------------------
//int student_client(int channel, int argc, char *argv[])

int student_client(int argc, char *argv[]) { 
    /* 0
        INPUT :
            0
        OUTPUT :
            0
    */
    int channel = connect_to_server(argv[1], atoi(argv[2]) );

    // Ignore SIGPIPE signals
    signal(SIGPIPE, SIG_IGN);

    // Variables for command-line options
    int analyze_flag = 0;
    int interactive_flag = 0;
    char analyze_file[256] = {0};
    char directory[256] = {0}; // Directory MUST ENDS WITH '/' 

    //check if the user is asking for help
    if(argc == 2 && strcmp(argv[1], "help") == 0 ){
        printf("\n%s", client_help_options);
    };

    // Step 1: Parse command-line arguments, figuring out which mode to activate
    for (int i = 3; i < argc; i++) {

        if (strcmp(argv[i], "-analyze") == 0) {
            if (analyze_flag || i + 1 >= argc) {
                //If the program encounters -analyze, it expects a value (commands.txt) immediately after it.
                //If i + 1 >= argc,there are no more arguments after argv[i]
                fprintf(stderr, "Error: Invalid or duplicate -analyze option\n");//In case the user is messing with us
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
            fprintf(stderr, "Error: Unknown option %s\n", argv[i]);//User isn't making any sense
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
                continue; // Exit if memory allocation fails
            }

            // Check if the command can be converted to a packet
            if (CmdlinetoPacket(line, package) == -1) {
                printf("Error: Command not recognized. Please try again.\n");
                free(package); // Free the memory to avoid memory leak
                continue; // Skip to the next loop iteration
            }

            // Print the packet for debugging (optional)
            //print_packet(package);

            // Converting the packet into a string for sending
            char* package_string = malloc((70 + package->data_size)*sizeof(char));
            packet_to_string(package, package_string);

            // Sending the packet to the server
            send_pkt(package_string, channel);

            // Receiving the response from the server
            //Total packet size must not exceed 2048 bytes, including the header.
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
                    free(answer_package); // Free the memory to avoid memory leak
                    continue; // Skip to the next loop iteration
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
        printf("> "); // Command prompt simulator XD

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
        Packet* package = malloc(sizeof(Packet)); // Allocate memory for the packet
        if (package == NULL) {
            printf("Memory allocation failed. Exiting.\n");
            continue; // Exit if memory allocation fails
        }

        // Check if the command can be converted to a packet
        if (CmdlinetoPacket(command, package) == -1) {
            printf("Error: Command not recognized. Please try again.\n");
            free(package); // Free the memory to avoid memory leak
            continue; // Skip to the next loop iteration
        }

        // Print the packet for debugging (optional)
        //print_packet(package);


        // Converting the packet into a string for sending
        char package_string[2048];//maximum number of bytes is 2048
        packet_to_string(package, package_string);

        // Sending the packet to the server
        send_pkt(package_string, channel);

        // Receiving the response from the server
        //Total packet size must not exceed 2048 bytes, including the header.
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
                free(answer_package); // Free the memory to avoid memory leak
                continue; // Skip to the next loop iteration
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
