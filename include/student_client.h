#ifndef STUDENT_CLIENT_H
#define STUDENT_CLIENT_H

extern const char * const client_help_options;

void print_ls_format(const char *stri, int n);


int student_client(int channel, int argc, char *argv[]);

#endif

