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

2. Go to usrc directory using `cd usrc && make` in the command prompt.  

3. The server launches the program using `./bin/EDserver/server` and returns a port and an IP adress.

4. Th client launches the program using  `./bin/EDclient/client` with the IP adress and the port given by the server. 


Then you can type all the functions that follow.


<br />

## <a name="implementation"></a> **Implementation choices** 

### <a name="functions"></a> **Functions** 

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
$ cd usrc && make
$ ./bin/EDclient/client 192.168.123.132 443 -directory directory -interactive -analyse file.txt
$ cat MyFile.txt 3
```

The string of the first 3 lines of MyFile.txt is converted into a packet format. The packet is then converted into a 32-bit “packet string” starting with E, D, r, then converted into 0's and 1's and sent to the server.

The server converts the 0s and 1s into a “packet string”, then into a packet and analyzes it. It then sends the first 3 lines with the same process back to the client, who will see :  

```bash
First line
Second line
Third line
```  