#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include <stddef.h> // DEBUG ONLY

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

Packet * empty_packet() {
	//Packet * packet = calloc(1, sizeof(*packet));
	Packet * packet = (Packet *)malloc(sizeof(Packet));
	return packet;
}

void free_packet(Packet * packet) {
	if (packet) free(packet);
}

void print_packet(Packet * packet) {
	printf("Print Packet :\n");
	if (! packet) 	{printf("\t-Empty Packet\n");  return ;} 
	if ( packet->E) printf("\t-Const E : %c\n", packet->E); else printf("\t-No Const E\n");
	if ( packet->D) printf("\t-Const D : %c\n", packet->D); else printf("\t-No Const D\n");
	if ( packet->r) printf("\t-Const r : %c\n", packet->r); else printf("\t-No Const r\n");
	if ( packet->data_size) printf("\t-Data Size : %d bytes\n", packet->data_size); else printf("\t-No Data Size\n");
	if ( packet->code) printf("\t-Code : %d\n", packet->code); else printf("\t-No Code\n");
	if ( packet->option1) printf("\t-Option 1 : %s\n", packet->option1); else printf("\t-No Option 1\n");
	if ( packet->option2) printf("\t-Option 2 : %s\n", packet->option2); else printf("\t-No Option 2\n");
	if (packet->data_ptr) printf("\t-Data Pointer provided ? : %d (1 <=> True)", *(packet->data_ptr) != '\0'  ); else printf("\t-No Data Pointer Provided");
}



int string_to_packet (char * string, Packet * packet) { /* Convert a string argument 
	to a packet struct given INPUT :
		string	: the string to convert
		packet	: the pointer to a given packet which will be overwritted
	OUTPUT :
		0	: Conversion is complete
		-1 	: Error Code - Empty String
		-2	: Error Code - Constant Value (E,D,r) not here
		-3	: Error Code - Option 1 is too long
		-4	: Error Code - Option 2 is too long
	*/
		if ( ! *string) { return -1 ; }  // String is Empty

	char * ptr = string ; // ptr is pointing to the first element in string

	// CONSTANTS PART
	if (*ptr) 	packet->E = *ptr;  	else return -2 ;
	if (*(++ptr))	packet->D = *(ptr);	else return -2 ;
	if (*(++ptr))	packet->r = *(ptr);	else return -2 ;

	// DATA SIZE PART
	uint16_t data_size = (uint16_t)(  (unsigned char)(  (unsigned char)(*(++ptr)) ) | (unsigned char)(  (unsigned char)( *(++ptr) ) << 8 ) );
	packet->data_size = data_size;

	// CODE PART
	uint8_t code = *(uint8_t *)(++ptr);
	packet->code = code;


	// OPTION 1 PART
	int i = 0;
	++ptr; // Skip the code final character
	while(*(ptr) != '\0') {
		if (i > 31) return -3;
		packet->option1[i] = *ptr;
		i ++;
		++ptr;
	}

	// OPTION 2 PART
	int j = 0;
	++ptr; // Skip the option1 last character : '\0'
	while(*(ptr) != '\0' ) {
		if (j > 31) return -4 ;
		packet->option2[j] = *ptr;
		j ++;
		++ptr;
	}

	// DATA PART
	++ptr; // Skip the option2 last character : '\0'
	packet->data_ptr = ptr;

	return 0;




}


void main() {


	Packet * packet = empty_packet();
	char * string1 = "EDr\x00\x0A\x02OptionA\0Another option\0Hello\0XXX";
	char * string2 = "EDr\x00\x0A\x02Opt1\0Opt2\0";
	char * string3 = "EDr\x01\xF4\x05ThisIsOption1WithFullLength1234\0ThisIsOption2WithFullLength6543\0";
	char * string4 = "EDr\x00\x14\x03\0\0";
	char * string5 = "EDr\x00\x10\x04OptPartiallyFilled\0";
	char * string6 = "EDr\x00\x64\x07OptionEndingHere123\0AnotherOptionBoundary123\0data \0";
	char *string_list[] = {
        	string1,
        	string2,
        	string3,
        	string4,
        	string5,
        	string6
	};

	for (int i = 1; i < 7 ; i ++) { 
		Packet * packet = empty_packet();
		int code = string_to_packet(string_list[i-1], packet);
		printf("--- STRING %d --- \n", i );
		print_packet(packet);
		printf("\nCode %d\n", code);
	}
	free_packet(packet);
}
