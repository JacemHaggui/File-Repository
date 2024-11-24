#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

typedef struct {
	uint16_t data_size;
	int8_t code;
	char * option1;
	char * option2;
	char * data;
} Packet ;

Packet * create_empty_packet() {
	Packet * packet = malloc(sizeof(Packet));
	return packet;
}


int string_to_packet (char * string, int n, Packet * packet) {
	/*
	Convert a string argument to a packet struct given
	INPUT :
		string	: the string to convert
		n	: the length of the string to convert
		packet	: the pointer to a packet which will be overwritted
	OUTPUT :
		0	: Conversion is complete
		-1 	: Error Code - Empty String
	*/
	//int size_byte = sizeof(uint8_t) ; //  1 byte is equivalent to the size of 1 uint_t
	
	if ( ! *string || n == 0) { return -1 ; }  // String is Empty
	
	char * ptr = string ; // ptr is pointing to the first element in string
	
	if ( n >= 3 &&  *ptr != 'E' || *(++ptr) != 'D' || *(++ptr) != 'r' ) { return -2; } // The Constants are not there
	n -= 3;
	
	data_size = *(uint16_t *)(++ptr);


	
	return 0;

	
	
}


void main() {
	Packet * packet = create_empty_packet();
	char * string = "EDr GE GH";
	int n = strlen(string);

	int code = string_to_packet(string, n, packet);
	printf("CODE : %d\n", code );
}
