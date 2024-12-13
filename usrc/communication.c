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

/* a function to receive a packet (in pkt) on a socket channel 
 * it assumes that the received packet respects the format.
 * (in a real program, this should be checked... )
 * Returns 1 in case of success, 0 in case of failure
 */
int recv_pkt(char *pkt, int channel) {
    int amount_received;
    int total_size = 70; // First, we expect the 70-byte header
    char *buf = pkt; // pointer to data buffer

    // Read the header first
    amount_received = read(channel, buf, total_size);
    if (amount_received == -1) { // Error case
        perror("Cannot read");
        return 0;
    }
    if (amount_received == 0) { // Connection closed
        fprintf(stderr, "Connection closed\n");
        return 0;
    }

    // After the header, read the actual data (based on data_size field)
    uint16_t data_size = *(uint16_t*)(pkt + 3); // Get data size(2 bytes) from the packet
    if (data_size > 0) { //If we actually have data (duuuuh)
        total_size = data_size; // Set total size to read the actual data
        buf += 70; // Move the buffer pointer past the header

        // Read the data field
        while (total_size > 0) {//While there is stuff to read keep reading
            amount_received = read(channel, buf, total_size);
            if (amount_received == -1) { // Error case
                perror("Cannot read");
                return 0;
            }
            if (amount_received == 0) { // Connection closed
                fprintf(stderr, "Connection closed\n");
                return 0;
            }

            total_size -= amount_received; // Update the remaining data size to read
            buf += amount_received; // Move the buffer pointer forward
        }
    }

    return 0; // Success
}



/* A function to send a packet pkt in a socket channel. 
 * The "packet" parameter must respect the specified format (see before).
 * Returns 1 for success and 0 for failure
 */
void send_pkt(char *pkt, int channel) {
    // Header is 70 bytes: 3 for 'E', 'D', 'r' + 2 for data_size + 1 for command/error
    // + 32 for option1 + 32 for option2 = 70.
    uint16_t data_size = *(uint16_t*)(pkt + 3);
    int total_size = 70 + data_size; // 70 bytes header + data_size
    int amount_sent;
    char *buf = pkt; // pointer to data to send

    // Send the entire packet, including the header and data
    while (total_size > 0) {//while there is stuff to send, keep sending
        amount_sent = write(channel, buf, total_size);
        if (amount_sent == -1) { // Error case
            if (errno == EPIPE)
                fprintf(stderr, "Connection closed\n");
            else
                perror("Cannot write");
            return 0;
        }
        if (amount_sent == 0) { // Connection issue
            fprintf(stderr, "Write problem\n");
            return 0;
        }

        total_size -= amount_sent; // Update remaining size to send
        buf += amount_sent; // Move buffer pointer forward
    }
    printf("Succesfully sent packet\n");
}
