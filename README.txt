README Documentation for FALL 2022 EE450 PROJECT
=====================================

+--------------------------+
| a/b. STUDENT INFORMATION |
+--------------------------+

a. Your Full Name as given in the class list
b. Your Student ID

NAME: Aaishah Kelani
USC ID: 3981517964
DUE DATE: 21NOV2022

+--------------------------------------+
| c. COMPLETED PARTS OF THE ASSIGNMENT |
+--------------------------------------+

c. What you have done in the assignment, if you have completed the optional part
(suffix). If it’s not mentioned, it will not be considered.

The complete project requriements as specified by the project description have been 
implemented. The program implements the authentication of the client and any subsequent
query requests via the main server to the backend server.

The extra credit has been implemented in my program files. The client can input multiple
course codes to receive the printout of the information for all course codes found.

+------------------+
| d. PROGRAM FILES |
+------------------+

d. What your code files are and what each one of them does. (Please do not repeat the
project description, just name your code files and briefly mention what they do).

Required Files:
- Makefile
    - makefile used to complile the program and build the program executables

- client.cpp
    - The client program interacts with the main serverM to send and recieve 
    authentication and query requests/results.

- serverM.cpp
    - serverM is a central web registration server, which forwards the authentication
    requests of the client to the credential server and the query requests of the 
    client to the backend department servers. 

- serverC.cpp
    - serverC is a credential server, which processes and returns a response to the
    authentication requests intiated from the client and sent to serverC via the main 
    central server.

- serverCS.cpp
    - a computer science department databse server, which replies to and fulfills query 
    requests from the client sent by the main central registration server.

- serverEE.cpp
    - an electrical engineering department databse server, which replies to and 
    fulfills query requests from the client sent by the main central registration server.

Other Files:
- globals.h 
    - This file is a header file that contains global variable information including the
    the hardcoded static port numbers, the hardcoded localhost IP address and the data 
    struct definition used in the back end servers. Maximum limits on the the sizes of 
    buffers used in sending and recieved are defined. The hardcorded encoding used between 
    the middle and backend servers are used here.

+---------------------------------+
| e. FORMAT OF MESSAGES EXCHANGED |
+---------------------------------+

e. The format of all the messages exchanged.

From the client and serverM:
- The username and password were sent as follows: "username,password"
- Single query requests were sent as follows: "x,course_code,category"
	- where "x" = 1, to indicate this is a single query, not a multiple
- Multiple query requests were sent as follows: "x,course_code, course_code..."
	- where "x" > 1 to indicate the message contains multiple queries

From serverM to serverC:
- The username and password were sent as follows: "username,password"

From serverC to serverM
- "0" indicated the username did not exist, "1" indicated the username existed
   but the password did not match, "2" inidcated success

From serverM to the client:
- For authentication, "0" indicated the username did not exist, "1" indicated 
  the username existed but the password did not match, "2" inidcated success 
- For querying, the general format was: "x,y,course_code,category,information"
  for single queries and "x,y,information$x,y,information" for multiple queries
    - "x" = 0 represented single queries, "x" = 1 represented multiple queries
    - "y" = 0 meant the information was not found, "y" = 1 meant the information
       was found
    - $ was used as a delimeter to separate multiple incoming query results
      after having sent multiple queries in a row

From serverM to the department servers (serverEE and serverCS):
- For single queries, the general format is: "course_code,category"
- For multiple queries, the general format is: "course_code"

From department servers (serverEE and serverCS) to serverM:
- For querying, the general format was: "x,y,course_code,category,information"
  for single queries and "x,y,information$x,y,information" for multiple queries
    - "x" = 0 represented single queries, "x" = 1 represented multiple queries
    - "y" = 0 meant the information was not found, "y" = 1 meant the information
       was found
    - $ was used as a delimeter to separate multiple incoming query results
      after having sent multiple queries in a row
    - information represented the found data; if the information was not found,
      this was ommitted


+--------------------------+
| g. BUGS / TESTS TO SKIP  |
+--------------------------+

g. Any idiosyncrasy of your project. It should say under what conditions the project
fails, if any.

There are no known bugs in my code of any idiosyncrasies. Everything should work as intended.

+----------------------------+
| h. REUSED CODE (CITATIONS) |
+----------------------------+

h. Reused Code: Did you use code from anywhere for your project? If not, say so. If so,
say what functions and where they're from. (Also identify this with a comment in the
source code.)

Code from Beej's Networking Tutorial is used within the following files:
    - serverM.cpp
        - Functions: get_in_addr, sigchld_handler, create_tcp_client_socket, 
        talk_to_serverEE, talk_to_serverCS, talk_to_serverC, create_udp_server_socket,
        create_tcp_server_socket, authenticate_client, query_driver
    - client.cpp
        - Functions: get_in_addr, create_tcp_client_socket, authenticate_client
    - serverC.cpp
        - Functions: get_in_addr, create_udp_server_socket
    - serverEE.cpp
        - Functions: get_in_addr, create_udp_server_socket
    - serverCS.cpp
        - Functions: get_in_addr, create_udp_server_socket


Link to Beej's Networking Tutorial:
https://beej.us/guide/bgnet/html/

+-----------------------------------------------+
| OTHER (Optional) - Not considered for grading |
+-----------------------------------------------+

