// --------------------------------------------------
//  EXAMPLE.  Replace it withyour project code      |
// --------------------------------------------------
/* In this example, the server receives command line arguments
 * from the client sent using the following packet format
 *    ----------------------------------------
 *   | nb | command | parameter1 | parameter2 |
 *    ----------------------------------------
 * where
 * - nb (1 byte) is the number of parameters
 * - command (16 bytes) is a (null-terminated) string
 * - parameter1 (32 bytes) is a (null-terminated) string
 * - parameter2 (32 bytes) is a (null-terminated) string
 * irrelevant parameters are skipped
 * (w.r.t nb, depending of the command )
 *
 * Once a packet has been parsed, the server writes the received arguments
 * int its terminal
 */

// INCLUDES
/* user defined library for sending and receiving packets */
#include "../uinclude/communication.h"
/* for printf,...*/
#include <stdio.h>
/* Used to modify SIGPIPE signal handling, as the default behaviour makes
 * the program exits when trying to write to a closed socket.
 * We don't want thi :-).
 */
#include <../include/utilities.h>
#include <../uinclude/client.h>
#include <signal.h>
#include <stdbool.h> // bool type
#include <string.h>
#include <sys/stat.h> // stat
#include <dirent.h> // For directory handling.


const int INT_MAX = 2048 - 70; 

// FUNCTIONS
// ...
// Empty because all functions used in example are provided by
// "utilities", "communication" and system provided libraries.
// --------------------------------------------------
//  END of EXAMPLE code to be replaced              |
// --------------------------------------------------

/**  help message for commandline options */
const char *const server_help_options = "\
 server options are :\n\
\n\
 -quotasize int :\n\
	maximum number of bytes that the remote directory can store.\n\
\n\
 -quotanumber int :\n\
	maximum number of files that the remote directory can store.\n\
\n\
 -directory string :\n\
	specifies the directory to be used to store files. If this\n\
	directory is non empty when the server starts, then existing\n\
	files are assumed to be part of the remote drive.\n\
\n\
";

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

int print_lines(char string[], int n, char outstring[], bool print_state) { // returns number of packets needed to store.
  int l = 0;
  int i = 0;
  for (i = 0; i < strlen(string) && l < n; i++) {
    if(print_state){ printf("%c", string[i]); }
    outstring[i] = string[i];
    if (string[i] == '\n') {
      l++;
    }
  }
  if (i > INT_MAX){
    return (i / INT_MAX) + 1;
  }
  return 1;
}

char *file_to_string(char filename[]){
  char * buffer = 0;
  long length;
  FILE * f = fopen (filename, "rb");
  if (f)
  {
    fseek (f, 0, SEEK_END);
    length = ftell (f);
    fseek (f, 0, SEEK_SET);
    buffer = malloc (length);
    if (buffer)
    {
      fread (buffer, 1, length, f);
    }
    fclose (f);
    return 0;
  }
  else{
    printf("Error: File '%s' does not exist.\n", filename);
  }
  return buffer;
}


/* TO DO: THESE FUNCTIONS MUST FAIL IF THE FILE ALREADY EXISTS! NO OVERWRITING
 * IS ALLOWED.*/
int write_to_file(char filepath[], char data[],
                   char destination[]) { // FILENAME IS NOT ENOUGH. FILEPATH
                                         // MUST CONTAIN THE PATH TO THE FILE!
  if (file_exists(filepath)) {
    printf("ERROR: File %s already exists on directory!\n", filepath);
    printf("No modifications will be made.\n");
    return -1;
  }

  FILE *new;
  new = fopen(strcat(destination,filepath), "w");
  fprintf(new, data);
  fclose(new);
  return 0;
}

/* Returns -1 if trying to rename to existing file, returns -2 if trying to rename a non-existing file. Returns 0 if done correctly.*/
int rename_file(char newfile[], char oldfile[]) { // full paths need to be given in parameter.
  FILE *oldf;

  if(!file_exist(oldfile)){
    print("ERROR: File %s does not exist on directory!\n", oldfile);
    print("No modifications will be made.\n");
    return -2;
  }

  if (file_exists(newfile)) {
    printf("ERROR: File %s already exists on directory!\n", newfile);
    printf("No modifications will be made.\n");
    return -1;
  }

  oldf = fopen(oldfile, "r");
  char *buffer = 0;
  long length;

  if (oldf) {
    fseek(oldf, 0, SEEK_END);
    length = ftell(oldf);
    fseek(oldf, 0, SEEK_SET);
    buffer = malloc(length);
    if (buffer) {
      fread(buffer, 1, length, oldf);
    }
    fclose(oldf);
  }

  FILE *fptr;

  fptr = fopen(newfile, "w");
  fprintf(fptr, buffer);
  fclose(fptr);
  // Now that we've added the context of the oldfile to the newfile, since we're
  // renaming the file, we have to remove the old one. Note that we have to add
  // the case in which renaming is not possible (e.g if the new filepath already
  // exists.)

  remove(oldfile);
  return 0;
}


void slice(const char* str, char* result, size_t start, size_t end) {
    strncpy(result, str + start, end - start);
}

