#ifndef _NETTOOLS_H
#define _NETTOOLS_H

#include <stdio.h>
#include <netdb.h>

#define Err_SOCKET   0
#define Err_BIND     1
#define Err_PORT     2
#define Err_LISTEN   3
#define Err_NOADDR   4
#define Err_CONNECT  5
#define Err_GETADDR  6
#define Err_GETIFADDRS  7
extern int nettools_errcode;

/* Get the error message associatd to the error that have just occured.
   The message is updated each time the function is called, thus if you
   want to keep it, copy it somewhere.
 */
char *nettools_errmsg();

/* Get listening socket. Returns the socket. Port parameter is updated with the found port.
 * In case of error: -1 is returned, nettools_errcode is updated, and nettools_errmsg
 * can be used (before any other call of nettools function).
 * Errors may be Err_SOCKET, Err_BIND, Err_PORT, Err_LISTEN.
 * IPV4 supported, and IPV6 if macro IPV6 is defined.
 */
int getListeningSocket (uint16_t *port, int queue_size);

/* connects sock to the service at address:port 
 * Returns 0 in case of success
 * In case of error: -1 is returned, nettools_errcode is updated, and nettools_errmsg
 * can be used (before any other call of nettools function).
 * Errors may be Err_GETADDR, Err_NOADDR, Err_CONNECT.
 */
int setConnectedSocket(char *addr, char *port, int *sock);

/* Prints the list of IPv4/IPv6 network interfaces on file provided by parameter.
 * Returns 0 in case of success
 * In case of error: -1 is returned, nettools_errcode is updated, and nettools_errmsg
 * can be used (before any other call of nettools function).
 * Errors may be Err_GETIFADDRS.
 */
int print_inetinterfaces(FILE *file) ;

#endif