/* to awoid including this header twice */
#ifndef COMMUNICATION_H
#define COMMUNICATION_H

// --------------------------------------------------
//  EXAMPLE.  to remove or replace by your project code        |
// --------------------------------------------------
/* Library for sending and receiving packets.
 * Packet format (different from project specification):
 *    ----------------------------------------
 *   | nb | command | parameter1 | parameter2 |
 *    ----------------------------------------
 * where 
 * - nb (1 byte) is 0, 1 or 2, the number of parameters,
 * - command (16 bytes) is a (null-terminated) string
 * - parameter1 (32 bytes) is a (null-terminated) string
 * - parameter2 (32 bytes) is a (null-terminated) string
 * irrelevant parameters (w.r.t nb) are not sent
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

#endif
