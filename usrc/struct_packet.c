#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <ctype.h>
#include "../uinclude/struct_packet.h"
#include "../uinclude/functions.h"
#include <stddef.h>  // DEBUG ONLY


Packet * empty_packet() {
	/* 
		Create an empty Packet. 
		Each packet begins with E, D and r.
	INPUT :
	OUTPUT :
		packet : Empty Packet
	*/
	Packet * packet = calloc(1, sizeof(*packet));
	packet->E = 'E'; packet->D = 'D'; packet->r = 'r';
	//Packet * packet = (Packet *)malloc(sizeof(Packet));
	return packet;
}


Packet * error_packet(int errcode){
	/* 
		Create a Packet containing an error code.
	INPUT :
		errcode : Error code
	OUTPUT :
		out : Packet with error code
	*/
	Packet * out = empty_packet();
	out->data_size = 0; out->code = errcode;
	return out;
}


void free_packet(Packet * packet) {
	/* 
		Free Packet if exist 
	INPUT :
		packet : The pointer to a given packet which will be overwritted
	OUTPUT :
	*/
	if (packet) free(packet);
}


void print_packet(Packet * packet) {
	/* 
		ONLY FOR DEBUGGING : Print the Packet in a decent format
	INPUT :
		packet : The pointer to a given packet which will be overwritted
	OUTPUT :
	*/
	printf("Print Packet :\n");
	if (! packet) 	{printf("\t-Empty Packet\n");  return ;} 
	if ( packet->E) printf("\t-Const E : %c\n", packet->E); else printf("\t-No Const E\n");
	if ( packet->D) printf("\t-Const D : %c\n", packet->D); else printf("\t-No Const D\n");
	if ( packet->r) printf("\t-Const r : %c\n", packet->r); else printf("\t-No Const r\n");
	if ( packet->data_size) printf("\t-Data Size : %d bytes\n", packet->data_size); else printf("\t-No Data Size\n");
	if ( packet->code) printf("\t-Code : %d\n", packet->code); else printf("\t-No Code\n");
	if ( strlen( packet->option1 ) > 0) printf("\t-Option 1 : %s\n", packet->option1); else printf("\t-No Option 1\n");
	if ( strlen( packet->option2 ) > 0) printf("\t-Option 2 : %s\n", packet->option2); else printf("\t-No Option 2\n");
	if (packet->data_ptr) printf("\t-Data Pointer provided ? : %d (1 <=> True)\n", *(packet->data_ptr) != '\0'  ); else printf("\t-No Data Pointer Provided\n");
}


void print_string(char * str, int n) {
	/* 
		ONLY FOR DEBUGGING : Print the first n characters in the string.
	<!> 0x0000 and '\0' are the same character in C. They will be shown as '\0' 
	INPUT :
		str : The string to print
		n : Number of caracters we want to print
	OUTPUT :
	*/
	for (int i = 0; i < n ; i ++) {
		unsigned char c = (unsigned char) str[i];
		if (c == '\0') printf("\\0");
		else if(c == '\x0A') printf("\\n");
		else if(c == '\x0D') printf("\\r");
		else if (isprint(c)) printf("%c", c);
		else printf("\\x%02X", c);
	}
	putchar('\n');
}


int string_to_packet (char * string, Packet * packet) { 
	/* 
		Convert a string argument to a struct packet given 
	INPUT :
		string : The string to convert
		packet : The pointer to a given packet which will be overwritted
	OUTPUT :
		0 : Success - Conversion is complete
		-1 : Error Code - Bad packet format
		-5 : Error Code - Quota exceeded
	*/
		if ( ! *string) { return BAD_PACKET_FORMAT ; }  // String is Empty

	char * ptr = string ; // ptr is pointing to the first element in string

	// CONSTANTS PART
	if (*ptr) 	packet->E = *ptr;  	else return BAD_PACKET_FORMAT ;  // Constant Value E not here
	if (*(++ptr))	packet->D = *(ptr);	else return BAD_PACKET_FORMAT ;  // Constant Value D not here
	if (*(++ptr))	packet->r = *(ptr);	else return BAD_PACKET_FORMAT ;  // Constant Value r not here

	// DATA SIZE PART
	uint16_t data_size = (uint16_t)(  (unsigned char)(  (unsigned char)(*(++ptr)) ) | (unsigned char)(  (unsigned char)( *(++ptr) ) << 8 ) );
	packet->data_size = data_size;

	// CODE PART
	uint8_t code = *(uint8_t *)(++ptr);
	packet->code = code;


	// OPTION 1 PART
	int i = 0;
	++ptr; // Skip the code final character
	while(*(ptr) != '\0') {
		//printf("\t\t%d : %c\n", i, *ptr);
		if (i > 31) return QUOTA_EXCEEDED;  // Option 1 is too long
		packet->option1[i] = *ptr;
		i ++;
		++ptr;
	}
	++ptr; // Skip the option1 last character : '\0'
	i++;

	// OPTION 1 SKIP '.' noise after '\0' null terminator :
	while( i < 32) {
		++ptr;
		i ++ ;
	}

	// OPTION 2 PART
	int j = 0;
	
	while(*(ptr) != '\0' ) {
		//printf("\t\t%d : %c\n", i, *ptr);
		if (j > 31) return QUOTA_EXCEEDED ;  // Option 2 is too long
		packet->option2[j] = *ptr;
		j ++;
		++ptr;
	}
	++ptr; // Skip the option2 last character : '\0'
	j++;

	// OPTION 1 SKIP '.' noise after '\0' null terminator :
	while( j < 32) {
		++ptr;
		j ++ ;
	}

	// DATA PART
	packet->data_ptr = ptr;

	return SUCCESS;

} 


