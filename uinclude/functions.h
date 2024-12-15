#ifndef STRUCT_FUNCTIONS_H
#define STRUCT_FUNCTIONS_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdbool.h> // bool type
#include <sys/stat.h> // stat
#include <dirent.h> // For directory handling.

// DIRECTORY  VARIABLES HANDLER
extern char * SERVER_DIRECTORY; // SERVER WORKING DIRECTORY
extern char * CLIENT_DIRECTORY; // CLIENT WORKING DIRECTORY
extern int QUOTASIZE;      // max number of bytes available in directory. 
extern int QUOTANUMBER;    // max number of files available in directory 


// Define constants for packet
#define HEADER_SIZE 70
#define MAX_PACKET_SIZE 2048
#define MAX_DATA_SIZE (MAX_PACKET_SIZE - HEADER_SIZE)
// Defining error codes
#define BAD_PACKET_FORMAT    -1
#define FILE_NOT_FOUND       -2
#define FILE_ALREADY_EXISTS  -3
#define COMMAND_FAILS        -4
#define QUOTA_EXCEEDED       -5
#define SYNTAX_ERROR         -6
#define BAD_SERVER_RESPONSE  -7
#define CONNECTION_CLOSED    -8
#define CANNOT_READ          -9

#define SUCCESS              0

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


char *  cats(char* dest, char* source);

char* itoa(int val, int base);

void slice(const char* str, char* result, size_t start, size_t end);

int write_to_file(char filepath[], char data[]);

int line_count(char string[]);

bool file_exists(char *filename);

void set_quota_size(int qs);
void set_quota_number(int qn);

void set_server_directory(const char *string);
void set_client_directory(const char *string);

void force_server_directory_format();
void force_client_directory_format();


int count_caracter_inside_n_first_lines(char * file, int n);

int convert_cmd_string_to_packet_string(char * cmd, char * string);

char* read_file(const char *filename);

#endif // STRUCT_FUNCTIONS_H