# Introduction

The executables server and client in this directory are an implementation
of the project which respects the web page specification.
All required options are implemented.

As they are intended to be used for debugging or testing the peer side, 
the server and the client offer some more features w.r.t the project specification.

# client commandline

print help : ./client or ./client -h

otherwise, commandline : ./client IPaddress Port options

Following options must not be inserted before IPaddress Port.
IPaddress can be a machine name if this machine is known by DNS.

options are:

- -interactive :
	interactive mode ("better" interface messages)
	
- -analyze filename :
	reads commands from filename.
	If -interactive is present, it then switch to interactive mode.
	Otherwise it exits.
- -p :
	prints sent and received packets headers (useful for server debug)

- -directory string :
	   specifies the directory to be used to store files. If this
	   directory is non empty when the client starts, then existing
	   files are assumed to be part of the local drive.

- - seed n :
       produces a buggy client. Some packets are randomly corrupted.
       messages on stderr details the corruption.
       n is the seed to initialize random.

Default behaviour is interactive but with the printing of analyze mode,
no packets, no bug.
Both modes send quite the same messages on stderr.


# additionnal commands (to use in interactive mode)

- interactive :
	switch to interactive mode printing
- analyze :
	switch to analyze mode printing
- help :
	prints help message
- pkt :
	toggle on/off the printing of packets
- quit :
	quits
- restart :
	reset connection



# server commandline: 


print help : ./server or ./server -h

otherwise, commandline : ./server options

options are :

- -quotasize int :
	maximum number of bytes that the remote directory can store.

- -quotanumber int :
	maximum number of files that the remote directory can store.

- -directory string :
	specifies the directory to be used to store files. If this
	directory is non empty when the server starts, then existing
	files are assumed to be part of the remote drive.

- -p :
	prints packets (useful for client debug)

- -seed n :
	produces a buggy server. Some packets are randomly corrupted.
	messages on stderr details the corruption.
	n is the seed to initialize random.