/*Returns multiple packets.*/
Packet **f_print_n_lines(Packet* in, char directory[]){
  
  
  char *filename = strcat(directory, in->option1);

  char * string = file_to_string(filename);
  char * datastring;

  int packnum = print_lines(filename, atoi(in->option2), datastring, 1);

  Packet ** list = calloc(packnum, sizeof(Packet));
  Packet * out;

  for(int i = 0; i < packnum; i++){
    out = empty_packet();
    char buffer[INT_MAX];
    out->code = 1;
    itoa(packnum, out->option1, 10);
    out->E = 'E'; out->D = 'D'; out->r = 'r'; 
    slice(datastring, buffer, i*packnum, (i+1)*packnum);
    out-> data_size = strlen(buffer);
    out->data_ptr = buffer;
    list[i] = out;
  }
  return list;
}

Packet *add_remote_file(Packet* in, char directory[]){ // Returns Packet for the operation. Packet.code = 0 if correctly done, -1 otherwise (file named filename already exists)
  int errcode = write_to_file(strcat(directory, in->option1), in->data_ptr, directory);
  
  Packet *out = empty_packet();
  out->E = 'E'; out->D = 'D'; out->r = 'r';
  out->code = errcode;
  out->data_size = 0;
  return out;
}

Packet * renamefile(Packet* in, char directory[]){
  int res = rename_file(strcat(directory, in->option2), strcat(directory, in->option1));
  
  Packet *out = empty_packet();
  out->code = res;
  return out;
}

int remove_file(char filename[]) {
  return remove(filename);
}

Packet * removefile(Packet* in, char directory[]){
  int errcode = remove_file(strcat(directory, in->option1));
  
  Packet *out = empty_packet();
  out->E = 'E'; out->D = 'D'; out->r = 'r';
  out->code = errcode;
  out->data_size = 0;
  return out;
}

/*Very similar to print_n_lines */
Packet **fetch(Packet* in, char directory[]){
  char *filename = strcat(directory, in->option1);

  char * string = file_to_string(filename);
  char * datastring;

  int packnum = print_lines(filename, line_count(string), datastring, 0);

  Packet ** list = calloc(packnum, sizeof(Packet));
  Packet * out;

  for(int i = 0; i < packnum; i++){
    out = empty_packet();
    char buffer[INT_MAX];
    out->code = 5;
    itoa(packnum, out->option1, 10);
    out->E = 'E'; out->D = 'D'; out->r = 'r'; 
    slice(string, buffer, i*packnum, (i+1)*packnum);
    out-> data_size = strlen(buffer);
    out->data_ptr = buffer;
    list[i] = out;
  }

  return list;
}

Packet **list_files(Packet* in){
  struct dirent *de;  // Pointer for directory entry 

  // opendir() returns a pointer of DIR type.  
  DIR *dr = opendir("."); 

  if (dr == NULL)  // opendir returns NULL if couldn't open directory 
  { 
      printf("Could not open current directory"); 
      Packet ** list = malloc(sizeof(Packet));
      Packet * out = empty_packet();
      out->code = -1;
      out->E = 'E'; out->D = 'D'; out->r = 'r'; 
      out-> data_size = 0;
      list[0] = out;
      return list; 
  } 
  else{
    char* string;
    while (de = readdir(dr) != NULL) {
      // printf("%s\n", de->d_name); 
      strcat(string, de->d_name);
      strcat(string, ",");
      
      struct stat* restrict buf;
      stat(de->d_name, buf);

      strcat(string, ",");
      strcat(string, buf->st_size);
    }
    closedir(dr);
    string[strlen(string) - 1] = '\0'; // o remove the last comma and add a null terminator at the end.
    char* trash;
    int packnum = print_lines(string, 1, trash,0);

    Packet *out;
    Packet **list;

    for(int i = 0; i < packnum; i++){
      out = empty_packet();
      char buffer[INT_MAX];
      out->code = 6;
      itoa(packnum, out->option1, 10);
      out->E = 'E'; out->D = 'D'; out->r = 'r'; 
      slice(string, buffer, i*packnum, (i+1)*packnum);
      out-> data_size = strlen(buffer);
      out->data_ptr = buffer;
      list[i] = out;
      } return list; 
  }
}
// Copy a remote file to the local filesystem (WIP)

// !! This is the function you must implement for your project.
// Here we provide an implementation related to the example, not
// to the project... return makes the server wait for next client.
void student_server(int channel, int argc, char *argv[]) {
  // Writing to a closed socket causes a SIGPIPE signal, which makes
  // the program exits. The following line prevents this default behaviour.
  // Thus, writing to a closed socket makes the program simply return -1, put
  // the EPIPE in errno: this avoids the program to directly exit. (cf the line
  // with "EPIPE" in send_pkt in usrc/communication.c).
  signal(SIGPIPE, SIG_IGN);

  // --------------------------------------------------
  //  EXAMPLE.  To be replaced by your project code   |
  // --------------------------------------------------

  // buffer to receive packets (max size: 81)
  char recvbuf[81];
  // infinite loop -> use ^C to exit the program
  while (1) {
    // get the command from user, exit if it fails
    printf(" -- wait a packet (^C to exit) --\n");
    // get the packet
    int res = recv_pkt(recvbuf, channel);
    if (!res)
      return; // return if communication error occured
    // prints the command, relying on the number of parameters
    if (*recvbuf == 0)
      printf("     command: %s\n", recvbuf + 1);
    else if (*recvbuf == 1)
      printf("     command: %s   param: %s\n", recvbuf + 1, recvbuf + 17);
    else
      printf("     command: %s   param1: %s   param2: %s\n", recvbuf + 1,
             recvbuf + 17, recvbuf + 49);
  }
  // -----------------------------------------------------
  //  END of EXAMPLE code to be replace by your own code |
  // -----------------------------------------------------
}
