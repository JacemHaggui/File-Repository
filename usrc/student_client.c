#include "../include/utilities.h"
#include "../uinclude/communication.h"
//#include "../uinclude/struct_packet.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>

// Define constants for packet
#define HEADER_SIZE 70
#define MAX_PACKET_SIZE 2048
#define MAX_DATA_SIZE (MAX_PACKET_SIZE - HEADER_SIZE)


// Packet structure
typedef struct  {
	char E; // 1 byte
	char D; // 1 byte
	char r; // 1 byte
	uint16_t data_size; // 2 byte  <!> little-endian MODE ! (So we have to swap from little-endian to big-endian order)
	int8_t code; // 1 byte
	char option1[32]; //32 bytes
	char option2[32]; //32 bytes
	char * data_ptr;
} Packet ;

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

int CmdlinetoPacket(const char *input, Packet *pkt);
void print_packet(Packet * packet);
int student_client(int channel, int argc, char *argv[]);
int packet_to_string(Packet * packet, char * string);

int main(int argc, char *argv[]){
    student_client(111, argc, argv);

    return 0;
    /*
    Packet* test_packet = malloc(sizeof(Packet));
    const char* test_command = "mv titi.txt toto.txt";
    int res = CmdlinetoPacket(test_command, test_packet);
    print_packet(test_packet);
    */
    return 1;
}

int CmdlinetoPacket(const char *input, Packet *pkt) {
    // Initialize the packet with fixed values
    pkt->E = 'E';
    pkt->D = 'D';
    pkt->r = 'r';
    
    // Initializing data_size and the option1 and option2 arrays to zero.
    // We use memset here to efficiently set all 32 bytes of each array to 0.
    // This ensures that the options are empty before we populate them with actual data.
    memset(pkt->option1, 0, sizeof(pkt->option1));
    memset(pkt->option2, 0, sizeof(pkt->option2));
    pkt->data_size = 0;

    // Split input into command and arguments
    // Buffers to store command and arguments
    char command[64];  // Size 64 for most commands
    char option1[256]; // Size 256 for first argument (e.g., filename)
    char option2[256]; // Size 256 for second argument (if needed)

    // Parse the input into command and options
    int args = sscanf(input, "%s %s %s", command, option1, option2); 
    /* sscanf Explanation
    * The sscanf function is used to read formatted input from the string `input`.
    * 
    * - "%s %s %s" is the format string that tells sscanf to:
    *   - Read a string (word) up to the first space and store it in `command`
    *   - Read the next string up to the next space and store it in `option1`
    *   - Read the next string up to the next space and store it in `option2`
    * 
    * In the case of an input like "put myfile.txt /path/to/destination":
    * - `command` will store "put"
    * - `option1` will store "myfile.txt"
    * - `option2` will store "/path/to/destination"
    * 
    * `args` will store the number of successful assignments, which is 3 in this case 
    * because three strings were successfully extracted from the `input`.
    */

    strncpy(pkt->option1, option1, sizeof(pkt->option1) - 1);  // Copy the first option (e.g., filename)
    strncpy(pkt->option2, option2, sizeof(pkt->option2) - 1);  // Copy the second option (if available)

    // Determine which command was entered
    if (strcmp(command, "put") == 0) {
        pkt->code = 1;  // Command code for "put" (not permanent)
        //TODO: Put the entire file contents in data
        //Seems weird since the packet only takes a pointer to data ?
        //Reminder: Discuss with the group 
    } 
    else if (strcmp(command, "rm") == 0) {
        pkt->code = 2;  // Command code for "rm"
    } 
    else if (strcmp(command, "get") == 0) {
        pkt->code = 3;  // Command code for "get"
        // pkt->option1 and pkt->option2 have already been set earlier
    } 
    else if (strcmp(command, "ls") == 0) {
        pkt->code = 4;  // Command code for "ls"
    } 
    else if (strcmp(command, "cat") == 0) {
        pkt->code = 5;  // Command code for "cat"
    } 
    else if (strcmp(command, "mv") == 0) {
        pkt->code = 6;  // Command code for "mv"
        // pkt->option1 and pkt->option2 have already been set earlier
    } 
    else if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) {
        pkt->code = 7;  // Command byte for "quit" or "exit"
    } 
    else if (strcmp(command, "restart") == 0) {
        pkt->code = 8;  // Command byte for "restart"
    }

    else {
        fprintf(stderr, "Error: Unknown command '%s'\n", command);
        return -1;
    }

    return 1;  // Indicate success
}


