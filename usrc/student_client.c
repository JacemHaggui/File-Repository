// ------------------------------------------------------
//  PROJECT IMPLEMENTATION: Client Packet Format         |
// ------------------------------------------------------
/* This implementation respects the project-specific packet format.
 * The client analyzes the command line arguments using the 
 * parse_commandline() function. Then, it sends these arguments 
 * to the server, which processes the command.
 * 
 * Packet Format (Header + Data) as defined for the project:
 *    ---------------------------------------------------------------------
 *   | 'E' | 'D' | 'r' | data size (2 bytes) | command/error (1 byte)     |
 *   | option1 (32 bytes) | option2 (32 bytes) | data (0 to 1978 bytes)    |
 *    ---------------------------------------------------------------------
 * 
 * Where:
 * - The first three bytes are fixed: 'E', 'D', and 'r'.
 * - Data size (2 bytes): Represents the size of the data field in the packet.
 * - Command/Error (1 byte): Stores the command type or error code.
 * - Option1 (32 bytes): Stores a string (e.g., filename or other metadata).
 * - Option2 (32 bytes): Stores a second string (or additional metadata).
 * - Data (0 to 1978 bytes): Contains raw data if the command requires it 
 *   (e.g., file contents). This field is optional.
 * 
 * Total packet size must not exceed 2048 bytes, including the header.
 */

// INCLUDES
/* to use the provided parse_commandline function. */
#include "../include/utilities.h"
/* user defined library for sending and receiving packets */
#include "../uinclude/communication.h"
/* for stdin,...*/
#include <stdio.h>
/* for memcpy,...*/
#include <string.h>
/* to modify SIGPIPE signal handling, as default behaviour makes
 * the program exits when trying to write to a closed socket. 
 * We don't want this.
 */
#include <signal.h>

// FUNCTIONS
// ...
// empty because all used functions in this example are provided by
// "provided", "communication" and system provided libraries.
// -----------------------------------------------------
//  END of EXAMPLE code to be replace                   |
// -----------------------------------------------------


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

// The following function is the one you must implement for your project.
// Here, we provide an implementation related to the example given above, which is
// not the exactly the project you have to implement
// It returns 0 to exit or another value to restart 
// the client.

int student_client(int channel, int argc, char *argv[]) {
    // Writing to a closed socket causes a SIGPIPE signal, which makes 
    // the program exits. The following line inhibits this default behaviour.
    // Thus, writing to a cloned socket will simply return -1 with the EPIPE
    // error in errno and will therefore not cause an exit. 
    // (see the line with "EPIPE" in send_pkt in usrc/communication.c).
    signal(SIGPIPE, SIG_IGN);

    // --------------------------------------------------
    //  EXAMPLE. To be replaced by your project code     |
    // --------------------------------------------------

    // illustrate sort_dir
    char arr[] = "fileC,100,fileA,50,fileB,75";
    printf("\nIllustrating function sort_dir\n string before: %s\n",arr);
    if(!sort_dir(arr))
        fprintf(stderr," Error while sorting\n\n");
    else
        printf(" string after: %s\n\n", arr);

    // Example of a client...
    printf("Example of a client\nS");
    // Buffer to receive the command line
    char cmdline[128];
    // Buffer to build the packet to send (max size: 81)
    char sendbuf[81];
    // Structure to be filled by parse_commandline
    usercommand parsed_cmd;
    // print info to terminal
    printf("(^C to exit)\n\n");
    // infinite loop -> use ^C to exit the program 
    while (1) {
        // get the command from user, exit if it fails
        printf("Enter a command > ");
        if(! fgets(cmdline, 128, stdin)){
            printf("Cannot read command line\n");
            // return 0 to exit
            return 0;
        }
        // parse it
        int test = parse_commandline(&parsed_cmd, cmdline);
        if (test) { // parsing was successful
            // prepare packet to be sent
            // 1. fill all fields except the number of parameters. 
            //    To simplify the example, unused parameters are 
            //    also copied, but they won't be sent
            memcpy(sendbuf+1, parsed_cmd.cmd, 16); // copy command
            memcpy(sendbuf+17, parsed_cmd.param1, 32); // 1st parameter
            memcpy(sendbuf+49, parsed_cmd.param2, 32); // snd parameter
            // 2. set the number or parameter (command dependent)
            if (   strcmp(parsed_cmd.cmd,"help") == 0
                || strcmp(parsed_cmd.cmd,"quit") == 0
                || strcmp(parsed_cmd.cmd,"restart") == 0
                || strcmp(parsed_cmd.cmd,"ls") == 0 )
                *sendbuf = 0; //command without parameter
            else if ( strcmp(parsed_cmd.cmd,"put") == 0
                || strcmp(parsed_cmd.cmd,"rm") == 0 )
                *sendbuf = 1; //command with 1 parameter
            else 
                *sendbuf = 2; //command with 1 parameter
            // 3. attempt to send the packet
            int res = send_pkt(sendbuf, channel);
            // returns 1 to restart if somme communication error occured
            if (!res) return 1; 
        }
    }
    // --------------------------------------------------
    //  END of EXAMPLE code to be replaced               |
    // --------------------------------------------------
}



// Printing n lines
Packet * print_n_lines(char * filename, int n) {
    Packet * p = empty_packet();
    p->E = 'E';
    p->D = 'D';
    p->r = 'r';
    p->code = 1;
    p->option1 = filename;
    p->option2 = atoi(n);
    return p;
}


// Adding a remote file
Packet * adding_remote_file(char * local_filename, int size_file) {
    Packet * p = empty_packet();
    p->E = 'E';
    p->D = 'D';
    p->r = 'r';
    p->code = 2;
    p->option1 = local_filename;
    p->option2 = atoi(size_file);
    return p;
}


// Renaming a remote file
Packet * renaming_remote_file(char * original_name, char * new_name) {
    Packet * p = empty_packet();
    p->E = 'E';
    p->D = 'D';
    p->r = 'r';
    p->code = 3;
    p->option1 = original_name;
    p->option2 = new_name;
    return p;
}


// Removing a remote file
Packet * removing_remote_file(char * filename, char * empty) {
    Packet * p = empty_packet();
    p->E = 'E';
    p->D = 'D';
    p->r = 'r';
    p->code = 4;
    p->option1 = filename;
    p->option2 = empty;
    return p;
}


// Getting a remote file
Packet * getting_remote_file(char * filename, char * empty) {
    Packet * p = empty_packet();
    p->E = 'E';
    p->D = 'D';
    p->r = 'r';
    p->code = 4;
    p->option1 = filename;
    p->option2 = empty;
    return p;
}


// Listing remote files
Packet * listing_remote_file(char * filename, char * empty) {
    Packet * p = empty_packet();
    p->E = 'E';
    p->D = 'D';
    p->r = 'r';
    p->code = 5;
    p->option1 = filename;
    p->option2 = empty;
    return p;
}
