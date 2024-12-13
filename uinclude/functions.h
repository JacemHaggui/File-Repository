#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdbool.h> // bool type
#include <sys/stat.h> // stat
#include <dirent.h> // For directory handling.

char *  cats(char* dest, char* source);

char* itoa(int val, int base);

int line_count(char string[]);

bool file_exists(char *filename);

// DIRECTORY  VARIABLES HANDLER
extern char * SERVER_DIRECTORY; // SERVER WORKING DIRECTORY
extern char * CLIENT_DIRECTORY; // CLIENT WORKING DIRECTORY

void set_server_directory(const char *string);
void set_client_directory(const char *string);

void force_server_directory_format();
void force_client_directory_format();