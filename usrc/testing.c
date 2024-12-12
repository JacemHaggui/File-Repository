#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "../uinclude/struct_packet.h"
#include "../include/student_server.h"

int main(int argc, char *argv[]) {
  
  // TESTING: PRINTING THE FIRST THREE LINES OF FILE CALLED "testext.TXT", located in "usrc" but not included in repo.
  Packet* pac1 = empty_packet();
  strcpy(pac1->option1, "testext.txt");
  strcpy(pac1->option2, "3");
  Packet** out = f_print_n_lines(pac1, "./");
  printf("\n == == \n");
  print_packet(*out);

  // TESTING: add remote file called "newfile.txt" containing "abcdefg"

  Packet *pac2 = empty_packet();

  char data[8] = "abcdefg";

  strcpy(pac2->option1, "newfile.txt");
  strcpy(pac2->option2, itoa(strlen(data), 10));
  pac2->data_ptr = data;

  Packet* out2 = add_remote_file(pac2, "./");

  printf("\n == PACKET 1 == \n");
  print_packet(out2);

  // TESTING: What if we try to do it a second time?

  printf("\n == PACKET 2 == \n");

  out2 = add_remote_file(pac2, "./");

  // TESTING: renaming "newfile.txt" to "newerfile.txt"

  Packet *pac3 = empty_packet();

  strcpy(pac3->option1, "newfile.txt");
  strcpy(pac3->option2, "newerfile.txt");

  Packet *out3 = renamefile(pac3, "./");

  printf("\n == PACKET 3 == \n");
  print_packet(out3);

  // TESTING: finally, we remove "newerfile.txt".

  Packet *pac4 = empty_packet();

  strcpy(pac4->option1, "newerfile.txt");

  Packet *out4 = removefile(pac4, "./");

  printf("\n == PACKET 4 == \n");
  print_packet(out4);

  return 0;
}