# **Report**

<br />

## **Table of Contents**
1. [How to launch the program](#launch)
2. [Implementation choices](#implementation)
    1. [Functions](#functions)
    2. [Packet Structure](#packet-structure)
    3. [Server & Client Directory](#server-&-client-directory)
    4. [Example of communication between client and server](#example)  

<br />

## <a name="launch"></a> **How to launch the program** 

To launch the program, you have to :

1. Clone the git repository by typing this command `git clone https://gitlab.eurecom.fr/della1/basicos2024-team07.git`. If you want to clone it in shh, use this command `git@gitlab.eurecom.fr:della1/basicos2024-team07.git`.

2. Go to the main directory and type `make` in the command prompt to compile the code.

	**(Type `make clean` before `make` if `make` was previously used.)**

3. Launch the server using `./bin/EDserver/server` 
The server should print information on:

		A port: Let's call it p.

		An IP address: Let's call it a.
(Don't forget to provide arguments)

4. Launch the client using  `./bin/EDclient/client` a p [options]

		Options:
		-analyze: Execute commands from a file
		-interactive: Wait for input from the keyboard.
		-directory: Specify a directory for client-side files.

<br />

## <a name="implementation"></a> **Global Approach** 

### <a name="Client Approach"></a> **Client Side**
#### <a name="CmdLine Analysis"></a> **1.Parameter Analysis and Mode Activation**

The program analyzes the command-line parameters by iterating through the arguments and checking for specific options:


If both -analyze and -interactive are provided, the program first processes the file specified by -analyze, then switches to interactive mode.

The program also ensures options are not repeated, and required arguments are provided.

Here’s the relevant block of the code:



```c
for (int i = 3; i < argc; i++) {
	// argv[1] and argv[2] are the ip adress and port number respectvely
	if (strcmp(argv[i], "-analyze") == 0) {
		if (analyze_flag || i + 1 >= argc) {
			// Invalid or missing file
			fprintf(stderr, "Error: Invalid or duplicate -analyze option\n");
			return SYNTAX_ERROR;
		}
		analyze_flag = 1;
		strncpy(analyze_file, argv[++i], sizeof(analyze_file) - 1);
	}
	else if (strcmp(argv[i], "-interactive") == 0) {
		if (interactive_flag) {
			// Duplicate -interactive
			fprintf(stderr, "Error: Duplicate -interactive option\n");
			return SYNTAX_ERROR;
		}
		interactive_flag = 1;
	}
	else if (strcmp(argv[i], "-directory") == 0) {
		// Invalid or duplicate directory
		if (directory[0] || i + 1 >= argc) {
			fprintf(stderr, "Error: Invalid or duplicate -directory option\n");
			return SYNTAX_ERROR;
		}
		set_client_directory(argv[++i]);
		force_client_directory_format(); // Ensure directory ends with '/'
	}
}
```
#### <a name="Analyze Mode"></a> **2.Handling Analyze Mode**

<br />

#### <a name="Interactive Mode"></a> **3.Handling Interactive Mode**

<br />

#### <a name="Conversion and Sending"></a> **4.Converting the command and sending**

<br />

## <a name="implementation"></a> **Implementation choices** 

### <a name="functions"></a> **Functions** 
**Total packet size must not exceed 2048 bytes, including the header.**

**The Limit for the data size is 1978 bytes**

* Use `put filename` to **copy a local file** to the Eurecom Drive. This function takes the name of the file as input and returns either 0 (success) or an error code.

* Use `rm filename` to **remove a remote file** from the Eurecom Drive. This function takes the name of the file as input and retruns either 0 (success) or an error code.

* Use `get filename localfilename` to **copy a remote file** to the local file system. This function takes the name of the file and the the name of the copied file and returns either 0 (success) or an error code.

* Use `ls` to **list all the files** remotely stored in the Eurecom Drive. This function returns the files or an error code.

* Use `cat filename n` to **print the first n lines** of a file. This function takes the name of the file as input and returns the first n lines or an error code.

* Use `mv originfilename destinationfilename` to **rename a remote file**. This function takes the name of the file as input and returns either 0 (success) or an error code.

* Use `quit` or `exit` to **exit**.

* Use `restart` to **reset the connection**.

* Use `help` to have the **description of the available commands**.


<br />

### <a name="packet-structure"></a> **Packet Structure** 

We've implemented a packet structure that simplifies code accessibility. 

```c
// PACKET STRUCTURE
typedef struct  {
	char E;              // 1 byte
	char D;              // 1 byte
	char r;              // 1 byte
	uint16_t data_size;  // 2 byte
	int8_t code;         // 1 byte
	char option1[32];    // 32 bytes
	char option2[32];    // 32 bytes
	char * data_ptr;
} Packet ;
```
Each packet begins with E, D and r so we can recognise the packet.  

Then we implemented methods that use the packet structure.  

The function `string_to_packet(char * string, Packet * packet)` converts a string argument to a struct packet given.

The function `packet_to_string(Packet * packet, char * string)` converts a packet given in a string format.

<br />

### <a name="packet-String-structure"></a> **Packet String Structure** 

 Packet Format (Header + Data) as defined for the project:

	| 'E' | 'D' | 'r' | data size (2 bytes)
	| command/error (1 byte)|
	| option1 (32 bytes) | option2 (32 bytes)
	| data (0 to 1978 bytes)|
 Where:
 - The first three bytes are fixed: 'E', 'D', and 'r'.
 - Data size (2 bytes): Represents the size of the data field in the packet.
 - Command/Error (1 byte): Stores the command type or error code.
 - Option1 (32 bytes): Stores a string (e.g., filename or other metadata).
 - Option2 (32 bytes): Stores a second string (or additional metadata).
 - Data (0 to 1978 bytes): Contains raw data if the command requires it (e.g., file contents). This field is optional.

**Total packet size must not exceed 2048 bytes, including the header.**

<br />

### <a name="server-&-client-directory"></a> **Server & Client Directory** 

Server and client use a global variable to indicate the directory in which they work.

```bash
$ ./bin/EDclient/client <ip-adress> <port> -directory <directory> -interactive -analyse
```
Replace \<ip-adress\> by the IP adress, \<port\> by the port and \<directory\> by the directory.

<br />

### <a name="example"></a> **Example of communication between client and server** 

The client wants to print 3 lines of the file :  

```bash
$ git clone git@gitlab.eurecom.fr:della1/basicos2024-team07.git
$ cd basicos2024-team07.git && make
$ ./bin/EDserver/server [options] #gives us ip adress and port
$ ./bin/EDclient/client 192.168.123.132 443 -directory directory -interactive -analyse file.txt
$ cat MyFile.txt 3
```

The string of the first 3 lines of MyFile.txt is converted into a packet format.

The packet is then converted into a 2048 byte **packet string** starting with E, D, r, then converted into 0's and 1's and sent to the server.

The server converts the 0s and 1s into a “packet string”, then into a packet and analyzes it. It then sends the first 3 lines with the same process back to the client, who will see :  

```bash
First line
Second line
Third line
```  