
#include "globals.h"
using namespace std;

/******************************************************************************/
/*

serverC is a credential server, which processes and returns a response to the
authentication requests intiated from the client and sent to serverC via the main 
central server. The credential server contains encrypted username and password 
information that can be used to authenticate or deny a client access

To run:
make all
./serveC

NOTE: Link to Beej's Networking Tutorial which is used in this program:
https://beej.us/guide/bgnet/html/

*/                                                                                                                
/******************************************************************************/

/*------------------------------------- CLASS DEFINITION --------------------------------------*/

class serverC
{
    private:
        map<string, cred*> c_map;
        void loadDatabase(ifstream *fp, string fileName);
        int sockfd;
        int debug;
        //struct sockaddr_storage main_addr;
        //struct sockaddr main_addr;
        //socklen_t main_addrlen;
        
    public:
        serverC(/* args */);
        ~serverC();
        int getZero();
        void print_map(map<string, cred*> *map_var);
        int create_udp_server_socket(void);
        void *get_in_addr(struct sockaddr *sa);
        string recv_udp_info(struct sockaddr_storage *, socklen_t *);
        int send_udp_info(string, struct sockaddr_storage *, socklen_t);
        int authentication_check(string);
        int authentication_driver(void);
        void set_debug(void);
        void split_str(string, string*, string*);
};

/*
	Constructs a serverEE object. Loads database using data from "cred.txt" upon initialization.
*/
serverC::serverC(/* args */)
{
    debug = FALSE;
    ifstream file;
    string fileName = "cred.txt";
    loadDatabase(&file, fileName);
    //print_map(&c_map);
}

/*
	Void destructor for the serverC object
*/
serverC::~serverC()
{
}

/*---------------------------------------- UTILITIES -------------------------------------------*/

// used for debugging only
void serverC::set_debug(void)
{
    debug = TRUE;
}

/*
	Splits string into two words using a comma, "," , as a 
	delimiter

	@param		sentence		sentence that is to be split up
	@param		res_user		pointer to where the username from
								the split string is to be stored
	@param		res_pass		pointer to where the password from
								the split string is to be stored
*/
void serverC::split_str(string sentence, string *res_user, string * res_pass)
{
	//string first_half, second_half;
	stringstream line_reader(sentence);
	getline(line_reader, *res_user, ',');
	getline(line_reader, *res_pass, ',');
}

/*
	Loads data from the text file "cred.txt" into the database.

	@param		fp		        file stream pointer for reading data from the file
	@param		filename		name of the file to be loaded into the data base
*/
void serverC::loadDatabase(ifstream *fp, string fileName)
{   
    (*fp).open(fileName.c_str());
    string line;
    int num_of_fields = 2;
    string temp = "";
    int len = 0;

    if(!(*fp).is_open()) //TODO
    {
      cerr << "Unable to open file. Please provide valid \"" << fileName << "\" file.\n" << endl;
      exit(EXIT_FAILURE);
    }
    
    while(getline((*fp), line)) 
    {
        stringstream line_reader(line);
        if (line[0] == '\n' || line[0] == '\r') {continue;}

        cred* new_cred = new cred;

        for (int i = 1; i <= num_of_fields && line_reader.good(); i++)
        {
            switch (i)
            {
                case 1:
                    getline(line_reader, new_cred->user_name, ',');
                    break;
                case 2:
                    getline(line_reader, temp, ',');
                    len = temp.length();
                    if (temp[len-1] == '\n' || temp[len-1] == '\r') {new_cred->password = temp.substr(0, len-1);}
                    else {new_cred->password = temp;}
                    //getline(line_reader, new_cred->password, ',');
                    break;
                default:
                    break;
            }
        }
        c_map[new_cred->user_name] = new_cred;
    }

    (*fp).close();
}

/*
	Iterates through every record entry in the map and prints it.

	@param		map_var     variable of map that is to be iterated through
*/
void serverC::print_map(map<string, cred_struct*> *map_var)
{
    cout << endl << "-----------------------------------------" << endl;
    map<string, cred*> ::iterator iter;
    cred * cred_elem;
    for (iter = map_var->begin(); iter != map_var->end(); iter++)
    { 
        cout << "Key: " << iter->first << ": " << endl;
        cout << "Cred Items: " << endl;

        cred_elem = iter->second;

        cout << "User Name: " << cred_elem->user_name << endl;
        cout << "Password: " << cred_elem->password << endl;

        cout << "-----------------------------------------" << endl;
    }

}

