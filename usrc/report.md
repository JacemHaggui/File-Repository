# **Report**


## **How to launch the program**  

To launch the program, you have to enter `make` in the command prompt.  
Then you can type all these instructions :

`put filename` : to add a remote file called filename

`rm filename` : to remove a remote file called filename

`get filename localfilename` : to get a remote file called filename

`ls` : to list remote files

`cat filename n` : to print the first n lines of a file called filename

`mv originfilename destinationfilename` : to rename a remote file called originfilename

`quit` or `exit` : to exit

`restart` : to reset the connection

`help` : to have the description of the available commands

<br />

## **Functions**  

(input, Output)

<br />

## **Packet Structure**  

We've implemented a packet structure that simplifies code accessibility. 

```c
#ifndef STRUCT_PACKET_H
#define STRUCT_PACKET_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <ctype.h>

#include <stddef.h> // DEBUG ONLY

// PACKET STRUCTURE
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
```
Each packet begins with E, D and r so we can recognise the packet.  

Then we implemented methods that use the packet structure.  

The function ```string_to_packet(char * string, Packet * packet)``` converts a string argument to a struct packet given.

The function ```packet_to_string(Packet * packet, char * string)``` converts a packet given in a string format.


<br />

## **Server & Client Directory**
utilisation d'une variable globale????



<br />

## **Example of communication between client and server**

The client wants to print 3 lines of the file :  

<br />

```make```  

```cat MyFile.txt 3``` 

<br />

The string of the first 3 lines of MyFile.txt is converted into a packet format. The packet is then converted into a 32-bit “packet string” starting with E, D, r, then converted into 0's and 1's and sent to the server.

The server converts the 0s and 1s into a “packet string”, then into a packet and analyzes it. It then sends the first 3 lines with the same process back to the client, who will see :  

<br />

```First line.``` 

```Second line.```  

```Third line.```  