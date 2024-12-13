CC=gcc -Wall
DIR=$(CURDIR)
export

.PHONY: clean test reset

all: dirs ulib/communication.o ulib/functions.o ulib/struct_packet.o ulib/student_client.o ulib/student_server.o ulib/testing.o bin/EDclient/client bin/EDserver/server bin/usrc/testing

dirs: 
	mkdir -p bin
	mkdir -p bin/EDclient
	mkdir -p bin/EDserver
	mkdir -p ulib
	mkdir -p uinclude

	@# in the following lines, $@ denotes the current target

ulib/communication.o: uinclude/communication.h usrc/communication.c
	@echo "Compiling $@"
	$(CC) -c -o $@ usrc/communication.c

ulib/functions.o: uinclude/functions.h usrc/functions.c
	@echo "Compiling $@"
	$(CC) -c -o $@ usrc/functions.c

ulib/struct_packet.o: uinclude/struct_packet.h usrc/struct_packet.c
	@echo "Compiling $@"
	$(CC) -c -o $@ usrc/struct_packet.c

ulib/student_client.o: include/student_client.h usrc/student_client.c
	@echo "Compiling $@"
	$(CC) -c -o $@ usrc/student_client.c

ulib/student_server.o: include/student_server.h usrc/student_server.c
	@echo "Compiling $@"
	$(CC) -c -o $@ usrc/student_server.c

bin/EDclient/client: lib/nettools.o ulib/functions.o lib/utilities.o ulib/student_client.o ulib/communication.o lib/client.o
	@echo "Compiling $@"
	$(CC) -o $@ lib/nettools.o ulib/functions.o lib/utilities.o ulib/student_client.o ulib/communication.o lib/client.o ulib/struct_packet.o
	ln -f $@ bin/

bin/EDserver/server: lib/nettools.o ulib/functions.o ulib/student_server.o ulib/communication.o lib/server.o
	@echo "Compiling $@"
	$(CC) -o $@ lib/nettools.o ulib/functions.o ulib/student_server.o ulib/struct_packet.o ulib/communication.o lib/server.o
	ln -f $@ bin/

ulib/testing.o: usrc/testing.c
	@echo "Compiling $@"
	$(CC) -c -o $@ usrc/testing.c

bin/usrc/testing: ulib/testing.o ulib/functions.o lib/nettools.o ulib/student_server.o ulib/communication.o  ulib/struct_packet.o ulib/student_client.o
	@echo "Compiling $@"
	mkdir -p bin/usrc
	$(CC) -o $@ ulib/testing.o ulib/functions.o lib/nettools.o ulib/student_server.o ulib/communication.o  ulib/struct_packet.o ulib/student_client.o
	ln -f $@ usrc/

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