void print_packet(Packet * packet) {
	printf("Print Packet :\n");
	if (! packet) 	{printf("\t-Empty Packet\n");  return ;} 
	if ( packet->E) printf("\t-Const E : %c\n", packet->E); else printf("\t-No Const E\n");
	if ( packet->D) printf("\t-Const D : %c\n", packet->D); else printf("\t-No Const D\n");
	if ( packet->r) printf("\t-Const r : %c\n", packet->r); else printf("\t-No Const r\n");
	if ( packet->data_size) printf("\t-Data Size : %d bytes\n", packet->data_size); else printf("\t-No Data Size\n");
	if ( packet->code) printf("\t-Code : %d\n", packet->code); else printf("\t-No Code\n");
	if ( packet->option1) printf("\t-Option 1 : %s\n", packet->option1); else printf("\t-No Option 1\n");
	if ( packet->option2) printf("\t-Option 2 : %s\n", packet->option2); else printf("\t-No Option 2\n");
	if (packet->data_ptr) printf("\t-Data Pointer provided ? : %d (1 <=> True)", *(packet->data_ptr) != '\0'  ); else printf("\t-No Data Pointer Provided");
}


int student_client(int channel, int argc, char *argv[]) {
    // Ignore SIGPIPE signals
    signal(SIGPIPE, SIG_IGN);

    // Variables for command-line options
    int analyze_flag = 0;
    int interactive_flag = 0;
    char analyze_file[256] = {0};
    char directory[256] = {0};

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
        
    // Continuous loop for interactive mode
    while (1) {
        char command[256];
        printf("> "); // Command prompt simulator XD

        // Read a command from the command line
        if (fgets(command, sizeof(command), stdin) == NULL) {
            printf("Error reading input. Exiting interactive mode.\n");
            continue;
        }
        command[strcspn(command, "\n")] = 0;  // Remove the newline character

        // Check for an exit command
        if (strcmp(command, "exit") == 0) {
            printf("Exiting interactive mode.\n");
            break;
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
        printf("%s", package_string);

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




int packet_to_string(Packet * packet, char * string) {
	/*
	Convert a packet given in a string format.
	INPUT :
		packet : The packet to transform in a string
		string : The string which will be filled in place
	OUTPUT :
		0 : R.A.S (All goooooood)
	*/
	*(string) 	= packet->E;
	*(++string) 	= packet->D;
	*(++string) 	= packet->r;
	*(++string)	= ((packet->data_size & 0xF0) >> 8) ; // First get the 8 upper bit with the mask then shift right 8 times to remove the lower part
	*(++string)	= (packet->data_size & 0x0F) ; // Then get the 8 lower bits with an another mask 
	*(++string)	= packet->code;

	int n_opt1 = strlen(packet->option1);
	for (int i = 0; i < n_opt1 ; i ++ ) {
		*(++string) = packet->option1[i];
	}
	*(++string)	= '\0'; // ADD NUL-TERMINATOR

	int n_opt2 = strlen(packet->option2);
	for (int i = 0; i < n_opt2 ; i ++) {
		*(++string)	= packet->option2[i];
	}
	*(++string)	= '\0'; // ADD NUL-TERMINATOR
	
	char * ptr = packet->data_ptr;
	for (int i = 0; i < packet->data_size; i ++) {
		*(++string)	= *(ptr++);
	}

	return 0;
}