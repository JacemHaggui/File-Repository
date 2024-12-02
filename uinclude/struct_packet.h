#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <ctype.h>

#include <stddef.h> // DEBUG ONLY

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

int student_client(int channel, int argc, char *argv[]);