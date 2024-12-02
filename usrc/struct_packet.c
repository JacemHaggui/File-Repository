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

Packet * empty_packet() {
	Packet * packet = calloc(1, sizeof(*packet));
	packet->E = 'E'; packet->D = 'D'; packet->r = 'r';
	//Packet * packet = (Packet *)malloc(sizeof(Packet));
	return packet;
}

Packet * error_packet(int errcode){
	Packet * out = empty_packet();
	out->data_size = 0; out->code = errcode;
	return out;
}

void free_packet(Packet * packet) {
	/* Free Packet if exist */
	if (packet) free(packet);
}

void print_packet(Packet * packet) {
	/* 
		ONLY FOR DEBUGGING : 
		Print the Packet in a decent format
	*/
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

void print_string(char * str, int n) {
	/* 
		ONLY FOR DEBUGGING :
		Print the first n characters in the string.
		<!> 0x0000 and '\0' are the same character in C. They will be shown as '\0' .
	*/
	for (int i = 0; i < n ; i ++) {
		unsigned char c = (unsigned char) str[i];
		if (c == '\0') printf("\\0");
		else if (isprint(c)) printf("%c", c);
		else printf("\\x%02X", c);
	}
	putchar('\n');
}



int string_to_packet (char * string, Packet * packet) { 
	/* Convert a string argument to a struct packet given 
	INPUT :
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

int packet_to_string(Packet * packet, char * string) {
	/*
	Convert a packet given in a string format.
	INPUT :
		packet : The packet to transform in a string
		string : The string which will be filled in place
	OUTPUT :
		0 : R.A.S (All goooooood)
	*/
	*(string) 	= packet->E;
	*(++string) 	= packet->D;
	*(++string) 	= packet->r;
	*(++string)	= ((packet->data_size & 0xF0) >> 8) ; // First get the 8 upper bit with the mask then shift right 8 times to remove the lower part
	*(++string)	= (packet->data_size & 0x0F) ; // Then get the 8 lower bits with an another mask 
	*(++string)	= packet->code;

	int n_opt1 = strlen(packet->option1);
	for (int i = 0; i < n_opt1 ; i ++ ) {
		*(++string) = packet->option1[i];
	}
	*(++string)	= '\0'; // ADD NUL-TERMINATOR

	int n_opt2 = strlen(packet->option2);
	for (int i = 0; i < n_opt2 ; i ++) {
		*(++string)	= packet->option2[i];
	}
	*(++string)	= '\0'; // ADD NUL-TERMINATOR
	
	char * ptr = packet->data_ptr;
	for (int i = 0; i < packet->data_size; i ++) {
		*(++string)	= *(ptr++);
	}

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
	/*
	for (int i = 1; i < 7 ; i ++) { 
		Packet * packet = empty_packet();
		int code = string_to_packet(string_list[i-1], packet);
		printf("--- STRING %d --- \n", i );
		print_packet(packet);
		printf("\nCode %d\n", code);
	} 
	*/
	//Packet * packet = empty_packet();
	int code = string_to_packet(string_list[0], packet);
	print_packet(packet);
	printf("\nCode %d\n", code);
	int n = 6 + strlen(packet->option1) + 1 + strlen(packet->option2) + 1 + packet->data_size;
	char * string_receiver = calloc(sizeof(packet) + packet->data_size , sizeof(string1));
	packet_to_string(packet, string_receiver);
	print_string(string_receiver, n);
	free_packet(packet);
}
