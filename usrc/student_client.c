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

int student_client(int channel, int argc, char *argv[]);

int main(int argc, char *argv[]){
    student_client(111, argc, argv);

    
    /*
    Packet* test_packet = malloc(sizeof(Packet));
    const char* test_command = "mv titi.txt toto.txt";
    int res = CmdlinetoPacket(test_command, test_packet);
    print_packet(test_packet);
    */
   return 0;
}


int student_client(int channel, int argc, char *argv[]) {
    // Ignore SIGPIPE signals
    signal(SIGPIPE, SIG_IGN);

    // Variables for command-line options
    int analyze_flag = 0;
    int interactive_flag = 0;
    char analyze_file[256] = {0};
    char directory[256] = {0};

    // Directory by default is './' AND MUST ENDS WITH '/' 
    directory[0] = '.';
    directory[1] = '/';

    //check if the user is asking for help
    if(argc == 2 && strcmp(argv[1], "help") == 0 ){
        printf("\n%s", client_help_options);
    };

    // Step 1: Parse command-line arguments, figuring out which mode to activate
    for (int i = 1; i < argc; i++) {

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
            if (directory[2] || i + 1 >= argc) { // BY DEFAULT directory is './'
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
        FILE *file = fopen(strcat(directory, analyze_file), "r"); // DIRECTORY ENDS WITH '/'

        char line[256];  // maximum length of the line
        //char *packet = NULL;   <-- USELESS NOW

        // Take each line of the file
        while (fgets(line, sizeof(line), file)) {
            // Allocate memory for the packet (based on line length)
            Packet * packet = empty_packet();
            // Convert the line to a packet
            error_code = string_to_packet(line, packet);
            // Send packet to the server
            send_packet(packet);
            // Free the allocated memory after use
            free(packet);
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
        if (strcmp(command, "exit") == 0) {
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
        print_packet(package);


        //Converting the packet to a string
        char package_string[2048];//maximum number of bytes is 2048
        packet_to_string(package, package_string);
        // Print the string we will send for debugging (optional)
        printf("%s\n", package_string);

        //Sending the packet
        //send_pkt(package_string, 863548516);

        // Free the allocated memory after processing the command
        free(package);
        }
    }

    // Step 4: Handle -directory option
    if (directory[0]) {
        printf("Using directory: %s\n", directory);
        // TODO: Check existing files and process as needed
    }

    // TODO: Populate the packet structure based on user commands or file input
    // TODO: Send the packet using send_pkt()
    return 0; // Success
}
