#ifndef COMMUNICATION_H
#define COMMUNICATION_H

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

/* a function to send a packet pkt on a socket channel 
 * the parameter packet must respect the format.
 * Returns 1 for success and 0 for failure
 */
int send_pkt(char *pkt, int channel);

/* a function to receive a packet (in pkt) on a socket channel 
 * it supposes that the received packet respects the format.
 * ( in a real program, this should be checked... )
 * Returns 1 for success and 0 for failure
 */
int recv_pkt(char *pkt, int channel);

void treat_response_code(int code);
#endif
