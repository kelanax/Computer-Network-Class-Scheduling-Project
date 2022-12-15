
#include "globals.h"
using namespace std;

/******************************************************************************/
/*

serverEE is a department databse server, which replies to and fulfills query 
requests from the client sent by the main central registration server.
Only autheticated clients may query department servers via serverM.

To run:
make all
./serveEE


NOTE: Link to Beej's Networking Tutorial which is used in this program:
https://beej.us/guide/bgnet/html/

*/                                                                                                                
/******************************************************************************/

/*------------------------------------- CLASS DEFINITION --------------------------------------*/

class serverEE
{
    // Private instance variables and functions for the EE department server object
    private:
        map<string, Record*> ee_map;
        int sockfd;
        void loadDatabase(ifstream *fp, string fileName);
        void *get_in_addr(struct sockaddr *sa);
        
    public:
        serverEE(/* args */);
        ~serverEE();

        void print_map(map<string, Record*> *map);
        string int_to_str(int val);
        int str_to_int(string str_val);
        void split_str(string sentence, string *word1, string * word2);
        void split_str_arr(string sentence, int count, string* str_arr);
        int send_udp_info(string str_msg, struct sockaddr_storage *their_addr, socklen_t addr_len);
        string recv_udp_info(sockaddr_storage *their_addr, socklen_t *addr_len);
        int create_udp_server_socket(void);
        string query_helper(string msg);
        int query_driver(void);



};

/*
	Constructs a serverEE object. Loads database using data from "ee.txt" upon initialization.
*/
serverEE::serverEE(/* args */)
{
    ifstream file;
    string fileName = "ee.txt";
    loadDatabase(&file, fileName);
    //print_map(&ee_map);
}

/*
	Void destructor for the serverEE object
*/
serverEE::~serverEE()
{
}


/*---------------------------------------- UTILITIES -------------------------------------------*/

