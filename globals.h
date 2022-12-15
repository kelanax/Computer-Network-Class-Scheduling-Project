/*
 * Author: Aaishah Kelani (aaishahk@usc.edu)
 * Title: eeproject
 */

#ifndef GLOBALS
#define GLOBALS

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>


//int client_socket, main_socket, ee_socket, cs_socket;
#define C_UDP_PORT "21964"
#define CS_UDP_PORT "22964"
#define EE_UDP_PORT "23964"
#define MAIN_UDP_PORT "24964" 
#define MAIN_TCP_PORT "25964"  // the TCP port the client will be connecting to
#define HOST_NAME_MAX_ 65 
#define INET6_ADDRSTRLEN_ 46

#define BACKLOG 10	 // how many pending connections queue will hold
#define MAXDATASIZE 1024 // max number of bytes we can get at once 

#define TRUE 1
#define FALSE 0

#define FAIL_NO_USER 0 // username is not in cred database
#define FAIL_PASS_NO_MATCH 1 // pass word does not match the user
#define PASS 2 // user passes authentication

#define SUCCESS 1 // a query inqury was succesful
#define FAIL 0 // a query inquiry was not successful

#define SINGLE 0 // a single query was sent to the department servers
#define MULTIPLE 1 // multipe queries were sent to the department servers

#define CRED 1 // encoding for the CRED server
#define EE 2 // encoding for the EE server
#define CS 3 //  encoding for the CS server
#define localhost "127.0.0.1" // 

// data type to hold server record information
typedef struct record_struct
{
    std::string course_code;
    std::string credit;
    std::string professor;
    std::string days;
    std::string course_name;
    //course code, credit, professor, days and course name
    // ex: EE450,4,Ali Zahid,Tue;Thu,Introduction to Computer Networks
} Record;

// data type to hold server credential information
typedef struct cred_struct
{
    std::string user_name;
    std::string password;
} cred;




#endif