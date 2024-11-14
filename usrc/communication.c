// ----------------------------------------------------------
//  EXAMPLE.  To be removed or replaced by your project code |
// ----------------------------------------------------------

/* implementation of communication.h, a library for sending 
 * and receiving packets. Packet format (different from project):
 *    ----------------------------------------
 *   | nb | command | parameter1 | parameter2 |
 *    ----------------------------------------
 * where 
 * - nb (1 byte) is 0, 1 or 2, the number of parameters,
 * - command (16 bytes) is a (null-terminated) string
 * - parameter1 (32 bytes) is a (null-terminated) string
 * - parameter2 (32 bytes) is a (null-terminated) string
 * irrelevant parameters (w.r.t. nb) are not sent
 */

/* to print messages associated to detected errors */
#include <errno.h>
/* for fprintf, stderr,...*/
#include <stdio.h>
/* for read/write,...*/
#include <unistd.h>
/* for memcpy,...*/
#include <string.h>

/* a function to receive a packet (in pkt) on a socket channel 
 * it assumes that the received packet respects the format.
 * (in a real program, this should be checked... )
 * Returns 1 in case of success, 0 in case of failure
 */
int recv_pkt(char *pkt, int channel) {
    int amount_to_receive;
    int amount_received;
    char *buf = pkt; // pointer to the place where received 
                     // data will be put, begins at pkt

    // decides of the amount of data to be received by looking at the "nb" field.
    // -> first reads this field
    amount_received = read(channel, buf, 1);
    if (amount_received == -1) { //error case
        // print error-associated message on stderr cf errno.h
        perror("cannot write"); 
        return 0;
    }
    if (amount_received == 0) { // connection closed
        // print relevant error message on stderr
        fprintf(stderr, "connection closed\n"); 
        return 0;
    }
    else { // set amount_to_receive
        if (*pkt == 0) amount_to_receive = 16;
        else if (*pkt == 1) amount_to_receive = 48;
        else amount_to_receive = 80;
    }
    buf++; // set pkt to the beginning of the command string
    // receive the remaining data, which may require several reads
    while(amount_to_receive > 0) { // data remains to send
        amount_received = read(channel, buf, amount_to_receive);
        if (amount_received == -1) { //error case
            // print error-associated message on stderr. cf errno.h
            perror("cannot write"); 
            return 0;
        }
        if (amount_received == 0) { // connection closed
            // print relevant error message on stderr
            fprintf(stderr, "connection closed\n"); 
            return 0;
        }
        else { // amount_received data has been read
            // update amount of data to receive
            amount_to_receive = amount_to_receive - amount_received;
            // points to relevant data location
            buf = buf + amount_received;
        }
    }
    return 1; // if this line is reached, no error occurred.
}



/* A function to send a packet pkt in a socket channel. 
 * The "packet" parameter must respect the specified format (see before).
 * Returns 1 for success and 0 for failure
 */
int send_pkt(char *pkt, int channel) {
    int amount_to_send = 17; // value when the field nb contains 0, 
    int amount_sent;
    char *buf = pkt; // pointer to the data to send, begins at pkt

    // compute the amount of data to be sent by looking at the nb field
    if(*pkt == 1) amount_to_send = 49; // nb contains 1
    else if (*pkt == 2) amount_to_send = 81; // nb contains 2

    // send the packet which may require several writes
    while(amount_to_send > 0) { // data remains to send
        amount_sent = write(channel, buf, amount_to_send);
        if (amount_sent == -1) { //error case
            // print error message for closed connection (cf errno.h)
            if(errno == EPIPE)
                fprintf(stderr, "connection closed\n"); 
            else
            // print error-associated message to stderr cf (errno.h)
                perror("cannot write"); 
            return 0;
        }
        if (amount_sent == 0) { // problem: loop risk
            // print relevant error message on stderr
            fprintf(stderr, "write problem\n"); 
            return 0;
        }
        else { // amount_sent data has been sent
            // update amount of data to be sent
            amount_to_send = amount_to_send - amount_sent;
            // points to remaining data
            buf = buf + amount_sent;
        }
    }
    return 1; // if this line is reached, no error occurred.
}
// --------------------------------------------------
//  END of EXAMPLE code to be removed or replaced    |
// --------------------------------------------------
