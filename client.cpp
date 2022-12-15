
#include "globals.h"
using namespace std;

/******************************************************************************/
/*

The client program is a web registration query system that facilities the 
querying of the upcoming's semester's course data upon the autherntication of 
the connected client. The client program interacts with the main serverM to send 
and recieve authentication and query messages.

To run:
make all
./client

NOTE: Link to Beej's Networking Tutorial which is used in this program:
https://beej.us/guide/bgnet/html/

*/                                                                                                                
/******************************************************************************/

/*---------------------------------------------------- GLOBAL VARIABLES ---------------------------------------------------------------------*/

int sockfd; // TCP socket for the client
int debug = FALSE;
unsigned int clientPort; // store the dynamic TCP client socket in global variable
//QUESTION: If I don't close the client socket for the duration of the exchange, can 
//          just use the "gotsockaddr" function once?
string curr_client; // store the user name of the successfully authenticated client

/*---------------------------------------------------- UTILITY FUNCTIONS ---------------------------------------------------------------------*/
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
	Splits string into words using a comma, "," , as a delimiter

	@param		sentence		sentence that is to be split up
	@param		count			pointer to where the 1st word from
								the split string is stored
	@param		str_arr			pointer to an array where the splits 
								words are to be stored
*/
void split_str_arr(string sentence, int count, string* str_arr)
{
	//string first_half, second_half;
	stringstream line_reader(sentence);
	for (int i = 0; i < count; i++)
	{
		getline(line_reader, str_arr[i], ',');
	}
}


/*------------------------------------------------------ TCP FUNCTIONS -----------------------------------------------------------------------*/

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
	Creates the TCP socket for the client to be used to send and receive TCP messages
	CITATION: This function contains code from Beej's Networking Tutorial

	@return		returns 0 if successful or a negative int if unsuccessful
*/
int create_tcp_client_socket(void)
{
    // START CITATION: Code from Beej's Networking Tutorial starts below:
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN_];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if ((rv = getaddrinfo(localhost, MAIN_TCP_PORT, &hints, &servinfo)) != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) 
    {
        // create a socket with the same attributes as the server
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1 && debug) {
            perror("error encountered while trying to create socket - client: socket");
            continue;
        }

        // connect to a server socket that has an IP address returned in the servinfo struct
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1 && debug) 
        {
            close(sockfd);
            perror("error encountered while trying to connect - client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Error: - client: failed to connect\n");
        return 2;
    }

    if (debug)
    {
        // converts a network address like, 125.125.125.125, 
        // for example, to a string like "125.125.125.125"
        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
                s, sizeof s);
        printf("client: connecting to %s\n", s);
    }

    freeaddrinfo(servinfo); // all done with this structure
    
    struct sockaddr_in clientAddress;
    bzero(&clientAddress, sizeof(clientAddress));
    socklen_t len = sizeof(clientAddress);
    getsockname(sockfd, (struct sockaddr *) &clientAddress, &len);
    clientPort = ntohs(clientAddress.sin_port);
    if (debug) {printf("Client's dynamic port number : %u\n", clientPort);}

    // END CITATION: Code from Beej's Networking Tutorial
    

    return 0;
}

/*
	Sends TCP message to the main server using the client socket

	@param      msg		the string message to be sent to the main server
*/
void send_info(string msg)
{
    int msg_len = msg.length();

    if (send(sockfd, msg.c_str(), msg_len, 0) == -1) {perror("send error on the client side\n");}
    else if (debug) 
    {
        cout << "client: sent the following string to serverM: " << msg << endl;
    }
}

/*
	Receives a TCP message from the main server using the client socket

	@return      the string message received from the main server
*/
string recv_info(void)
{
    char buf[MAXDATASIZE];
    int numbytes;

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) 
    {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';
    string msg(buf);

    if (debug) {printf("client: received the following string from the server: '%s'\n", buf);}

    //close(sockfd);

    return msg;
}


/*--------------------------------------------------- AUTHENTICATE FUNCTIONS -----------------------------------------------------------------*/

/*
	Prompts user for user name and password and then sends input to the main server. Receives 
    the result of the authentication from the main server. The client has 3 attempts to input the 
    correct username/password combo before being the system shuts down

	CITATION: This function contains code from Beej's Networking Tutorial

    PRE-CONDITIONS:
    - The username must be lowercase
    - The password is case sensitive

*/
void authenticate_client(void)
{
    int count = 3, result;
    string response;
    string username, password, msg;

    while (count > 0)
    {
        username = "";
        cout << "Please enter the username: ";
        getline(cin, username);
        
        password = "";
        cout << "Please enter the password: ";
        getline(cin, password);

        msg = username + "," + password;

        //create_tcp_client_socket();
        send_info(msg);
        cout << username << " sent an authentication request to the main server." << endl;
        response = recv_info();
        
        result = atoi(response.c_str());
        if (result == FAIL_NO_USER) 
        {
            //if (debug) {cout << "AUTHENTICATION FAIL: WRONG USERNAME. Try again!" << endl;}
            count--;
            cout << username << " received the result of authentication using TCP over port "; 
            cout << clientPort << ". Authentication failed: Username Does not exist" << endl;
            cout << endl << "Attempts remaining:" << count << endl;
        }
        else if (result == FAIL_PASS_NO_MATCH)
        {
            //if (debug) {cout << "AUTHENTICATION FAIL: WRONG PASSWORD. Try again!" << endl;}
            count--;
            cout << username << " received the result of authentication using TCP over port "; 
            cout << clientPort << ". Authentication failed: Password does not match" << endl;
            cout << endl << "Attempts remaining:" << count << endl;
        }
        else if (result == PASS)
        {
            //if (debug) {cout << endl << "AUTHENTICATION PASS: You're in!" << endl;}
            cout << username << " received the result of authentication using TCP over port ";
            cout << clientPort << "." << " Authentication is successful" << endl;
            
            curr_client = username;
            return;
        }
        response[0] = 0;
    }

    cout << "Authentication Failed for 3 attempts. Client will shut down." << endl;
    exit(-1);
}