/*
	Converts an int into a string

	@param		val		integer to be converted to a string
	@return		string of converted integer
*/
string serverEE::int_to_str(int val)
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
int serverEE::str_to_int(string str_val)
{
	return atoi(str_val.c_str());
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
void serverEE::split_str(string sentence, string *word1, string * word2)
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
void serverEE::split_str_arr(string sentence, int count, string* str_arr)
{
	//string first_half, second_half;
	stringstream line_reader(sentence);
	for (int i = 0; i < count; i++)
	{
		getline(line_reader, str_arr[i], ',');
	}
}

/*
	Loads data from the text file "ee.txt" into the database.

	@param		fp		        file stream pointer for reading data from the file
	@param		filename		name of the file to be loaded into the data base
*/
void serverEE::loadDatabase(ifstream *fp, string fileName)
{   
    (*fp).open(fileName.c_str());
    string line, temp = "";
    int num_of_fields = 5, len = 0;

    if(!(*fp).is_open()) //TODO
    {
      cerr << "Unable to open file. Please provide valid \"" << fileName << "\" file.\n" << endl;
      exit(EXIT_FAILURE);
    }
    
    while(getline((*fp), line)) 
    {
        stringstream line_reader(line);
        if (line[0] == '\n' || line[0] == '\r') {continue;}

        Record* new_record = new Record;

        for (int i = 1; i <= num_of_fields && line_reader.good(); i++)
        {
            switch (i)
            {
                case 1:
                    getline(line_reader, new_record->course_code, ',');
                    break;
                case 2:
                    getline(line_reader, new_record->credit, ',');
                    break;
                case 3:
                    getline(line_reader, new_record->professor, ',');
                    break;
                case 4:
                    getline(line_reader, new_record->days, ',');
                    break;
                case 5:
                    getline(line_reader, temp, ',');
                    len = temp.length();
                    if (temp[len-1] == '\n' || temp[len-1] == '\r') {new_record->course_name = temp.substr(0, len-1);}
                    else {new_record->course_name = temp;}
                    break;
                default:
                    break;
            }
        }
        ee_map[new_record->course_code] = new_record;
    }

    (*fp).close();
}

/*
	Iterates through every record entry in the map and prints it.

	@param		map_var     variable of map that is to be iterated through
*/
void serverEE::print_map(map<string, Record*> *map_var)
{
    map<string, Record*> ::iterator iter;
    Record * record_elem;
    for (iter = map_var->begin(); iter != map_var->end(); iter++)
    { 
        cout << "Key: " << iter->first << endl;
        cout << "Record Items:" << endl;

        record_elem = iter->second;

        cout << "Course Code: " << record_elem->course_code << endl;
        cout << "Credit: " << record_elem->credit << endl;
        cout << "Days: " << record_elem->days << endl;
        cout << "Course Name: " << record_elem->course_name << endl;
        cout << "-----------------------------------------" << endl;
    }

}

/*-------------------------------------- UDP FUNCTIONS -----------------------------------------*/

/*
	Gets sockaddr, for IPv4 or IPv6:
	CITATION: This function is used from Beej's Networking Tutorial

	@param	sa	sockaddr struct where the result is to be stored
*/
void *serverEE::get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*
	Sends UDP message to the main server using the deparment socket.

	@param      str_msg		    the string message to be sent to the main server
    @param      their_addr      struct holding the address of the final destination
    @param      addr_len        length of the destination address

    @return		returns 0 if successful or a negative int if unsuccessful

*/
int serverEE::send_udp_info(string str_msg, struct sockaddr_storage *their_addr, socklen_t addr_len)
{
    int numbytes;

    if ((numbytes = sendto(sockfd, str_msg.c_str(), str_msg.length(), 0,
             (const struct sockaddr *)their_addr, addr_len)) == -1) {
        perror("serverEE: sendto");
        exit(1);
    }

    cout << "The serverEE finished sending the response to the Main Server." << endl;

    return 0;
}

/*
	Receives UDP message from the main server.

    @param      their_addr      struct to hold the address of the source of the message
    @param      addr_len        length of the source address

    @return		string of the message received

*/
string serverEE::recv_udp_info(sockaddr_storage *their_addr, socklen_t *addr_len)
{
    char buf[MAXDATASIZE];
    int numbytes;

    if (((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *)their_addr, addr_len)) == -1)) 
    {
        perror("recvfrom");
        exit(1);
    }
            
    buf[numbytes] = '\0';
    string msg(buf);

    return msg;
}

/*
	Creates the UDP socket for serverM to be used to send and receive UDP messages
	CITATION: This function contains code from Beej's Networking Tutorial

	@return		returns 0 if successful or a negative int if unsuccessful
*/
int serverEE::create_udp_server_socket(void)
{
    // START CITATION: Code from Beej's Networking Tutorial starts below:
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4 // // set to AF_INET6 to use IPv6
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(localhost, EE_UDP_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }


    // This gets the remote address of the serverEE server
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        //return 2;
        exit(2);
    }

    cout << "The serverEE is up and running using UDP on port " << EE_UDP_PORT << "." << endl;

    freeaddrinfo(servinfo);
    // END CITATION: Code from Beej's Networking Tutorial

    return 0;
}


/*-------------------------------------- QUERY FUNCTIONS ---------------------------------------*/

