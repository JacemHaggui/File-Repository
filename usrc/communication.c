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

/* to print messages associated to detected errors */
#include <errno.h>
/* for fprintf, stderr,...*/
#include <stdio.h>
/* for read/write,...*/
#include <unistd.h>
/* for memcpy,...*/
#include <string.h>

#include <inttypes.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> // For close()
#include "../uinclude/struct_packet.h"
#include "../uinclude/functions.h"

// Defining error codes
#define BAD_PACKET_FORMAT    -1
#define FILE_NOT_FOUND       -2
#define FILE_ALREADY_EXISTS  -3
#define COMMAND_FAILS        -4
#define QUOTA_EXCEEDED       -5
#define SYNTAX_ERROR         -6
#define BAD_SERVER_RESPONSE  -7
#define CONNECTION_CLOSED    -8
#define CANNOT_READ          -9

#define SUCCESS              0

/* a function to receive a packet (in pkt) on a socket channel 
 * it assumes that the received packet respects the format.
 * (in a real program, this should be checked... )
 */
int recv_pkt(char *pkt, int channel) {
    /*
        Stores in the string pkt the data received from channel
	INPUT :
		pkt :       The string filled with the data received
		channel :   The channel on which we are listening
	OUTPUT :
        CONNECTION_CLOSED : The connection has been closed
        CANNOT_READ :       The data can't be read
        SUCCESS :           The data have been received and transmitted to pkt
	*/

    int amount_received;
    int total_size = 70; // First, we expect the 70-byte header
    char *buf = pkt; // pointer to data buffer

    // Read the header first
    amount_received = read(channel, buf, total_size);
    if (amount_received == -1) { // Error case
        perror("Cannot read");
        return CANNOT_READ;
    }
    if (amount_received == 0) { // Connection closed
        fprintf(stderr, "Connection closed\n");
        return CONNECTION_CLOSED;
    }

    printf("\n--[ Packet Received ]-- \n");
        printf("\tString Header Format :\n\t\t"); print_string(buf,amount_received);
        printf("\tData Header Received (sould be 70): %d\n", amount_received);
    
    // After the header, read the actual data (based on data_size field)
    //uint16_t data_size = *(uint16_t*)(pkt + 3); // WORK ~
    //uint16_t data_size = *(uint16_t*)( buf + 3 ); // WORK ~
    //uint16_t data_size = *(uint16_t *)(  (unsigned char)(  (unsigned char)(*(buf + 3)) ) | (unsigned char)(  (unsigned char)( *(buf + 4) ) << 8 ) ); // NOT WORKING
    uint16_t data_size = ((uint8_t)buf[4] << 8) | (uint8_t)buf[3];
        printf("\tData Received (should be below 1978): %u DATA\n", data_size);

    if (data_size > 0) { //If we actually have data (duuuuh)
        total_size = data_size; // Set total size to read the actual data
        buf += 70; // Move the buffer pointer past the header

        // Read the data field
        while (total_size > 0) {//While there is stuff to read keep reading
            amount_received = read(channel, buf, total_size);
            if (amount_received == -1) { // Error case
                perror("Cannot read");
                return CANNOT_READ;
            }
            if (amount_received == 0) { // Connection closed
                fprintf(stderr, "Connection closed\n");
                return CONNECTION_CLOSED;
            }

            total_size -= amount_received; // Update the remaining data size to read
            buf += amount_received; // Move the buffer pointer forward
        }
    }
    printf("\tData Content : \n\t\t");
    print_string((pkt + 70), data_size);

    printf("\n\tPacket Complete String Format :\n\t\t");
    print_string(pkt, 70 + data_size);

    printf("--[ END - Packet Received ]-- \n\n");
    

    return SUCCESS; // Success
}



/* A function to send a packet pkt in a socket channel. 
 * The "packet" parameter must respect the specified format (see before).
 * Returns 1 for success and 0 for failure
 */
int send_pkt(char *pkt, int channel) {
    // Header is 70 bytes: 3 for 'E', 'D', 'r' + 2 for data_size + 1 for command/error
    // + 32 for option1 + 32 for option2 = 70.
    uint16_t data_size = *(uint16_t*)(pkt + 3);
    int total_size = 70 + data_size; // 70 bytes header + data_size
    int amount_sent;
    char *buf = pkt; // pointer to data to send

    // Send the entire packet, including the header and data
    while (total_size > 0) {//while there is stuff to send, keep sending
        amount_sent = write(channel, buf, total_size);
        printf("Amount Send : %d\n", amount_sent);
        if (amount_sent == -1) { // Error case
            if (errno == EPIPE) {
                fprintf(stderr, "Connection closed\n");
                return  CONNECTION_CLOSED; // TO DO verify this line
            }
            else {
                perror("Cannot write");
                return COMMAND_FAILS; // TO DO verify this line
            }
        }
        if (amount_sent == 0) { // Connection issue
            fprintf(stderr, "Write problem\n");
            return COMMAND_FAILS; // TO DO verify this line

        }

        total_size -= amount_sent; // Update remaining size to send
        buf += amount_sent; // Move buffer pointer forward
    }
    printf("Succesfully sent packet\n");
    return SUCCESS;
}


void treat_response_code(int code){
    switch (code) {
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
}