#include "../include/utilities.h"
#include "../uinclude/communication.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>

// Define constants for packet
#define HEADER_SIZE 70
#define MAX_PACKET_SIZE 2048
#define MAX_DATA_SIZE (MAX_PACKET_SIZE - HEADER_SIZE)

// Packet structure
typedef struct {
    char E;                      // 'E'
    char D;                      // 'D'
    char r;                      // 'r'
    unsigned short data_size;    // Size of data (2 bytes)
    unsigned char command;       // Command/Error (1 byte)
    char option1[32];            // Option 1 (32 bytes)
    char option2[32];            // Option 2 (32 bytes)
    char data[MAX_DATA_SIZE];    // Data field (up to 1978 bytes)
} Packet;

/*
CLIENT MODES:
-analyze [string]: the client executes the command of the file given as input.

-interactive: the client waits for commands on the keyboard.
If the "-analyze" option is also provided, then the client first executes of the commands of the referenced files
before waiting for keyboard-entered commands.

-directory [string]: specifies the directory to be used for client side files.
If this directory is non empty when the client starts,
then existing files are assumed to be part of the handled files,
as long as they respect the requirements on file names.
*/



int student_client(int channel, int argc, char *argv[]) {
    // Ignore SIGPIPE signals
    signal(SIGPIPE, SIG_IGN);

    // Variables for command-line options
    int analyze_flag = 0;
    int interactive_flag = 0;
    char analyze_file[256] = {0};
    char directory[256] = {0};

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
            if (interactive_flag || i + 1 >= argc) {
                fprintf(stderr, "Error: Duplicate -interactive option\n");
                return -1;
            }
            interactive_flag = 1;
        }
        else if (strcmp(argv[i], "-directory") == 0) {
            if (directory[0] || i + 1 >= argc) {
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
        // TODO: Open the file, read commands, and send packets to the server
    }

    // Step 3: Handle -interactive option
    if (interactive_flag) {
        printf("Entering interactive mode...\n");
        // TODO: Accept keyboard commands, construct packets, and send them
    }

    // Step 4: Handle -directory option
    if (directory[0]) {
        printf("Using directory: %s\n", directory);
        // TODO: Check existing files and process as needed
    }

    // Placeholder for communication logic
    Packet pkt = { 'E', 'D', 'r', 0, 0, "", "", "" };
    // TODO: Populate the packet structure based on user commands or file input
    // TODO: Send the packet using send_pkt() or other communication logic

    printf("Client setup complete.\n");
    return 0; // Success
}