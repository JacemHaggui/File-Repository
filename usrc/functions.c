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

/* help message for commandline options */
const char * const client_help_options = "\
 client options are:\n\
\n\
 -interactive :\n\
	interactive mode ('better' interface messages)\n\
\n\
 -analyze filename :\n\
	reads commands from filename.\n\
\n\
 -directory string :\n\
	   specifies the directory to be used to store files. If this\n\
	   directory is non empty when the client starts, then existing\n\
	   files are assumed to be part of the local drive.\n\
\n\
";

int write_to_file(char filepath[], char data[]) { // FILENAME IS NOT ENOUGH. FILEPATH
                                         // MUST CONTAIN THE PATH TO THE FILE!
    /*
      Open a file with its filename, and convert it into the text string given in parameter
      Output an array : [number of caracters, number of lines] based on the file stats.
    INPUT :
        filepath : The path to the file to process and to open
        data : data that will be written inside the file
    OUTPUT : ERROR CODES
      FILE_ALREADY_EXISTS
      SUCCESS
    */


  if (file_exists(filepath)) {
    printf("ERROR: File %s already exists on directory!\n", filepath);
    printf("No modifications will be made.\n");
    return FILE_ALREADY_EXISTS; // FILE ALREADY EXISTS.
  }


  FILE *new;
  new = fopen(filepath, "w");
  fputs(data, new);
  fclose(new);
  return SUCCESS; // Success !
}

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

void slice(const char* str, char* result, size_t start, size_t end) {
  /*
      Extract a substring from the input string str and store it in result.
    INPUT:
        str    : The original string from which a slice is extracted.
        result : A pre-allocated buffer to store the resulting substring.
        start  : The starting index (inclusive) of the slice in the original string.
        end    : The ending index (exclusive) of the slice in the original string.
    OUTPUT:
  */
    strncpy(result, str + start, end - start);
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
int QUOTASIZE = -1;             // max number of bytes available in directory. INITIALISED at "no quota"
int QUOTANUMBER = -1;           // max number of files available in directory INITIALISED at "no quota"

void set_quota_size(int qs){
  QUOTASIZE = qs;
}

void set_quota_number(int qn){
  QUOTANUMBER = qn;
}

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
  int l = 0;  // Compteur de lignes
  int count = 0;  // Compteur de caractères

  // Parcourir le fichier caractère par caractère
  for (int i = 0; file[i] != '\0'; i++) {
      count++; // Compter chaque caractère
      if (file[i] == '\n') {
          l++; // Compter les sauts de ligne
          if (l == n) {
              break; // Si on atteint n lignes, on arrête
          }
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
    Success 
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
      char * filename = parsed_cmd.param1;

      // Create a buffer large enough to hold both strings and the additional slash
      //char filepath[ strlen(filename) + strlen(CLIENT_DIRECTORY) + 2 ];
      // concatenates the directory and filename adding a slash at the end to respect the format
      //snprintf(filepath, sizeof(filepath), "%s%s/", CLIENT_DIRECTORY, filename);// tested and works


      char * filepath = cats(CLIENT_DIRECTORY, filename);

      printf("Attempting to read file contents of: %s\n", filepath);
      char * file_data = read_file(filepath);
    
      if (file_data == NULL) {
          // Handle error, e.g., file not found
          return FILE_NOT_FOUND;
      }
      
      uint16_t datasize = strlen(file_data) + 1; // Include null terminator
      if (datasize > MAX_DATA_SIZE) {
        return QUOTA_EXCEEDED;
        // TODO: Continuous sending
      }
      else {
        packet->code = CMD_ADD;
        memcpy(packet->option1, parsed_cmd.param1, 32);

        // Allocate memory for data_ptr 
        packet->data_ptr = (char *) malloc(datasize);
        if (packet->data_ptr == NULL) {
          printf("Memory Allocation Failed");
          free(packet->data_ptr);
          return 1;
        }
        // Copy file_data
        memcpy(packet->data_ptr, file_data, datasize);

        packet->data_size = datasize;
      }
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
      //we quit and don't send the packet, check with the group
      return CMD_QUIT;
    }
    else if (strcmp(parsed_cmd.cmd,"restart") == 0 ) {
      //we restart and don't send the packet, check with the group
      return CMD_RESTART;
    }
    else if (strcmp(parsed_cmd.cmd,"help") == 0) {
      printf("%s\n",client_help_options);
    }

    print_packet(packet);

    int error_code = packet_to_string(packet, string);
    free_packet(packet);

    return error_code;

  }

  return SUCCESS; 

}

char* read_file(char *filename) {
    FILE *file = fopen(filename, "r");  // Open the file in read mode

    if (file == NULL) {
        printf("Could not open file\n");
        printf("The filepath is: %s\n", filename);
        return NULL;
    }

    // Move the file pointer to the end to get the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);  // Move back to the beginning of the file

    // Allocate memory for the file content
    char *content = (char *)malloc(file_size + 1);
    if (content == NULL) {
        printf("Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    // Read the entire file into the buffer
    fread(content, 1, file_size, file);
    content[file_size] = '\0';  // Null-terminate the string

    fclose(file);  // Close the file
    return content;
}