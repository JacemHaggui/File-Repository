#include <inttypes.h>

typedef struct __attribute__((packed))  { // NO PADDING IN THIS CASE ONLY <!>
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

void free_packet(Packet * packet);

void print_packet(Packet * packet);

int string_to_packet (char * string, Packet * packet);

