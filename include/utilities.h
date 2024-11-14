#ifndef UTILITIES_H
#define UTILITIES_H

struct usercmd {
    char cmd[16]; // expects a null-terminated string
    char param1[32]; // expects a null-terminated string
    char param2[32]; // expects a null-terminated string
};

typedef  struct usercmd usercommand;


/** fills the ucmd parameter structure by parsing the user command line in str.
 * returns 0 if the command line is not recognized (in this case, an error message 
 * has been sent on stderr except for empty commands) and ucmd is undefined. 
 * Otherwise returns a non-zero value and ucmd is up-to-date.
 * Parameters of ucmd that are not relevant w.r.t the commandline command
 * are left unchanged.
 * 
 * @param ucmd the usercommand structure to fill
 * @param str the command line to parse
 * @return 1 if success, otherwise 0. 
 * 
 * Command line str must be a null terminated string
*/
int parse_commandline(usercommand *ucmd, char *str);

/* this function provides a string that allow to observe packet headers
 * content in a human readable way. Each time the function is called
 * the result of the previous call is erased.  The input parameter pkt
 * must be an array whose length is at least the size of packet's headers.
 * The function returns the "readable" string.
 * Warning: this string is erased by the next call of the function.
 */
char * const pkt_string(char *pkt);

/* sort the parameter string w.r.t alphabetical order. The parameter lsstring
 * must respect the syntax specified for the data part of packets returned by
 * the server responding to a "ls" command. Moreover, it must be null 
 * terminated.
 * Returns 0 in case of error, otherwise returns a non zero value.
*/
int sort_dir(char *lsstring);

#endif