int packet_to_string(Packet * packet, char * string) {
	/* 
		Convert a packet given in a string format.
	INPUT :
		packet : The packet to transform in a string
		string : The string which will be filled in place
	OUTPUT :
		0 : Success
	*/
	*(string) 	= packet->E;
	*(++string) 	= packet->D;
	*(++string) 	= packet->r;
	//*(++string)	= ((packet->data_size & 0xF0) >> 8) ; // First get the 8 upper bit with the mask then shift right 8 times to remove the lower part
	//*(++string)	= (packet->data_size & 0x0F) ; // Then get the 8 lower bits with an another mask 
	*(++string) 	= (uint8_t)(packet->data_size & 0xFF);
	*(++string)		= (uint8_t)((packet->data_size >> 8) & 0xFF);
	*(++string)	= packet->code;

	int n_opt1 = strlen(packet->option1);
	for (int i = 0; i < n_opt1 ; i ++ ) {
		*(++string) = packet->option1[i];
	}
	*(++string)	= '\0'; // ADD NUL-TERMINATOR

	// fill option1 with void :
	for (int i = 0; i < 32 - (n_opt1 + 1)  ; i ++ ) {
		*(++string) = '.';
	}

	int n_opt2 = strlen(packet->option2);
	for (int i = 0; i < n_opt2 ; i ++) {
		*(++string)	= packet->option2[i];
	}
	*(++string)	= '\0'; // ADD NUL-TERMINATOR

	// fill option2 with void :
	for (int i = 0; i < 32 - (n_opt2 + 1)  ; i ++ ) {
		*(++string) = '.';
	}
	
	char * ptr = packet->data_ptr;
	for (int i = 0; i < packet->data_size; i ++) {
		*(++string)	= *(ptr++);
	}

	return 0;
}


int CmdlinetoPacket(const char *input, Packet *pkt) {
	/* 
		Convert a string given in a packet format.
	INPUT :
		input : The string to transform in a packet
		pkt : The packet which will be filled in place
	OUTPUT :
		0 : Success
		-6 : Syntax error in command line
	*/

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
    
    if (args < 3) {
		printf("CmdLineToPacket : args < 3 and args = %d \n", args);
    }

    strncpy(pkt->option1, option1, sizeof(pkt->option1) - 1);  // Copy the first option (e.g., filename)
    strncpy(pkt->option2, option2, sizeof(pkt->option2) - 1);  // Copy the second option (if available)

    // Determine which command was entered
	if (strcmp(command, "cat") == 0) {
        pkt->code = 1;  // Command code for "cat"
    }
    else if (strcmp(command, "put") == 0) {
        pkt->code = 2;  // Command code for "put" (not permanent)
    }
	else if (strcmp(command, "mv") == 0) {
        pkt->code = 3;  // Command code for "mv"
        // pkt->option1 and pkt->option2 have already been set earlier
    }
    else if (strcmp(command, "rm") == 0) {
        pkt->code = 4;  // Command code for "rm"
    }
    else if (strcmp(command, "get") == 0) {
        pkt->code = 5;  // Command code for "get"
        // pkt->option1 and pkt->option2 have already been set earlier
    }
    else if (strcmp(command, "ls") == 0) {
        pkt->code = 6;  // Command code for "ls"
    }
    else if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) {
        pkt->code = 7;  // Command byte for "quit" or "exit"
    }
    else if (strcmp(command, "restart") == 0) {
        pkt->code = 8;  // Command byte for "restart"
    }

    else {
        fprintf(stderr, "Error: Unknown command '%s'\n", command);
        return -6;
    }

    return 0;  // Success
}


// Utility function to swap byte order (little-endian to big-endian)
// 2 byte  <!> little-endian MODE ! (So we have to swap from little-endian to big-endian order)
uint16_t swap_endian_16(uint16_t value) {
    return (value >> 8) | (value << 8);
}

// Function to print response details based on the packet
void print_response(const Packet *packet) {
    if (!packet) {
        printf("Error: Null packet received.\n");
        return;
    }

    // Swap `data_size` from little-endian to big-endian
    uint16_t data_size_be = swap_endian_16(packet->data_size);

    // Decode the command based on the `code` field
    printf("Response Details:\n");
    switch (packet->code) {
        case CMD_LIST:
            printf("Listing remote files\n");
            break;
        case CMD_GET:
            printf("Getting a remote file\n");
            printf("File Name: %s\n", packet->option1);
            break;
        case CMD_REMOVE:
            printf("Removing a remote file\n");
            printf("File Name: %s\n", packet->option1);
            break;
        case CMD_RENAME:
            printf("Renaming a remote file\n");
            printf("Old Name: %s, New Name: %s\n", packet->option1, packet->option2);
            break;
        case CMD_ADD:
            printf("Adding a remote file\n");
            printf("File Name: %s\n", packet->option1);
            break;
        case CMD_PRINT:
            printf("Printing n lines of file\n");
            printf("File Name: %s\n", packet->option1);
            break;
        default:
            printf("Unknown command (Code: %d)\n", packet->code);
            break;
    }

    // Handle additional data if `data_ptr` is not NULL
    if (packet->data_ptr && data_size_be > 0) {
		print_string(packet->data_ptr, data_size_be);
    }
	else {
        printf("No additional data.\n");
    }
}

