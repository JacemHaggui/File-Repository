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
#include "../include/utilities.h"
#include "../uinclude/client.h"
#include "../uinclude/struct_packet.h"
#include "../uinclude/functions.h"

#include <signal.h>
#include <stdbool.h> // bool type
#include <string.h>
#include <sys/stat.h> // stat
#include <dirent.h> // For directory handling.
#include <stdlib.h>

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

int file_to_string(char *filename, char *text)
{ 
  char c;
  int cpt = 0;

  if(!file_exists(filename)){
    printf("Error: File '%s' does not exist.\n", filename);
    return -2;
    }

  /*Counts size file.*/
  FILE *f = fopen(filename, "r");

  while (fscanf(f, "%c", &c) != EOF)
  {
    cpt = cpt + 1;
  }
  fclose(f);
  /* Stocks file content into a string*/
  text[cpt] = '\0';
  f = fopen(filename, "r");
  cpt = 0;

  while (fscanf(f, "%c", &c) != EOF)
  {
    text[cpt] = c;
    cpt = cpt + 1;
  }
  return 0;
}


int write_to_file(char filepath[], char data[],
                   char destination[]) { // FILENAME IS NOT ENOUGH. FILEPATH
                                         // MUST CONTAIN THE PATH TO THE FILE!
  if (file_exists(filepath)) {
    printf("ERROR: File %s already exists on directory!\n", filepath);
    printf("No modifications will be made.\n");
    return -3; // FILE ALREADY EXISTS.
  }


  FILE *new;
  new = fopen(filepath, "w");
  fputs(data, new);
  fclose(new);
  return 0;
}

