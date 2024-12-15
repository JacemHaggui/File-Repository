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
#define _DEFAULT_SOURCE // necessary for usage of DT_REG
#include <dirent.h> // For directory handling.
#include <stdlib.h>

#define INT_MAX 1978 // Maximum data_size for packet data.

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



int print_lines(char string[], int n, char outstring[], bool print_state) {
    /*
      Print (or not depending on the parameter print_state) the n first lines of string.
      Stores in outstring the first n lines of string.
      And output the number of packet needed to stores the first n lines of string.
    INPUT :
        string : The string to process
        n : the number of line to get
        outstring : the string in which the  n lines will be written
        print_state : 1 <=> Print the content
    OUTPUT :
        integer : the number of packet needed to stores the first n lines of string.
    */
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

int * file_to_string(char *filename, char ** text){
    /*
      Open a file with its filename, and convert it into the text string given in parameter
      Output an array : [number of caracters, number of lines] based on the file stats.
    INPUT :
        filename : The name of the file to process and to open
        text : the string that will contains the content of the file
    OUTPUT :
        Array : [number of caracters, number of lines] needed
    */
  int * result = malloc(sizeof(int) * 2);

  if(!file_exists(filename)){
    printf("Error: File '%s' does not exist.\n", filename);
    result[0] = FILE_NOT_FOUND;
    result[1] = FILE_NOT_FOUND;
    return result;
  }

  char c;
  int cpt_carac = 0;
  int cpt_line = 0;

  /*Counts size file.*/
  FILE *f = fopen(filename, "r");

  while (fscanf(f, "%c", &c) != EOF)
  {
    cpt_carac = cpt_carac + 1;
  }
  fclose(f);

  /* Stocks file content into a string*/
  *text = malloc(sizeof(char) * cpt_carac);

  (*text)[cpt_carac] = '\0';
  f = fopen(filename, "r");
  cpt_carac = 0;

  while (fscanf(f, "%c", &c) != EOF)
  {
    if (c == EOF) break;
    else if (c == '\n') cpt_line++;
    (*text)[cpt_carac] = c;
    cpt_carac++;
  }
  
  result[0] = cpt_carac; // '\0' included !
  result[1] = cpt_line;
  return result;
}

/* Returns -1 if trying to rename to existing file, returns -2 if trying to rename a non-existing file. Returns 0 if done correctly.*/
int rename_file(char newfile[], char oldfile[]) { // full paths need to be given in parameter.
    /*
      Rename the file at oldfile place with newfile name
    INPUT :
        newfile : The path to the new file
        oldfile : The path to the old file to rename
    OUTPUT : ERROR CODES
      FILE_NOT_FOUND
      FILE_ALREADY_EXISTS
      SUCCESS
    */
  FILE *oldf;

  if(!file_exists(oldfile)){
    printf("ERROR: File %s does not exist on directory!\n", oldfile);
    printf("No modifications will be made.\n");
    return FILE_NOT_FOUND;
  }

  if (file_exists(newfile)) {
    printf("ERROR: File %s already exists on directory!\n", newfile);
    printf("No modifications will be made.\n");
    return FILE_ALREADY_EXISTS;
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
  return SUCCESS;
}




/*Returns multiple packets.*/
Packet ** f_print_n_lines(Packet* input, char *directory){
   /*
    Generate a list of packets that contains the data from a file given in the input packet
    INPUT:
        input    :  The command packet
        directory : The path to the directory where the file should be.
    OUTPUT:
      List of Packets
  */
  
  char * filename = cats(directory, input->option1);

  char * file_string = "";
  int * result = file_to_string(filename, &file_string); // HERE WE DON'T CHECK THE CONTENT OF RESULT EXCEPT FOR ERRORS.
  
  printf("\nThe string to split : \n");
  print_string(file_string, result[0] + 1); // +1 <=> WILL PRINT '\0' at the end

  if(result[0] < 0){
    Packet ** single_slot = calloc(1, sizeof(Packet));
    single_slot[0] = error_packet(result[0]);
    return single_slot;
  }

  int number_lines = atoi(input->option2);
  int number_caracters_to_print = count_caracter_inside_n_first_lines(file_string, number_lines);

  //char * datastring = malloc(sizeof(char) * number_caracters_to_print);
  //int packnum = print_lines(file_string, atoi(input->option2), datastring, 0);

  int packnum = 1;
  if (number_caracters_to_print > INT_MAX) packnum = number_caracters_to_print / INT_MAX + 1;


  Packet ** list = calloc(packnum, sizeof(Packet));
  Packet * out;

  for(int i = 0; i < packnum; i++){
    out = empty_packet();
    char buffer[INT_MAX + 1]; // We have to include the null terminator (1978 of data and 1 octet for the '\0' given that it's a string)
    out->code = 1;
    strcpy(out->option1, itoa(packnum,10));
    out->E = 'E'; out->D = 'D'; out->r = 'r';
    slice(file_string, buffer, i*INT_MAX, (i+1)*INT_MAX);

    
    out-> data_size = (int)strlen(buffer);
    out->data_ptr = calloc(out-> data_size + 1, sizeof(char));
    strcpy(out->data_ptr, buffer);
    //printf("\n\n\n\nLength of Buffer : %d\nThe Content of Buffer : \n%s \n\n\n\n",out-> data_size, out->data_ptr);
    list[i] = out;
  }
  return list;
}

Packet * add_remote_file(Packet* in, char directory[]){ 
  /*
    Generate a file with data content of the packet data field
    INPUT:
        in    :  The command packet
        directory : The path to the directory where the file should be.
    OUTPUT:
      Packet with error code associated to the conversion
  */
  char * filename = cats(directory, in->option1);
  
  if(files_in_folder(directory) + 1 > QUOTANUMBER){
    printf("Error: QUOTA NUMBER (%d) WOULD BE EXCEEDED BY TRANSFER!\n", QUOTANUMBER);
    return error_packet(QUOTA_EXCEEDED);
  }

  if(folder_size(directory) + in->data_size > QUOTASIZE){
    printf("Error: QUOTA SIZE (%d) WOULD BE EXCEEDED BY TRANSFER!\n", QUOTASIZE);
    return error_packet(QUOTA_EXCEEDED);
  }

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
  /*
    Rename a file based on the input packet
    INPUT:
        in    :  The command packet
        directory : The path to the directory where the file should be renamed.
    OUTPUT:
      Packet with error code associated to the conversion
  */
  char * oldfilename = cats(directory, in->option1);
  char * newfilename = cats(directory, in->option2);

  int errcode = rename_file(newfilename, oldfilename);

  return error_packet(errcode);
}

int remove_file(char filename[]) {
  /*
    Remove a file based on the input packet
    INPUT:
        filename : the name of the file.
    OUTPUT:
      Packet with error code associated to the removal
  */
  if(!remove(filename)){
    return FILE_NOT_FOUND;
  }
  else{
    return SUCCESS;
  }
}

Packet * removefile(Packet* in, char directory[]){
    /*
    Remove file Handler
    INPUT:
        in    :  The command packet
        directory : The path to the directory where the file should be there.
    OUTPUT:
      Packet with error code associated to the removal
  */
  int errcode = remove_file(cats(directory, in->option1));
  
  return error_packet(errcode);
}

int files_in_folder(char* directory){
  /*
    Returns number of files present in directory.
  */

  int file_count = 0;
  DIR * dr =opendir(directory);
  struct dirent * de;

  while ((de = readdir(dr)) != NULL) {
    if (strcmp(de->d_name , ".") == 0 || strcmp(de->d_name , "..") == 0) continue;
    file_count++;
  }
  closedir(dr);
}

int folder_size(char* directory){  
  /*
    Returns sum of size of files in directory
  */

  int size_count = 0;
  DIR * dr =opendir(directory);
  struct dirent * de;

  while ((de = readdir(dr)) != NULL) {
    if (strcmp(de->d_name , ".") == 0 || strcmp(de->d_name , "..") == 0) continue;
    struct stat buf;
    stat(de->d_name, &buf);
    size_count += buf.st_size;
  }
  closedir(dr);
}

Packet **fetch(Packet* in, char directory[]){
  /*
    Get the data inside of a file given by the packet in parameter
    INPUT:
        in    :  The command packet
        directory : The path to the directory where the file should be there.
    OUTPUT:
      List with the data Packet from the file
  */
  char * filename = cats(directory, in->option1);

  char * contents = "" ;
  
  int * result = file_to_string(filename, &contents);
  int number_caracter = result[0];

  if (number_caracter < 0) { // ERROR !
    Packet ** single_slot = calloc(1, sizeof(Packet));
    single_slot[0] = error_packet(result[0]);
    return single_slot;
  }

  int packnum = 1;
  if (number_caracter > INT_MAX ) packnum = number_caracter / INT_MAX + 1;

  Packet ** list = calloc(packnum, sizeof(Packet));
  
  for (int i = 0; i < packnum; i++){
    Packet * out = empty_packet();
    char buffer[INT_MAX + 1];
    out->code = 5;
    strcpy(out->option1, itoa(packnum,10)); // FOR NOW BUT WILL CHANGE WHEN WE SEND THE PACKET
    strcpy(out->option2, itoa(number_caracter,10)); // STAY LIKE THAT
    out->E = 'E'; out->D = 'D'; out->r = 'r';
    slice(contents, buffer, i*INT_MAX, (i+1)*INT_MAX);
    out-> data_size = strlen(buffer);
    out->data_ptr = calloc(out-> data_size + 1, sizeof(char));
    strcpy(out->data_ptr, buffer);
    list[i] = out;
  }
  return list;
}

Packet **list_files(Packet* in, char destination[]){
  /*
    List the files inside of a given destination
    INPUT:
        in    :  The command packet
        directory : The path to the directory where the files should be.
    OUTPUT:
      List with the Packet containing the names and size of the files
  */
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

  char * string = "";
  while ((de = readdir(dr)) != NULL) {
    if (strcmp(de->d_name , ".") == 0 || strcmp(de->d_name , "..") == 0) continue;

    string = cats(string, de->d_name);
    string = cats(string, ",");
    struct stat buf ;
    stat(de->d_name, &buf);
    string = cats(string, itoa(buf.st_size, 10));
    string = cats(string, ",");
  }
  closedir(dr);

  int number_caracter = strlen(string);
  string[number_caracter - 1] = '\0'; // o remove the last comma and add a null terminator at the end.

  // SORT THE STRING :
  if(!sort_dir(string)) {
    fprintf(stderr," Error while sorting\n\n");
    Packet ** list = malloc(sizeof(Packet));
    list[0] = error_packet(COMMAND_FAILS);
    return list;
  }

  int packnum = 1;
  if (number_caracter > packnum) packnum = number_caracter / INT_MAX + 1;

  Packet **list = calloc(packnum, sizeof(Packet));

  for(int i = 0; i < packnum; i++){
    Packet * out = empty_packet();
    char *buffer = malloc(INT_MAX * sizeof(char));
    out->code = 6;
    strcpy(out->option1, itoa(packnum, 10));
    //slice(datastring, buffer, i*INT_MAX, (i+1)*INT_MAX);
    slice(string, buffer, i*INT_MAX, (i+1)*INT_MAX);
    out-> data_size = strlen(buffer);
    out->data_ptr = calloc(out-> data_size + 1, sizeof(char));
    strcpy(out->data_ptr, buffer);
    list[i] = out;
    }
    return list;
}
// Copy a remote file to the local filesystem (WIP)

int process_packet(Packet * packet, int channel) {
  /* 
		Process the packet received in parameter, launch the function dedicated to it
	INPUT :
    packet: packet to be processed
    channel : in which the answer packets will be sent
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
    printf("\n\n--- SENDING PACKETS: ---\n\n");
    for (int i = 0; i < number_packet_to_send; i ++) {
        char packet_string_to_send[MAX_PACKET_SIZE];

        int error_code = packet_to_string(list_packet_to_send[i], packet_string_to_send);
        int res = send_pkt(packet_string_to_send, channel);

        printf("--[THE FOLLOWING PACKET IS SENT :]--\n\n");
        printf("\tPacket %d Send: \n\t\tError Code : %d\n\t\tSend Code: %d\n\n\t", i+1, error_code, res);
        print_packet(list_packet_to_send[i]);
        printf("\nNumber of Data to Print : (70 + data_size) %d\n", list_packet_to_send[i]->data_size + 70);
        print_string(packet_string_to_send, list_packet_to_send[i]->data_size + 70);
        printf("\n--[END OF THE PACKET SENT :]--\n");
    }
    printf("\n\n--- END SENDING PACKETS: ---\n\n");
  }

  // ADDING A REMOTE FILE
  else if (packet->code == CMD_ADD) {
    //ADD THE REMOTE FILE USING THE PACKET DATA
    Packet * packet_error_code = add_remote_file(packet, SERVER_DIRECTORY);
    
    // SEND PACKET WITH THE ERROR CODE ASSOCIATED TO THE CREATION OF THE REMOTE FILE
    char packet_string_to_send[MAX_PACKET_SIZE];
    int error_code = packet_to_string(packet_error_code, packet_string_to_send);
    int res = send_pkt(packet_string_to_send, channel);
    printf("Packet Send : \tError Code : %d\tSend Code: %d\n",  error_code, res);
  }


  // RENAMING A REMOTE FILE
  else if (packet->code == CMD_RENAME) {
    // RENAME THE REMOTE FILE
    Packet * packet_error_code = renamefile(packet, SERVER_DIRECTORY);

    // SEND PACKET WITH THE ERROR CODE ASSOCIATED TO THE RENAMING OF THE REMOTE FILE
    char packet_string_to_send[MAX_PACKET_SIZE];
    int error_code = packet_to_string(packet_error_code, packet_string_to_send);
    int res = send_pkt(packet_string_to_send, channel);
    printf("Packet Send : \tError Code : %d\tSend Code: %d\n",  error_code, res);
  }


  // REMOVING A REMOTE FILE
  else if (packet->code == CMD_REMOVE) {
    // REMOVING THE REMOTE FILE
    Packet * packet_error_code = removefile(packet, SERVER_DIRECTORY);

    // SEND PACKET WITH THE ERROR CODE ASSOCIATED TO THE REMOVAL OF THE REMOTE FILE
    char packet_string_to_send[MAX_PACKET_SIZE];
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
      number_packet_to_send = atoi(first_packet->option1); //OPTION 1 contains the number of packets
    }

    // SEND THE PACKETS
    for (int i = 0; i < number_packet_to_send; i ++) {
        char packet_string_to_send[MAX_PACKET_SIZE];
        if (error_code >= 0) strcpy(list_packet_to_send[i]->option1, packet->option1);
        int error_code = packet_to_string(list_packet_to_send[i], packet_string_to_send);
        int res = send_pkt(packet_string_to_send, channel);
        printf("Packet %d : \tError Code : %d\tSend Code: %d\n", i, error_code, res);

        printf("THE FOLLOWING PACKET IS SENT :\n");
        print_packet(list_packet_to_send[i]);
        print_string(packet_string_to_send, list_packet_to_send[i]->data_size + 70);
        printf("END OF THE PACKET SENT :\n");
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
        char packet_string_to_send[MAX_PACKET_SIZE];
        int error_code = packet_to_string(list_packet_to_send[i], packet_string_to_send);
        int res = send_pkt(packet_string_to_send, channel);
        printf("Packet %d : \tError Code : %d\tSend Code: %d\n", i, error_code, res);
        print_packet(list_packet_to_send[i]);
        print_string(list_packet_to_send[i]->data_ptr , list_packet_to_send[i]->data_size);
    }

  }
  else {
    // CREATE ERROR PACKET
    Packet * packet_error_code = error_packet(SYNTAX_ERROR);
    // SEND PACKET WITH THE ERROR CODE ASSOCIATED TO THE REMOVAL OF THE REMOTE FILE
    char packet_string_to_send[MAX_PACKET_SIZE];
    int error_code = packet_to_string(packet_error_code, packet_string_to_send);
    int res = send_pkt(packet_string_to_send, channel);
    printf("Packet Send : \tError Code : %d\tSend Code: %d\n",  error_code, res);
  }

  return SUCCESS;

}


void student_server(int channel, int argc, char *argv[]) {
  // Writing to a closed socket causes a SIGPIPE signal, which makes
  // the program exits. The following line prevents this default behaviour.
  // Thus, writing to a closed socket makes the program simply return -1, put
  // the EPIPE in errno: this avoids the program to directly exit. (cf the line
  // with "EPIPE" in send_pkt in usrc/communication.c).
  signal(SIGPIPE, SIG_IGN);

  char directory[256] = {0};  // Directory MUST ENDS WITH '/'

      // Step 1: Parse command-line arguments, figuring out which mode to activate
  for (int i = 1; i < argc; i++) {

    if (strcmp(argv[i], "-quotasize") == 0) {
        if ( i+1 > argc || atoi(argv[i+1]) <= 0) { // IF THERE IS NO QUOTA SIZE OR IT'S NOT A NUMBER (OR THE NUMBER IS 0).
            fprintf(stderr, "Error: Invalid or duplicate -quotasize option\n");  // In case the user is messing with us
            return ; // EXIT
        }
        set_quota_size(argv[i+1]);
    }
    else if (strcmp(argv[i], "-quotanumber") == 0) {
        if (i+1 > argc || atoi(argv[i+1] <= 0)) { // TO DO
            fprintf(stderr, "Error: Invalid or -interactive option\n");
            return ; // EXIT
        }
        set_quota_number(argv[i+1]); 
    }

    else if (strcmp(argv[i], "-directory") == 0) {
        if (directory[0] || i + 1 >= argc) { // BY DEFAULT directory is './'
            fprintf(stderr, "Error: Invalid or duplicate -directory option\n");
            return ; // EXIT
        }
        
        set_server_directory(argv[++i]); // DEFINE the new directory to work with
        force_server_directory_format(); // ADD '/' at the end of directory given.
        printf("\nCURRENT DIRECTORY : %s\n", SERVER_DIRECTORY);
    }
    else {
        fprintf(stderr, "Error: Unknown option %s\n", argv[i]);  // User isn't making any sense
        return ; // EXIT
    }
  }


  // buffer to receive packets (max size: 81)
  char string_packet_received[2048];
  // infinite loop -> use ^C to exit the program
  while (1) {
    // GET USER COMMAND
    printf(" -- wait a packet (^C to exit) --\n");

    //GET THE PACKET
    int res = recv_pkt(string_packet_received, channel);

    if (res == CONNECTION_CLOSED) {
      printf("--[Connection Closed]--\n\n");
      return ;

    }
    else if (res == CANNOT_READ) {
      // CREATE ERROR PACKET
      Packet * packet_error_code = error_packet(CANNOT_READ);
      // SEND PACKET WITH THE ERROR CODE ASSOCIATED TO THE REMOVAL OF THE REMOTE FILE
      char packet_string_to_send[MAX_PACKET_SIZE];
      int error_code = packet_to_string(packet_error_code, packet_string_to_send);
      int res = send_pkt(packet_string_to_send, channel);
      printf("Packet Send : \tError Code : %d\tSend Code: %d\n",  error_code, res);
      break;

    }else if (res == SUCCESS) {
      // PROCESS THE STRING PACKET RECEIVED CONTENT
      Packet * packet_received = empty_packet();
      int error_code_conversion = string_to_packet(string_packet_received, packet_received);
      print_packet(packet_received);

      switch (error_code_conversion) {
        case BAD_PACKET_FORMAT:
        case QUOTA_EXCEEDED: {
          // CREATE ERROR PACKET
          Packet * packet_error_code = error_packet(error_code_conversion);
          // SEND PACKET WITH THE ERROR CODE ASSOCIATED TO THE REMOVAL OF THE REMOTE FILE
          char packet_string_to_send[MAX_PACKET_SIZE];
          int error_code = packet_to_string(packet_error_code, packet_string_to_send);
          int res = send_pkt(packet_string_to_send, channel);
          printf("Packet Send : \tError Code : %d\tSend Code: %d\n",  error_code, res);
          break;
        }
        default : {
          int error_code_process = process_packet(packet_received, channel);
        }
      }

      

    }else {
      printf("Error - Not Implemented\n");
    }





    //if (res )
    //  return; // return if communication error occured
    // prints the command, relying on the number of parameters
  
  }

}
