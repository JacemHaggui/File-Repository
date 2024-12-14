#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdbool.h>  // bool type
#include <sys/stat.h>  // stat
#include <dirent.h>  // For directory handling.
#include "../uinclude/struct_packet.h"
#include "../include/utilities.h"
#include "../uinclude/functions.h"



char *  cats(char* dest, char* source){
  /*  
    Concatenate dest and source into a new string both.
  INPUT :
    dest : Source string that will be copied into a new string
    source : Source string that will be concatenated to the end of dest
  OUTPUT :
    both : New string containing the concatenation of dest and source
  */
  char* both = malloc(sizeof(char) * (strlen(source) + strlen(dest)));
  strcpy(both, dest);
  strcat(both, source);
  return both;
}


char* itoa(int val, int base) {
  /*  
    Converts an int to string.
  INPUT :
    val : The integer value to be converted
    base : The numerical base for the conversion
  OUTPUT :
    &buf[i+1] : Pointer to the string representation of the integer in the specified base
  */
	static char buf[32] = {0};
	int i = 30;
	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	return &buf[i+1];
}


int line_count(char string[]) {
  /*  
    Returns the number of lines in a string.
  INPUT :
    string[] : The string to analyze
  OUTPUT :
    c : Number of lines in the string
  */
  int leng = strlen(string);
  if (leng == 0) {
    return 0;
  }
  int c = 1;
  for (int i = 0; i < leng; i++) {
    if (string[i] == '\n') {
      c++;
    }
  }
  return c;
}

bool file_exists(char *filename) { // Checks for file existence.
  struct stat buffer;
  return (stat(filename, &buffer) == 0);
}


char * SERVER_DIRECTORY = NULL; // DEFINE NULL at the beginning (Will be updated in student_server function)
char * CLIENT_DIRECTORY = NULL; // DEFINE NULL at the beginning (Will be updated in student_client function)


void set_server_directory(const char *string) {
  /*  
    Set the directory path for the server side.
  INPUT :
    string : The New Absolute Path (or relative in fact) to the server working directory
  OUTPUT :
  */
  // IF SERVER_DIRECTORY IS ALREADY FILLED -> FREE IT
  if (SERVER_DIRECTORY != NULL) {
      free(SERVER_DIRECTORY);
      SERVER_DIRECTORY = NULL;
  }

  SERVER_DIRECTORY = malloc((strlen(string) + 1) * sizeof(char));
  if (SERVER_DIRECTORY == NULL) {
      // Only if malloc didn't succeed
      printf("Error Malloc SERVER_DIRECTORY\n");
      return;
  }

    strcpy(SERVER_DIRECTORY, string); // Fill SERVER_DIRECTORY Global Variable with the content of string 
}


void set_client_directory(const char *string) {
  /*  
    Set the directory path for the client side.
  INPUT :
    string : The New Absolute Path (or relative in fact) to the client working directory
  OUTPUT :
  */
    // IF CLIENT_DIRECTORY IS ALREADY FILLED -> FREE IT
    if (CLIENT_DIRECTORY != NULL) {
        free(CLIENT_DIRECTORY); 
        CLIENT_DIRECTORY = NULL;
    }

    CLIENT_DIRECTORY = malloc((strlen(string) + 1) * sizeof(char));
    if (CLIENT_DIRECTORY == NULL) {
        // Only if malloc didn't succeed
        printf("Error Malloc CLIENT_DIRECTORY\n");
        return;
    }

    strcpy(CLIENT_DIRECTORY, string); // Fill CLIENT_DIRECTORY Global Variable with the content of string 
}


void force_server_directory_format(){
  /*  
    Apply the convention to the server directory string, Must ends with '/', and change it if it's not the case.
  INPUT :
  OUTPUT :
  */
	if (SERVER_DIRECTORY == NULL) {
		printf("SERVER DIRECTORY PATH == NULL");
		return;
	}

	int server_directory_size = strlen(SERVER_DIRECTORY);
	if (SERVER_DIRECTORY[server_directory_size - 1] != '/') {
		set_server_directory(cats(SERVER_DIRECTORY, "/"));
	}
} 


void force_client_directory_format(){
  /*  
    Apply the convention to the client directory string, Must ends with '/', and change it if it's not the case.
  INPUT :
  OUTPUT :
  */
	if (CLIENT_DIRECTORY == NULL) {
		printf("CLIENT DIRECTORY PATH == NULL");
		return;
	}

	int client_directory_size = strlen(CLIENT_DIRECTORY);
	if (CLIENT_DIRECTORY[client_directory_size - 1] != '/') {
		set_client_directory(cats(CLIENT_DIRECTORY, "/"));
	}
} 

int count_caracter_inside_n_first_lines(char * file, int n) {
  int l = 0;
  int i = 0;
  int length_file = strlen(file);
  int count = 0;

  for (i = 0; i < length_file && l < n; i++) {
    count++;
    if (file[i] == '\n') {
      l++;
    }
  }

  return count;
}



int convert_cmd_string_to_packet_string(char * cmd, char * string) {
  /*
    Convert a string to a packet format.
    The function uses a defined packet structure to transform a command-line string into a structured packet, then converts the packet into a string format.
  INPUT :
    cmd : The command-line string to be converted
    string : A buffer to store the resulting packet string
  OUTPUT :
    error_code : Error code from the packet_to_string function
    0 : Success
  */
  Packet * packet = empty_packet();

  usercommand parsed_cmd;
  int test = parse_commandline(&parsed_cmd, cmd);
  if (test) { // parsing was successful
      /*
      parsed_cmd :
          cmd : 16
          param1 : 32
          param2 : 32
      */
    if (strcmp(parsed_cmd.cmd,"put") == 0) {
      
      packet->code = CMD_ADD;
      memcpy(packet->option1, parsed_cmd.param1, 32);
      //packet->option1 = parsed_cmd.param1;
      // WHERE ARE THE DATA TO TRANSMIT ? In my cucu ?
    }
    else if (strcmp(parsed_cmd.cmd,"rm") == 0) {
      packet->code = CMD_REMOVE;
      memcpy(packet->option1, parsed_cmd.param1, 32);
    }
    else if (strcmp(parsed_cmd.cmd,"get") == 0) {
      packet->code = CMD_GET;
      memcpy(packet->option1, parsed_cmd.param1, 32);
      memcpy(packet->option2, parsed_cmd.param2, 32);
    }
    else if (strcmp(parsed_cmd.cmd,"ls") == 0) {
      packet->code = CMD_LIST;
    }
    else if (strcmp(parsed_cmd.cmd,"cat") == 0) {
      packet->code = CMD_PRINT;
      memcpy(packet->option1, parsed_cmd.param1, 32);
      memcpy(packet->option2, parsed_cmd.param2, 32);
    }
    else if (strcmp(parsed_cmd.cmd,"mv") == 0) {
      packet->code = CMD_RENAME;
      memcpy(packet->option1, parsed_cmd.param1, 32);
      memcpy(packet->option2, parsed_cmd.param2, 32);
    }
    else if (strcmp(parsed_cmd.cmd,"quit") == 0 || strcmp(parsed_cmd.cmd,"exit") == 0) {
      packet->code = CMD_QUIT;
    }
    else if (strcmp(parsed_cmd.cmd,"restart") == 0 ) {
      packet->code = CMD_RESTART;
    }
    else if (strcmp(parsed_cmd.cmd,"help") == 0) {
      // DO HELP CMD
    }

    print_packet(packet);

    int error_code = packet_to_string(packet, string);

    return error_code;

  }

  return 0; // A CHANGER

}

