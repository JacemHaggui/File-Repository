#ifndef STUDENT_SERVER_H
#define STUDENT_SERVER_H

#include "../uinclude/struct_packet.h"

extern const char * const server_help_options;

void student_server(int channel, int argc, char *argv[]);

char* itoa(int value, int base);

// better concatenation
char* cats(char* dest, char* source);

Packet **f_print_n_lines(Packet* in, char directory[]);

Packet *add_remote_file(Packet* in, char directory[]);

Packet * renamefile(Packet* in, char directory[]);

Packet * removefile(Packet* in, char directory[]);

Packet **fetch(Packet* in, char directory[]);

Packet **list_files(Packet* in, char destination[]);


#endif

