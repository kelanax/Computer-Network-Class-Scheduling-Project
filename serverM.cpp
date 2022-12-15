#include "globals.h"
using namespace std;


/******************************************************************************/
/*

serverM is a central web registration server, which forwards the authentication
requests of the client to the credential server and the query requests of the 
client to the backend department servers. 

To run:
make all
./serverM


NOTE: Link to Beej's Networking Tutorial which is used in this program:
https://beej.us/guide/bgnet/html/

*/                                                                                                                
/******************************************************************************/


/*------------------------------------ GLOBAL VARIABLES ---------------------------------------*/

// sockfd is the TCP socket of the serverM server that clients can connect to
// child_fd is the child socket of a connected client
// sockfd_udp is the UDP socket of serverM
int debug = FALSE, sockfd, child_fd, sockfd_udp;
int authenticate_done = FALSE; // set to TRUE when the authentication of a client is done
string curr_user = ""; // stores the username of the currently connected client

//struct sockaddr_in main_sockaddr_in;
//struct addrinfo main_udp_server_info; // struct to hold the udp address info of serverM


/*---------------------------------------- UTILITIES -------------------------------------------*/

/*
	Reaps all dead processes
	CITATION: This function is from Beej's Networking Tutorial

	@param	s	quiet unused variable warning
*/
void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

/*
	Converts an int into a string

	@param		val		integer to be converted to a string
	@return		string of converted integer
*/
string int_to_str(int val)
{
	ostringstream oss;
    oss << val;
    string new_str = oss.str();
	return new_str;
}

/*
	Converts a string into an int

	@param		str_val		integer to be converted to a string
	@return		integer of converted string
*/
int str_to_int(string str_val)
{
	return atoi(str_val.c_str());
}

/*
	Encrypts a given word

	@param		str		string of word to be encrypted
	@return		string of encrypted word
*/
string encrypt_word(string str)
{
	string word = "";
	int offset = 4, len = str.length();

	for (int i = 0; i < len; i++)
	{
		int c = str[i];
		char encrypt_key;

		if (isdigit(c))
		{
			// convert the character into a value between 0 and 9 (10 characters from 0 to 9)
			c = c - '0';
			//Increment the character by the offset provided; account for the case where the character
			// has to wrap around to the front of the single digit count
			c = (c + 4) % 10;
			// convert key back to ASCII values
			c = '0' + c;

			// add character to resulting word
			encrypt_key = (char) c;
			word = word + encrypt_key;

		}
		else if (!isalnum(c)) 
		{
			encrypt_key = (char) c;
			word = word + encrypt_key;
		}
		else if (isupper(c))
		{
			// convert the character into a value between 0 and 25 (26 characters in the alphabet)
			c = c - 'A';

			//Increment the character by the offset provided; account for the case where the character
			// has to wrap around to the front of the alphabet
			c = (c + offset) % 26;

			// convert key back to ASCII values
			c = 'A' + c;

			// add character to resulting word
			encrypt_key = (char) c;
			word = word + encrypt_key;
		}
		else if (islower(c))
		{
			// convert the character into a value between 0 and 25 (26 characters in the alphabet)
			c = c - 'a';

			//Increment the character by the offset provided; account for the case where the character
			// has to wrap around to the front of the alphabet
			c = (c + offset) % 26;

			// convert key back to ASCII values
			c = 'a' + c;

			// add character to resulting word
			encrypt_key = (char) c;
			word = word + encrypt_key;
		}

		//word = word + (char)c;
	}

	return word;
}

/*
	Splits string into two words using a comma, "," , as a 
	delimiter

	@param		sentence		sentence that is to be split up
	@param		word1			pointer to where the 1st word from
								the split string is stored
	@param		word2			pointer to where the 2nd string from
								the split string is stored
*/
void split_str(string sentence, string *word1, string * word2)
{
	//string first_half, second_half;
	stringstream line_reader(sentence);
	getline(line_reader, *word1, ',');
	getline(line_reader, *word2, ',');
}


/*
	Splits string into words using a comma, "," , as a delimiter

	@param		sentence		sentence that is to be split up
	@param		count			pointer to where the 1st word from
								the split string is stored
	@param		str_arr			pointer to an array where the splits 
								words are to be stored
*/
void split_str_arr(string sentence, int count, string* str_arr)
{
	stringstream line_reader(sentence);
	for (int i = 0; i < count; i++)
	{
		getline(line_reader, str_arr[i], ',');
	}
}

/*------------------------------------ TCP/UDP HELPER FUNCTIONS ----------------------------------------*/

