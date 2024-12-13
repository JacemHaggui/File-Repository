#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdbool.h> // bool type
#include <sys/stat.h> // stat
#include <dirent.h> // For directory handling.

char *  cats(char* dest, char* source){
  char* both = malloc(sizeof(char) * (strlen(source) + strlen(dest)));
  strcpy(both, dest);
  strcat(both, source);
  return both;
}


// Converts an int to string
char* itoa(int val, int base){
	static char buf[32] = {0};

	int i = 30;

	for(; val && i ; --i, val /= base)

		buf[i] = "0123456789abcdef"[val % base];

	return &buf[i+1];
}

/* Returns the number of lines in a string. */
int line_count(char string[]) {
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
	/*  Set the directory path for the server side.
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
	/*  Set the directory path for the client side.
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
	/*  Apply the convention to the server directory string, Must ends with '/', and change it if it's not the case.
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
	/*  Apply the convention to the client directory string, Must ends with '/', and change it if it's not the case.
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



