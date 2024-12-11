#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "../uinclude/struct_packet.h"
#include "../include/student_server.h"

int main(int argc, char *argv[]) {
  Packet* pac1 = empty_packet();

  strcpy(pac1->option1, "hello.txt");

  strcpy(pac1->option2, "3");

  Packet** out = f_print_n_lines(pac1, "./");

  printf("\n == == \n");

  print_packet(out[0]);

  return 0;
}