/*
	Gets sockaddr, for IPv4 or IPv6:
	CITATION: This function is used from Beej's Networking Tutorial

	@param	sa	sockaddr struct where the result is to be stored
*/
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*
	Sends a message to the backend EE server and recieves its response
	CITATION: This function contains code from Beej's Networking Tutorial

	@param		str_msg			the message that is to be sent to the backend
								server
	@return		returns the message received from the backend server
*/
string talk_to_serverEE(string str_msg)
{
	// START CITATION: Code from Beej's Networking Tutorial starts below:
	int numbytes;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4 // // set to AF_INET6 to use IPv6
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(localhost, EE_UDP_PORT, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	// loop through all the results and make a socket
	p = servinfo;

	if ((numbytes = sendto(sockfd_udp, str_msg.c_str(), str_msg.length(), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}

	free(servinfo);

	// END CITATION: Code from Beej's Networking Tutorial
	
	if (debug) {cout << "serverM sent the following msg to serverEE: " << str_msg << endl;}
	cout << "The main server sent a request to serverEE." << endl;

	/////////////////////////////////////////////

	// START CITATION: Code from Beej's Networking Tutorial starts below:
	struct sockaddr_storage their_addr;
	char buf[MAXDATASIZE];
	socklen_t addr_len;
	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd_udp, buf, MAXDATASIZE-1 , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	buf[numbytes] = '\0';

	// END CITATION: Code from Beej's Networking Tutorial

	string response = string(buf);

	if (debug) {cout << "serverM received the following from serverEE: " << response;}
	cout << "The main server received the response from serverEE using UDP over port "; 
	cout << MAIN_UDP_PORT << endl;

	return response;

}

/*
	Sends a message to the backend CS server and recieves its response
	CITATION: This function contains code from Beej's Networking Tutorial

	@param		str_msg			the message that is to be sent to the backend
								server
	@return		returns the message received from the backend server
*/
string talk_to_serverCS(string str_msg)
{
	// START CITATION: Code from Beej's Networking Tutorial starts below:
	int numbytes;
	int newsockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4 // // set to AF_INET6 to use IPv6
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(localhost, CS_UDP_PORT, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((newsockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) 
	{
		fprintf(stderr, "talker: failed to create socket\n");
		exit(2);
	}

	if ((numbytes = sendto(newsockfd, str_msg.c_str(), str_msg.length(), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}

	free(servinfo);

	// END CITATION: Code from Beej's Networking Tutorial
	
	if (debug) {cout << "serverM send the following msg to serverCS: " << str_msg << endl;}
	cout << "The main server sent a request to serverCS." << endl;

	/////////////////////////////////////////////

	// START CITATION: Code from Beej's Networking Tutorial starts below:
	struct sockaddr_storage their_addr;
	char buf[MAXDATASIZE];
	socklen_t addr_len;
	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(newsockfd, buf, MAXDATASIZE-1 , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	buf[numbytes] = '\0';

	close(newsockfd);
	// END CITATION: Code from Beej's Networking Tutorial

	string response = string(buf);

	if (debug) {cout << "serverM received the following from serverCS: " << response << endl;}
	cout << "The main server received the response from serverCS using UDP over port "; 
	cout << MAIN_UDP_PORT << endl;

	return response;

}

/*
	Sends a message to the backend credential server and recieves its response
	CITATION: This function contains code from Beej's Networking Tutorial

	@param		str_msg			the message that is to be sent to the backend
								server
	@return		returns the message received from the backend server
*/
string talk_to_serverC(string str_msg)
{
	// START CITATION: Code from Beej's Networking Tutorial starts below:
	int numbytes;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	string encrypt_msg = encrypt_word(str_msg);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4 // // set to AF_INET6 to use IPv6
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(localhost, C_UDP_PORT, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	// loop through all the results and make a socket
	p = servinfo; 

	if ((numbytes = sendto(sockfd_udp, encrypt_msg.c_str(), encrypt_msg.length(), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}
	
	free(servinfo);

	// END CITATION: Code from Beej's Networking Tutorial

	cout << "The main server sent an authentication request to serverC." << endl;

	/////////////////////////////////////////////

	// START CITATION: Code from Beej's Networking Tutorial starts below:
	struct sockaddr_storage their_addr;
	char buf[MAXDATASIZE];
	socklen_t addr_len;
	addr_len = sizeof their_addr;

	if ((numbytes = recvfrom(sockfd_udp, buf, MAXDATASIZE-1 , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	buf[numbytes] = '\0';

	cout << "The main server received the result of the authentication request from ";
	cout << "ServerC using UDP over port " << MAIN_UDP_PORT << endl;

	string response = string(buf);

	if (debug) {cout << "serverM received: " << response << " from serverC" << endl;}

	return response;
	// END CITATION: Code from Beej's Networking Tutorial

}

/*
	Creates the UDP socket for serverM to be used to send and receive UDP messages
	CITATION: This function contains code from Beej's Networking Tutorial

	@return		returns 0 if successful or a negative int if unsuccessful
*/
int create_udp_server_socket(void)
{
	// START CITATION: Code from Beej's Networking Tutorial starts below:
    struct sockaddr_in main_sockaddr_in;

    sockfd_udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd_udp == -1)
    {
        perror("serverM: socket creation error. Exiting.");
        exit(1);
    }

    memset((char *) &main_sockaddr_in, 0, sizeof(main_sockaddr_in));
    main_sockaddr_in.sin_family = AF_INET;
    main_sockaddr_in.sin_port = htons(str_to_int(MAIN_UDP_PORT));
	inet_aton(localhost, &main_sockaddr_in.sin_addr);

    if (bind(sockfd_udp, (struct sockaddr*) &main_sockaddr_in, sizeof(main_sockaddr_in))==-1)
    {
        perror("serverM: socket bind error. Exiting.");
        exit(1);
    }

    // END CITATION: Code from Beej's Networking Tutorial

    return 0;
}

/*
	Creates the TCP socket for serverM to be used to send and receive TCP messages
	CITATION: This function contains code from Beej's Networking Tutorial

	@return		returns 0 if successful or a negative int if unsuccessful
*/
int create_tcp_server_socket(void)
{
	// START CITATION: Code from Beej's Networking Tutorial starts below:
	struct addrinfo hints, *servinfo, *p;
	struct sigaction sa;
	int yes=1;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

    // getaddrinfo returns a pointer to the start of a linked list of address data structures on 
    // the host computer that meet the requirements
	if ((rv = getaddrinfo(NULL, MAIN_TCP_PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

    // loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
    {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) 
        {
			perror("server serror: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1)
        {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
        {
			close(sockfd);
			perror("server error: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  
	{
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	// END CITATION: Code from Beej's Networking Tutorial

	if (debug) {printf("serverM TCP: waiting for connections...\n");}

	return 0;
}


/*------------------------------------ AUTHENTICATION FUNCTIONS ---------------------------------------*/

/*
	Confirms the client is authorized to query department databases. Recieves the username and password
	from the client via TCP, encrypts the data and sends via UDP connection to the credential server. The results
	of the authorization check  are sent back to serverM and then forwarded to the client.

	CITATION: This function contains code from Beej's Networking Tutorial

*/
void authenticate_client(void)
{
	// START CITATION: Code from Beej's Networking Tutorial starts below:
	int numbytes, num_tries = 1;
	struct sockaddr_storage their_addr;
	socklen_t sin_size = sizeof their_addr;
	char s[INET6_ADDRSTRLEN];
	char buf[MAXDATASIZE];
	string val;

	child_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
	if (child_fd == -1) {perror("could not accept TCP connection. Exiting.");}
	if (debug)
	{
		inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);
	}

	while(!authenticate_done) 
    {   
		
		if (num_tries > 3)
		{
			close(child_fd);
			return;
		}

		sin_size = sizeof their_addr;
		string client_msg = "";
		
		if ((numbytes = recv(child_fd, buf, MAXDATASIZE-1, 0)) == -1) 
		{
			perror("recv");
			exit(-1);
		}
		
		buf[numbytes] = '\0';
		client_msg = string(buf);

		if (debug)
		{
			printf("server: received the following string from the client'%s'\n", buf);
		}
		buf[0] = 0;

		string user = "", pass = "";
		split_str(client_msg, &user, &pass);
		
		cout << "The main server received the authentication for " << user;
		cout << " using TCP over port " << MAIN_TCP_PORT << "." << endl;


		string str_result = talk_to_serverC(client_msg);
		int result = str_to_int(str_result);
		if (result == PASS) 
		{
			authenticate_done = TRUE;
			curr_user = user;
		}
		
		if (send(child_fd, str_result.c_str(), str_result.length(), 0) == -1) 
		{
			perror("sending error on the serverM side. Exiting.\n");
			exit(-1);
		}
		// END CITATION: Code from Beej's Networking Tutorial

		num_tries++;

		cout << "The main server sent the authentication result to the client." << endl;
	}
}



/*----------------------------------------- QUERY FUNCTIONS --------------------------------------------*/

/*
	Helper function to the query driver function. Uses predetermined encoding between the server 
	and client to send the query to the specified department server. Receives a single or
	multiple queries. 

	@param		msg				the query message sent from the client
	@param		exit_status		pointer to an int where the exit status of the function
								should be stored
	@return		the response from the backend servers regarding the queries

*/
string query_helper(string msg, int* exit_status)
{
	string response = "";
	string outgoing_msg = "";

	// converts the encoding from the client message into a size of
	// how many queries are contained in the message
	int q_size = str_to_int(string(1, msg[0]));
	if (q_size == 1 && msg[1] == '0') {q_size = 10;}
	
	// If the client only sent 1 query, enter this if bracket
	if (q_size == 1)
	{
		string course_code, type;
		string str_arr[3];

		// split the client message into 3 spearate parts
		// delimited by a comma
		split_str_arr(msg, 3, str_arr);

		// remove the encoding create a new message to send to the department servers
		string new_msg = str_arr[1] + "," + str_arr[2];
		
		// store the course code (E.g: "EE450" or "CS401")
		course_code = str_arr[1];
		// store type (e.g: "EE" or "CS")
		type = course_code.substr(0, 2);

		cout << "The main server received from " << curr_user << " to query course ";
		cout << course_code << " about " << str_arr[2] << " using TCP over port " << MAIN_TCP_PORT << ".";
		cout << endl;

		if ("EE" == type) 
		{
			outgoing_msg = talk_to_serverEE(new_msg);
		}
		else if ("CS" == type) 
		{
			outgoing_msg = talk_to_serverCS(new_msg);
		}
		else 
		{
			// If the type does not match "EE" or "CS", send to the EE server as a default
			// The EE server will send an error msg
			outgoing_msg += talk_to_serverEE(new_msg);
			/*
			cerr << "recieved str: " << course_code << " : had a format error. Query must begin with EE or CS.";
			cout << " exiting." << endl;
			exit(-1);
			*/
		}

		// do stuff
	}
	else if (q_size > 1) // Handle the extra credit case of sending multiple queries to the backend
	{
		string course_code;
		string str_arr[q_size + 1];
		split_str_arr(msg, q_size + 1, str_arr);

		// iterate through the number of queries
		for (int i = 1; i < q_size + 1; i++)
		{
			string type;
			char c; 

			course_code = str_arr[i];
			c = (char) course_code[0];
			type += c;
			c = (char) course_code[1];
			type += c;

			if ("EE" == type) 
			{
				outgoing_msg += talk_to_serverEE(course_code);
			}
			else if ("CS" == type) 
			{
				outgoing_msg += talk_to_serverCS(course_code);
			}
			else 
			{
				// If the type does not match "EE" or "CS", send to the EE server as a default
				// The EE server will send an error msg
				outgoing_msg += talk_to_serverEE(course_code);

				/*
				cerr << "recieved str: " << course_code << " : had a format error. Query must begin with EE or CS.";
				cout << " exiting." << endl;
				exit(-1);
				*/
			}

		}
	}
	else 
	{
		// QUESTION: how to tell if a connection with a client has been closed
		cerr << "recieved str from client had an error.";
		cerr << "Or client closed socket. " << endl << "Exiting current client's session." << endl;
		*exit_status = -1;
		return outgoing_msg;
		//cerr << "recieved str had an error. exiting." << endl;
		//exit(-1);
	}

	*exit_status = 0;
	return outgoing_msg;
}

/*
	Receives the query inquiry from the client. Sends the response from the backend servers to the client.

	CITATION: This function contains code from Beej's Networking Tutorial 

	@return		returns 0 if successful or a negative int if unsuccessful

*/
int query_driver(void)
{
	int numbytes;
	char buf[MAXDATASIZE];

	while(TRUE) 
    {

		string client_msg = "";
		
		if ((numbytes = recv(child_fd, buf, MAXDATASIZE-1, 0)) == -1) 
		{
			perror("Error with recv on serverM side. Exiting current client's session.\n");
			return -1;
			//exit(-1);
		}
		buf[numbytes] = '\0';
		client_msg = string(buf);

		if (debug)
		{
			printf("server: received the following string from the client: '%s'\n", buf);
		}
		
		int exit_status;
		string response = query_helper(client_msg, &exit_status);
		if (exit_status < 0) {return -1;}

		//sprintf(buf, "%s", response);
		if (send(child_fd, response.c_str(), response.length(), 0) == -1) 
		{
			perror("Sending error on the serverM side. Exiting current client's session.\n");
			return -1;
			//exit(-1);
		}

		cout << "The main server sent the query information to the client." << endl;

	}
	
	return 0;
}



/*-------------------------------------- PROGRAM FUNCTIONS ---------------------------------------------*/

/*
	Helper driver function to initiate and run serverM.
*/
void run_program (void)
{
    create_tcp_server_socket();
   	create_udp_server_socket();

	cout << "The main server is up and running." << endl;

	// QUESTION: How to check if a connection has been closed
    while (TRUE)
	{
		while(!authenticate_done) {authenticate_client();}
		query_driver();
		close(child_fd);
		authenticate_done = FALSE;
	}
}

int main(int argc, char *argv[])
{
   
   //if (argc > 1) {debug = TRUE;}
	
	run_program();
 
   return 0;
}