/*-------------------------------------- UDP FUNCTIONS -----------------------------------------*/

/*
	Gets sockaddr, for IPv4 or IPv6:
	CITATION: This function is used from Beej's Networking Tutorial

	@param	sa	sockaddr struct where the result is to be stored
*/
void *serverC::get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*
	Sends UDP message to the main server using the cred socket.

	@param      str_msg		    the string message to be sent to the main server
    @param      their_addr      struct holding the address of the final destination
    @param      addr_len        length of the destination address

    @return		returns 0 if successful or a negative int if unsuccessful

*/
int serverC::send_udp_info(string str_msg, struct sockaddr_storage *their_addr, socklen_t addr_len)
{
    int numbytes;

    if ((numbytes = sendto(sockfd, str_msg.c_str(), str_msg.length(), 0,
             (const struct sockaddr *)their_addr, addr_len)) == -1) {
        perror("serverC: sendto");
        exit(1);
    }

    cout << "The ServerC finished sending the response to the Main Server." << endl;

    return 0;
}

/*
	Receives UDP message from the main server.

    @param      their_addr      struct to hold the address of the source of the message
    @param      addr_len        length of the source address

    @return		string of the message received

*/
string serverC::recv_udp_info(sockaddr_storage *their_addr, socklen_t *addr_len)
{
    char buf[MAXDATASIZE];
    int numbytes;
    //struct sockaddr_storage their_addr;
    //socklen_t addr_len = sizeof *their_addr;
    //char s[INET6_ADDRSTRLEN];

    if (((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *)their_addr, addr_len)) == -1) && debug) 
    {
        perror("recvfrom");
        exit(1);
    }
            
    buf[numbytes] = '\0';
    string msg(buf);
    cout << "The ServerC received an authentication request from the Main Server." << endl;

    return msg;
}

/*
	Creates the UDP socket for serverM to be used to send and receive UDP messages
	CITATION: This function contains code from Beej's Networking Tutorial

	@return		returns 0 if successful or a negative int if unsuccessful
*/
int serverC::create_udp_server_socket(void)
{
    // START CITATION: Code from Beej's Networking Tutorial starts below:
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4 // // set to AF_INET6 to use IPv6
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(localhost, C_UDP_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }


    // This gets the remote address of the serverC server
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
        exit(2);
    }

    cout << "The ServerC is up and running using UDP on port " << C_UDP_PORT << "." << endl;

    freeaddrinfo(servinfo);
    // START CITATION: Code from Beej's Networking Tutorial starts below:


    return 0;
}

/*--------------------------------- AUTHENTICATION FUNCTIONS -----------------------------------*/

/*
	Helper function. Searches the credential server database to determine if the client
    with the provided username and password is valid. Returns the results of the search.

    @return		returns: 0 username is not in cred database, 
                         1 if the password does not match the user, 
                         2 if the user passes authentication
*/
int serverC::authentication_check(string msg)
{
    string username;
    string password;
    split_str(msg, &username, &password);
    cred* cred_ptr;

    if (c_map.find(username) != c_map.end())
    {
        cred_ptr = c_map[username];

        if (strcmp(cred_ptr->password.c_str(), password.c_str()) == 0) {return PASS;}

        return FAIL_PASS_NO_MATCH;
    }

    return FAIL_NO_USER;

}

/*
	Confirms the client is authorized by checking if the client username and password are in the
    cred database. Recieves the username and password The results of the authorization 
    check are sent back to serverM .

    @return		returns 0 if successful or a negative int if unsuccessful

*/
int serverC::authentication_driver(void)
{
    while(TRUE)
    {
        string client_msg;
        struct sockaddr_storage their_addr;
        socklen_t addr_len = sizeof their_addr;

        memset(&their_addr, 0, sizeof their_addr);
        client_msg = recv_udp_info(&their_addr, &addr_len);

        int result = authentication_check(client_msg);
        
        ostringstream oss;
        oss << result;
        string new_str = oss.str();

        addr_len = sizeof their_addr;
        send_udp_info(new_str, &their_addr, addr_len);

    }
    
   return 0; 
}



/*------------------------------------------- MAIN ---------------------------------------------*/

int main(int argc, char *argv[])
{
    // QUESTION: when querying, convert text to uppercase (e.g "ee450" becomes "EE450")
    
    serverC cred_server;
    //if (argc > 1) {cred_server.set_debug();}

    cred_server.create_udp_server_socket();
    cred_server.authentication_driver();

    return 0;
}