# Quick overview
This file provides a quick summary on how files are organized, 
and where are the various documents to be read to start on the project.

The first document to read is here:
https://perso.telecom-paristech.fr/pacalet/BasicOS/project.html
If provides the main specification of the project.

Additionally, this repository contains two documents you ought to read:
- doc/ProjectSpecification.pdf: additional specifications on the project
- doc/Cstructuring.pdf: how to split a C project in different files.

**Important note:** to complete this project, you will have to modify
- student_client.c and student_server.c (provided files are examples)
- what you want in usrc and uinclude (provided files are examples)
- the Makefile in the root directory (in particular if you add libraries)
- nothing in include, lib, reset,  tests,  tools (provided files to be used as they are)


# Files
Here is a summary of the sub-directories:
- Main dir: contains the main Makefile and this README
- bin/: generated executable files
- doc/: documents to be used to do the project
- include/: contains the provided .h files
- lib/: software libraries. to be used to simplify your coding
- reset/: see below the "reset" target of the Makefile
- tests/ contain a list of useful files to run tests. See below.
- tools/ contain our client and server, in binary format. You can use them to test your own client or your own server
- usrc/, uinclude/ are the C and h sources of an example of packet exchange. YOUR CODE should go there.
- ulib/ contains the library obtained when compiling the content of usrc/

Note: although student_client.h and student_server.h could be in
uinclude (as you have to implement these libraries), they are in
include as you must not modify these files.

# Expected work: A quick summary

IMPLEMENT:

1) implement function student_server in usrc/student_server.c
   (ideally, this function should always return
   and should never exit, except if the command line is buggy)

2) implement function student_client in usrc/student_client.c
   (ideally, this function should always return
   and never exit, except if the command line is buggy)
   - return 0 makes client exit
   - return non zero makes client restart


# Project compilation and execution
Once you have completed the above mentioned functions, 
do as follows to compile your code:

3) $ make

Then, to run your code:

4) type "./server" in bin/EDserver directory
   (Don't forget to provide arguments)
   => the server should print information on: 
   - a port. Let's call it p.
   - an IP address. Let's call it a.

5) type "./client a p" in bin/EDclient directory
   (potentially with available options)
   where a and p are the address and port obtained 
   at point 4 above. You can also use the name of the
   server machine as address.


# Function student_server in student_server.c
The first parameter "channel" is a file descriptor 
(an already connected TCP/IP socket) from which you can read 
and write to communicate with the client.

A small example code is provided using "read" and
"write" syscalls.
You can also use "send" and "recv" (more complex)

Parameters argv and argc are the ones given as
input to the main function of "server". 
Thus they contain the parameters given as argument to 
the server (point 4 above). If we wish, you can add new 
optional arguments to the server.
    "./server optional student parameters"


# Function student_client in student_client.c

The first parameter "channel" is a file descriptor 
(a connected TCP/IP socket) on which you can read 
and write to communicate with the server.

A small example code is provided using "read" and
"write" syscalls.
You can also use "send" and "recv" (more complex)

Parameters argv and argc are the ones given as input to the main 
of the "client". Thus they contain the 
parameters given as argument  to the client (point 5 
above). If we wish, you can add new 
optional arguments to the server.
    "./client a p optional student parameters"


# Provided functions

A small library is provided, which contains three
functions that may be useful for your implementation:

- parse_commandline: you can use it to parse the
  command lines you get from the user at client side.

- pkt_string: you can use it to obtain a human readable
  version of the content of a packet header, which can
  be useful while debugging (useless in the final code).

- sort_dir: you can use it to sort the string describing
  the directory content (server response to ls) w.r.t.
  alphabetical order.


These functions are in the "utilities" library
(include/utilities.h and lib/utilities.o)


# Makefile 

## Main targets


As usual:

- "make all" builds the whole project, i.e., the client
   and server binaries in bin/EDclient and bin/EDserver.
   These binaries are also available in bin/ as links

- "make clean" removes all the files that have been
  created by the build

Moreover,

- "make bin/EDclient/client" only builds the client

- "make bin/EDserver/server" only builds the server


## "test" target

"make test" runs a set of tests and prints OK or KO results on output.
Most commands can be evaluated using these tests.
However, some of them cannot be fully automatized as the expected result is
not totally specified: they require a manual check.

To go further:

- The expected results of the tests are in tests/golden
- The generated results obtained by running the tests are in tests/result
- The results to be manually checked are generated in tests/to_check

The file tests/TESTS_DESCRIPTION.txt provide more information on tests.


## "reset" target

"make reset" re-initializes the content of bin/EDclient and bin/EDserver
with the content of reset/EDclient and reset/EDserver.
Warning: all files that are not in reset/EDclient and reset/EDserver
are removed from bin/EDclient and bin/EDserver, except the client and server
executable files. This target is useful to easily restore the initial state.
reset/EDclient and reset/EDserver directories contain the files to be restored
with "reset". If you modify any file in this directory, restore its initial state before running the provided tests (see "test" target).
