CC=gcc -Wall
DIR=$(CURDIR)
export

.PHONY: clean test reset

all: dirs bin/EDclient/client bin/EDserver/server

dirs: 
	mkdir -p bin
	mkdir -p bin/EDclient
	mkdir -p bin/EDserver
	mkdir -p ulib
	mkdir -p uinclude

	@# in the following lines, $@ denotes the current target

ulib/communication.o: uinclude/communication.h usrc/communication.c
	$(CC) -c -o $@ usrc/communication.c

ulib/student_client.o: include/student_client.h usrc/student_client.c
	$(CC) -c -o $@ usrc/student_client.c

ulib/student_server.o: include/student_server.h usrc/student_server.c
	$(CC) -c -o $@ usrc/student_server.c

bin/EDclient/client: lib/nettools.o lib/utilities.o ulib/student_client.o ulib/communication.o lib/client.o
	$(CC) -o $@ lib/nettools.o lib/utilities.o ulib/student_client.o ulib/communication.o lib/client.o
	ln -f $@ bin/

bin/EDserver/server: lib/nettools.o ulib/student_server.o ulib/communication.o lib/server.o
	$(CC) -o $@ lib/nettools.o ulib/student_server.o ulib/communication.o lib/server.o
	ln -f $@ bin/

clean:
	if [ -d ./tests ]; then \
		$(MAKE) clean -C tests/ ; \
	fi
	rm -f bin/EDserver/server bin/EDclient/client ulib/*.o bin/server bin/client

reset:
	if [ -f ./bin/EDserver/server ]; then \
		cp bin/EDserver/server reset/EDserver/server; \
	fi
	if [ -f ./bin/EDclient/client ]; then \
		cp bin/EDclient/client reset/EDclient/client; \
	fi
	rm -f bin/EDclient/* bin/EDserver/*
	cp reset/EDserver/* bin/EDserver/
	cp reset/EDclient/* bin/EDclient/
	rm -f reset/EDserver/server reset/EDclient/client

test: all
	$(MAKE) test -C tests/

