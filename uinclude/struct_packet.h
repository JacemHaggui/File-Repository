#ifndef STRUCT_PACKET_H
#define STRUCT_PACKET_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <ctype.h>

#include <stddef.h> // DEBUG ONLY



// ORIGIN GROUP: Command Codes for GROUP
#define CMD_RESTART   8   // Restart command
#define CMD_QUIT      7   // Quit or exit command

// ORIGIN PROF: Command Codes for PROF
#define CMD_LIST      6   // List remote files
#define CMD_GET       5   // Get a remote file
#define CMD_REMOVE    4   // Remove a remote file
#define CMD_RENAME    3   // Rename a remote file
#define CMD_ADD       2   // Add a remote file
#define CMD_PRINT     1   // Print n lines of a file


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

Packet * empty_packet();

Packet * error_packet(int errcode);

void free_packet(Packet * packet);

void print_packet(Packet * packet);

void print_string(char * str, int n);

int string_to_packet (char * string, Packet * packet);

int packet_to_string(Packet * packet, char * string);

int CmdlinetoPacket(const char *input, Packet *pkt);

void print_packet(Packet * packet);

//int student_client(int argc, char *argv[]);//int student_client(int channel, int argc, char *argv[])

int CmdlinetoPacket(const char *input, Packet *pkt);

void print_response(const Packet *packet);

#endif // STRUCT_PACKET_H