/* Returns -1 if trying to rename to existing file, returns -2 if trying to rename a non-existing file. Returns 0 if done correctly.*/
int rename_file(char newfile[], char oldfile[]) { // full paths need to be given in parameter.
  FILE *oldf;

  if(!file_exists(oldfile)){
    printf("ERROR: File %s does not exist on directory!\n", oldfile);
    printf("No modifications will be made.\n");
    return -2;
  }

  if (file_exists(newfile)) {
    printf("ERROR: File %s already exists on directory!\n", newfile);
    printf("No modifications will be made.\n");
    return -3;
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
  fputs(buffer, fptr);
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
Packet **f_print_n_lines(Packet* input, char *directory){
  
 char * filename = cats(directory, input->option1);

  char * stringf = malloc(sizeof(char) * 1000); // Bad allocation!
  int errcode = file_to_string(filename, stringf);

  if(errcode != 0){
    Packet ** single_slot = calloc(1, sizeof(Packet));
    single_slot[0] = error_packet(errcode);
    return single_slot;
  }

  char * datastring = malloc(sizeof(char) * strlen(datastring));

  int packnum = print_lines(stringf, atoi(input->option2), datastring, 1);

  Packet ** list = calloc(packnum, sizeof(Packet));
  Packet * out;

  for(int i = 0; i < packnum; i++){
    out = empty_packet();
    char buffer[INT_MAX];
    out->code = 1;
    strcpy(out->option1, itoa(packnum,10));
    out->E = 'E'; out->D = 'D'; out->r = 'r';
    slice(datastring, buffer, i*INT_MAX, (i+1)*INT_MAX);
    out-> data_size = strlen(buffer);
    out->data_ptr = buffer;
    list[i] = out;
  }
  return list;
}

Packet *add_remote_file(Packet* in, char directory[]){ // Returns Packet for the operation. Packet.code = 0 if correctly done, -1 otherwise (file named filename already exists)
  
  char * filename = cats(directory, in->option1);
  
  int errcode = write_to_file(filename, in->data_ptr, directory);


  if(errcode != 0){
    return error_packet(errcode);
  }

  Packet *out = empty_packet();
  out->E = 'E'; out->D = 'D'; out->r = 'r';
  out->code = errcode;
  out->data_size = 0;
  return out;
}

Packet * renamefile(Packet* in, char directory[]){
  char * oldfilename = cats(directory, in->option1);
  char * newfilename = cats(directory, in->option2);

  int errcode = rename_file(newfilename, oldfilename);

  return error_packet(errcode);
}

int remove_file(char filename[]) {
  if(!remove(filename)){
    return -2;
  }
  else{
    return 0;
  }
}

Packet * removefile(Packet* in, char directory[]){
  int errcode = remove_file(cats(directory, in->option1));
  
  return error_packet(errcode);
}

/* UNFINISHED! */
/* Essentially does the same as */
Packet **fetch(Packet* in, char directory[]){
  char *contents = malloc(sizeof(char) * 2056);
  
  file_to_string(cats(directory, in->option1), contents);

  char * datastring = malloc(sizeof(char) * strlen(contents));

  int packnum = print_lines(contents, line_count(contents), datastring, 0);

  Packet ** list = calloc(packnum, sizeof(Packet));
  Packet * out;

  for(int i = 0; i < packnum; i++){
    out = empty_packet();
    char buffer[INT_MAX];
    out->code = 5;
    strcpy(out->option1, itoa(packnum,10));
    out->E = 'E'; out->D = 'D'; out->r = 'r'; 
    slice(datastring, buffer, i*INT_MAX, (i+1)*INT_MAX);
    out-> data_size = strlen(buffer);
    out->data_ptr = buffer;
    list[i] = out;
  }
  return list;
}

Packet **list_files(Packet* in, char destination[]){
  struct dirent *de;  // Pointer for directory entry 

  // opendir() returns a pointer of DIR type.  
  DIR *dr = opendir(destination); 

  if (dr == NULL)  // opendir returns NULL if couldn't open directory 
  { 
      printf("Could not open current directory\n");
      Packet ** list = malloc(sizeof(Packet));
      list[0] = error_packet(FILE_NOT_FOUND); // FILE NOT FOUND (here it's a directory)
      return list; 
  } 
  else{
    printf("Directory opened !\n");
    char * string;
    while ((de = readdir(dr)) != NULL) {
      // printf("%s\n", de->d_name); 
      
      string = cats(string, de->d_name);
      string = cats(string, ",");
      //strcat(string, de->d_name);
      //strcat(string, ",");
      
      struct stat* restrict buf;
      stat(de->d_name, buf);
	
      string = cats(string, ",");
      string = cats(string, itoa(buf->st_size, 10));
      //strcat(string, ",");
      //strcat(string, buf->st_size);
    }
    closedir(dr);
    string[strlen(string) - 1] = '\0'; // o remove the last comma and add a null terminator at the end.
    char* trash;
    int packnum = print_lines(string, 1, trash,0);

    //Packet *out;
    Packet **list = calloc(packnum, sizeof(Packet));

    for(int i = 0; i < packnum; i++){
      Packet * out = empty_packet();
      char *buffer = malloc(INT_MAX * sizeof(char));
      out->code = 6;
      strcpy(out->option1, itoa(packnum, 10));
      //out->E = 'E'; out->D = 'D'; out->r = 'r';
      slice(string, buffer, i*packnum, (i+1)*packnum);
      out-> data_size = strlen(buffer);
      out->data_ptr = buffer;
      list[i] = out;
      }
      return list;
  }
}
// Copy a remote file to the local filesystem (WIP)

int process_packet(Packet * packet, int channel) {
  /* 
		Process the packet received in parameter, launch the function dedicated to it
	INPUT :
    packet: packet to be processed
	OUTPUT :
		...
	*/
  // PRINT N LINES
  if (packet->code == CMD_PRINT) {  
    Packet ** list_packet_to_send = f_print_n_lines(packet, SERVER_DIRECTORY);
    Packet * first_packet = list_packet_to_send[0];

    // HOW MANY PACKETS SHOULD WE SEND ?
    int number_packet_to_send = 0;
    int error_code = first_packet->code;
    if (error_code < 0) { // ERROR HAPPENED
      number_packet_to_send = 1 ; // ERROR ONLY ONE PACKET WILL BE SENT
    } else {
      number_packet_to_send = atoi(first_packet->option1); //OPTION 1 contains the number of packets
    }

    // SEND THE PACKETS
    for (int i = 0; i < number_packet_to_send; i ++) {
        char packet_string_to_send[2048];
        int error_code = packet_to_string(list_packet_to_send[i], packet_string_to_send);
        int res = send_pkt(packet_string_to_send, channel);
        printf("Packet %d : \tError Code : %d\tSend Code: %d\n", i, error_code, res);
    }

  }

  // ADDING A REMOTE FILE
  else if (packet->code == CMD_ADD) {
    //ADD THE REMOTE FILE USING THE PACKET DATA
    Packet * packet_error_code = add_remote_file(packet, SERVER_DIRECTORY);
    
    // SEND PACKET WITH THE ERROR CODE ASSOCIATED TO THE CREATION OF THE REMOTE FILE
    char packet_string_to_send[2048];
    int error_code = packet_to_string(packet_error_code, packet_string_to_send);
    int res = send_pkt(packet_string_to_send, channel);
    printf("Packet Send : \tError Code : %d\tSend Code: %d\n",  error_code, res);
  }


  // RENAMING A REMOTE FILE
  else if (packet->code == CMD_RENAME) {
    // RENAME THE REMOTE FILE
    Packet * packet_error_code = renamefile(packet, SERVER_DIRECTORY);

    // SEND PACKET WITH THE ERROR CODE ASSOCIATED TO THE RENAMING OF THE REMOTE FILE
    char packet_string_to_send[2048];
    int error_code = packet_to_string(packet_error_code, packet_string_to_send);
    int res = send_pkt(packet_string_to_send, channel);
    printf("Packet Send : \tError Code : %d\tSend Code: %d\n",  error_code, res);
  }


  // REMOVING A REMOTE FILE
  else if (packet->code == CMD_REMOVE) {
    // REMOVING THE REMOTE FILE
    Packet * packet_error_code = removefile(packet, SERVER_DIRECTORY);

    // SEND PACKET WITH THE ERROR CODE ASSOCIATED TO THE REMOVAL OF THE REMOTE FILE
    char packet_string_to_send[2048];
    int error_code = packet_to_string(packet_error_code, packet_string_to_send);
    int res = send_pkt(packet_string_to_send, channel);
    printf("Packet Send : \tError Code : %d\tSend Code: %d\n",  error_code, res);
  }


  // GETTING A REMOTE FILE
  else if (packet->code == CMD_GET) {
    // GET THE DATA INSIDE THE FILE
    Packet ** list_packet_to_send = fetch(packet, SERVER_DIRECTORY);
    Packet * first_packet = list_packet_to_send[0];

    // HOW MANY PACKETS SHOULD WE SEND ?
    int number_packet_to_send = 0;
    int error_code = first_packet->code;
    if (error_code < 0) { // ERROR HAPPENED
      number_packet_to_send = 1 ; // ERROR ONLY ONE PACKET WILL BE SENT
    } else {
      number_packet_to_send = atoi(first_packet->option2); //OPTION 2 contains the number of packets
    }

    // SEND THE PACKETS
    for (int i = 0; i < number_packet_to_send; i ++) {
        char packet_string_to_send[2048];
        int error_code = packet_to_string(list_packet_to_send[i], packet_string_to_send);
        int res = send_pkt(packet_string_to_send, channel);
        printf("Packet %d : \tError Code : %d\tSend Code: %d\n", i, error_code, res);
    }
  }


  // LISTING REMOTE FILES
  else if (packet->code == CMD_LIST) {
    // LIST THE FILES
    Packet ** list_packet_to_send = list_files(packet, SERVER_DIRECTORY);
    Packet * first_packet = list_packet_to_send[0];

    // HOW MANY PACKETS SHOULD WE SEND ?
    int number_packet_to_send = 0;
    int error_code = first_packet->code;
    if (error_code < 0) { // ERROR HAPPENED
      number_packet_to_send = 1 ; // ERROR ONLY ONE PACKET WILL BE SENT
    } else {
      number_packet_to_send = atoi(first_packet->option1); //OPTION 1 contains the number of packets
    }

    // SEND THE PACKETS
    for (int i = 0; i < number_packet_to_send; i ++) {
        char packet_string_to_send[2048];
        int error_code = packet_to_string(list_packet_to_send[i], packet_string_to_send);
        int res = send_pkt(packet_string_to_send, channel);
        printf("Packet %d : \tError Code : %d\tSend Code: %d\n", i, error_code, res);
    }

  }

}


void student_server(int channel, int argc, char *argv[]) {
  // Writing to a closed socket causes a SIGPIPE signal, which makes
  // the program exits. The following line prevents this default behaviour.
  // Thus, writing to a closed socket makes the program simply return -1, put
  // the EPIPE in errno: this avoids the program to directly exit. (cf the line
  // with "EPIPE" in send_pkt in usrc/communication.c).
  signal(SIGPIPE, SIG_IGN);


  set_server_directory("./"); // DIRECTORY DEFAULT SERVER IS "./"
  printf("\nCURRENT DIRECTORY : %s\n", SERVER_DIRECTORY);
  force_server_directory_format();

  // buffer to receive packets (max size: 81)
  char string_packet_received[2048];
  // infinite loop -> use ^C to exit the program
  while (1) {
    // GET USER COMMAND
    printf(" -- wait a packet (^C to exit) --\n");

    //GET THE PACKET
    int res = recv_pkt(string_packet_received, channel);

    if (res == CONNECTION_CLOSED) {
      // DO SOMETHING

    }
    else if (res == CANNOT_READ) {
      // DO SOMETHING

    }else if (res == SUCCESS) {
      // PROCESS THE STRING PACKET RECEIVED CONTENT
      Packet * packet_received = empty_packet();
      int error_code_conversion = string_to_packet(string_packet_received, packet_received);
      print_packet(packet_received);

      int error_code_process = process_packet(packet_received, channel);

    }else {
      printf("Error - Not Implemented\n");
    }





    //if (res )
    //  return; // return if communication error occured
    // prints the command, relying on the number of parameters
  
  }

}