/*
	Helper function to the query driver function. Extracts course code from client message and 
    searches the data base for the requested message. Returns error encoding if information
    cannot be found. Otherwise returns the data requested

	@param		msg		the query message sent from the client

	@return		the response from the backend server regarding the queries

*/
string serverEE::query_helper(string msg)
{
    string outgoing_msg = "";
    size_t found = msg.find(",");

    // didn't find a "," in the string which means this is a single request 
    // of format --> EE__ (so gather and send data for all categories)
    if (found == std::string::npos) 
    {
        string course_code = msg;

        cout << "The serverEE recieved received a request from the Main Server ";
        cout << "about " << course_code << "." << endl;

        Record* found_record;
        if (ee_map.find(course_code) != ee_map.end())
        {
            found_record = ee_map[course_code];

            // 1,1,EE450: 4, Ali Zahid, Tue;Thu, Introduction to Computer Networks
            // EE669: 4, Jay Kuo, Mon;Wed, Multimedia Data Compression
            // CS402: 4, William Cheng, Mon;Wed, Operating Systems

            outgoing_msg += "1,1," + course_code + ": " + found_record->credit;
            outgoing_msg += "," + found_record->professor; 
            outgoing_msg += "," + found_record->days;
            outgoing_msg += "," + found_record->course_name;

            outgoing_msg += "$";

            // QUESTION: No message specified for extra credit in this case. what to add?
            cout << "The course information has been found for: " << course_code << "." << endl;

        }
        else
        {
            // QUESTION: No message specified for extra credit in this case. what to add?
            cout << "Didn’t find the course: " << course_code << "." << endl;
            outgoing_msg = "1,0," + course_code + "$";
        }
    }
    // found a "," in the string which means this is a category request 
    // of format --> EE__,Category
    else
    {
        string course_code, category;
        split_str(msg, &course_code, &category);
        Record* found_record;

        cout << "The serverEE recieved received a request from the Main Server ";
        cout << "about the " << category << " of " << course_code << "." << endl;

        if (ee_map.find(course_code) != ee_map.end())
        {
            found_record = ee_map[course_code];
            string info;

            // Examples:
            // EE450: 4, Ali Zahid, Tue;Thu, Introduction to Computer Networks
            // EE669: 4, Jay Kuo, Mon;Wed, Multimedia Data Compression
            // CS402: 4, William Cheng, Mon;Wed, Operating Systems

            outgoing_msg += "0,1," + course_code + "," + category + ","; 
            if (category == "Credit") {
                info += found_record->credit;
                outgoing_msg += info;}
            else if (category == "Professor") {
                info += found_record->professor;
                outgoing_msg += info;} 
            else if (category == "Days") {
                info += found_record->days;
                outgoing_msg += info;}
            else if (category == "CourseName") {
                info += found_record->course_name;
                outgoing_msg += info;}
            else 
            {
                // QUESTION: No message specified for if you can't find the course category; what to add?
                cout << "Didn’t find the course category: " << course_code << "." << endl;
                outgoing_msg = "0,0," + course_code + "," + category + "$";
                return outgoing_msg;
            }
            outgoing_msg += "$";
            cout << "The course information has been found: The " << category << " of ";
            cout << course_code << " is " << info << "." << endl;
        }
        else
        {
            cout << "Didn’t find the course: " << course_code << "." << endl;
            outgoing_msg = "0,0," + course_code + "," + category + "$";
        }

    }
    
    return outgoing_msg;
}

/*
	Receives the query inquiry from the serverM. Sends the result of query back to resverM.

	@return		returns 0 if successful or a negative int if unsuccessful

*/
int serverEE::query_driver(void)
{
    while(TRUE)
    {
        string client_msg;
        struct sockaddr_storage their_addr;
        socklen_t addr_len = sizeof their_addr;

        memset(&their_addr, 0, sizeof their_addr);
        client_msg = recv_udp_info(&their_addr, &addr_len);

        string outgoing_msg = query_helper(client_msg);

        addr_len = sizeof their_addr;
        send_udp_info(outgoing_msg, &their_addr, addr_len);
    }
    
   return 0; 
}


/*------------------------------------------- MAIN ---------------------------------------------*/

int main(int argc, char *argv[])
{
    // NOTE: when querying, convert text to uppercase (e.g "ee450" becomes "EE450")

    serverEE ee_server;
    ee_server.create_udp_server_socket();
    ee_server.query_driver();
    return 0;
}