/*------------------------------------------------------- QUERY FUNCTIONS --------------------------------------------------------------------*/

/*
	Prompts user for user name and password and then sends input to the main server. Receives 
    the result of the authentication from the main server. The client has 3 attempts to input the 
    correct username/password combo before being the system shuts down

	@param      incoming_str    the string sent to the client containing the query results

*/
void print_query_results(string incoming_str)
{
    string msg = incoming_str;
    size_t start = 0, found = msg.find("$"); // each quesry is delimited by a $
    //int done = FALSE;
    
    if (str_to_int(string(1,msg[0])) == 1) {cout << "CourseCode: Credits, Professor, Days, Course Name" << endl;}

    // loop thorugh all the results of query (single or multiple query)
    // each result is delimited by a "$"
    while (found != std::string::npos)
    {
        int len = found - start;
        string line = msg.substr(start, len-1);
        string sub_str = msg.substr(start + 4, len-4); // remove the encoding and delimiter from the message

        if (str_to_int(string(1,line[0])) == SINGLE) // use encoding to see if this is a regular sing query
        {
            // format of string received: 0,1,EE450,Category,Result
            if (str_to_int(string(1,line[2])) == SUCCESS)
            {
                string str_arr[3];
                split_str_arr(sub_str, 3, str_arr);
                cout << "The " << str_arr[1] << " of " << str_arr[0];
                cout << " is " << str_arr[2] << "." << endl;

            }
            // format of string received: 0,0,EE450,Category,Result
            else if (str_to_int(string(1,line[2])) == FAIL)
            {
                string str_arr[3];
                split_str_arr(sub_str, 3, str_arr);
                cout << "Didn't find the course: " << str_arr[0] << "." << endl;

            }
        }
        else if (str_to_int(string(1,line[0])) == MULTIPLE) // use encoding to see if this is an extra credit case
        {
            // format of string received: 1,1,EE450,Category,Result
            if (str_to_int(string(1,line[2])) == SUCCESS) {cout << sub_str << endl;}
            // format of string received: 1,0,EE450,Category,Result
            else if (str_to_int(string(1,line[2])) == FAIL)
            {
                cout << "Didn't find the course: " << sub_str << "." << endl;
            }
        }
        
        start = found + 1;
        found = msg.find("$", found + 1);

    }
}

/*
	Prompts user for a course code and a query to be sent to the central registration server.
    If the client enters a single course (e.g EE450), the client will then be pro,pted to enter
    a category (e.g (Credit / Professor / Days / CourseName)). If the client enters more than 1
    but less than 10 course codes in a row, each separated by a single space (e.g EE450, C402, EE571),
    the client will recieve all information about each course. If a course cannot be found, this 
    information will be stated to the client.

	PRE-CONDITIONS:
        - the course code must be valid (i.e begin with "EE" or "CS")
        - if prompted for a category, the category must be valid
          (i.e (Credit / Professor / Days / CourseName))
        - If entering multiple queries, must not end more than 10

*/
void query_database(void)
{
    int query_count = 0;
    string course_code, course_dept, category, word, response;
    string msg = "";

    while (TRUE)
    {
        query_count = 0;
        cout << endl << "-----Start a new request-----" << endl;

        cout << "Please enter the course code to query: " << endl;
        getline(cin, course_code);
        istringstream input_reader(course_code);
        while (input_reader >> word)
        {
            if (query_count != 0) {msg += "," + word;}
            else {msg = word;}
            query_count++;
        }

        if (query_count > 1) 
        {
            msg = int_to_str(query_count) + "," + msg;
            cout << curr_client << " sent a request with multiple CourseCode to the main server." << endl;
            send_info(msg);
            response = recv_info();

            cout << "The client received the response from the Main server using TCP over port ";
            cout << clientPort << "." << endl;
        }
        else 
        {
            msg = int_to_str(1) + "," + msg;
            cout << "Please enter the category (Credit / Professor / Days / CourseName): " << endl;
            getline(cin, category);
            
            // probably will remove
            while (category != "Credit" && category != "Professor" && category != "Days" && category != "CourseName")
            {
                cout << "Usage: (Credit / Professor / Days / CourseName)" << endl;
                cout << "Please enter the category (Credit / Professor / Days / CourseName): " << endl;
                getline(cin, category);
            }

            msg += "," + category;
            send_info(msg);
            cout << curr_client <<" sent a request to the main server." << endl;
            response = recv_info();
            cout << "The client received the response from the Main server using TCP over port ";
            cout << clientPort << "." << endl;

        }

        print_query_results(response);
    }

}


/*------------------------------------------------------ DRIVER FUNCTIONS --------------------------------------------------------------------*/

/*
	Helper driver function to initiate and run the client.
*/
void run_program(void)
{
    create_tcp_client_socket();   
    authenticate_client();
    query_database();
}




/*---------------------------------------------------------- MAIN ----------------------------------------------------------------------------*/



int main(int argc, char *argv[])
{
    // NOTE: when querying, convert text to uppercase (e.g "ee450" becomes "EE450")
    cout << "The client is up and running." << endl;
    //if (argc > 1) {debug = TRUE;}
    run_program();
    close(sockfd);
    
    return 0;
}
