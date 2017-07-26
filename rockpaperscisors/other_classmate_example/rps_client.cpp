/****************************************************************************
* Program:
*    Lab RockClientT2, Rock/Paper/Scissors with Sockets - Client Code
*    Brother Jones, CS 460
* Author:
*    Nick Nelson
* Summary:
*    Take 2 for the rock paper scissors client.
*
*****************************************************************************
 * Changes made to my code for the re-submission:
 *   - made it object oriented, created a client class
 *   - the only thing that is the same is the protocol
 *   - the while loop that drives the game is also mostly the same
*****************************************************************************/

#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

using namespace std;

#define BUFFER_SIZE 1024

/***********************************************************************
 * RPSClient - class that holds all the methods and variables of the 
 *             client.
 **********************************************************************/
class RPSClient {
private:
    int portNumber,
    clientFD;                      // file descriptor for the client
    string hostname;               // the server address
    struct hostent *server;        // the server info
    struct sockaddr_in serverAddr; // the server object - i think
    char buffer[BUFFER_SIZE];
    
public:
    string pNumber;                // the player number this client is
    RPSClient(string theHostname, int thePortNumber) throw (const string);
    void instructions();
    string readFromServer() throw (const string);
    void writeToServer(string msg) throw (const string);
};

/***********************************************************************
 * RPSClient -  constructor, takes two parameters, the hostname and the
 *              port number. 
 **********************************************************************/
RPSClient::RPSClient(string theHostname,
                     int thePortNumber) throw (const string) {
    hostname = theHostname;
    portNumber = thePortNumber;
    
    // create socket
    clientFD = socket(AF_INET, SOCK_STREAM, 0);
    if (clientFD == -1) {
        throw string("socket");
    }
    
    // make sure there is a host at hostname, see following link
    // http://www.retran.com/beej/gethostbynameman.html
    server = gethostbyname(hostname.c_str());
    if (server == NULL) {
        throw string("badServer");
    }
    
    // clear server buffer
    bzero((char *) &serverAddr, sizeof(serverAddr));
    
    // create the server object?
    serverAddr.sin_family = AF_INET; // for IPv4 im pretty sure
    // this copies from the first param to the second param
    bcopy((char *)server->h_addr,
          (char *) &serverAddr.sin_addr.s_addr,
          server->h_length);
    serverAddr.sin_port = htons(portNumber);
    
    // connect
    int cr = connect(clientFD,
                     (struct sockaddr*) &serverAddr,
                     sizeof(serverAddr));
    if (cr == -1) throw string("connect");
    
    // read server response
    string fromServer = readFromServer();
    if (fromServer ==
        "You are player 1! You must wait for another player to join.") {
        cout << fromServer << endl;
        pNumber = "1";
        // wait for the server to say player 2 has joined
        fromServer = readFromServer();
    } else {
        pNumber = "2";
    }
    cout << fromServer << endl;
    
    cout << "ready to play?" << endl;
}

/***********************************************************************
 * readFromServer - read the response from the server
 **********************************************************************/
string RPSClient::readFromServer() throw (const string) {
    if (read(clientFD, buffer, BUFFER_SIZE) < 0) {
        throw string("readFromServer");
    }
    return string(buffer);
}

/***********************************************************************
 * writeToServer - send game move to server
 **********************************************************************/
void RPSClient::writeToServer(string msg) throw (const string) {
    if (write(clientFD, msg.c_str(), msg.length()) < 0) {
        throw string("writeToServer");
    }
}

/***********************************************************************
 * instructions - display the instructions to play the game
 **********************************************************************/
void instructions() {
    cout << endl << "How to play:" << endl;
    cout << "h - display this help message" << endl;
    cout << "q - quit the game" << endl;
    cout << "r - rock" << endl;
    cout << "p - paper" << endl;
    cout << "s - scissors" << endl;
}

/***********************************************************************
 * go - run the whole program, throw all errors back to main.
 **********************************************************************/
int go(int argc, char *argv[]) throw (const string) {
    // error message and exit for wrong number of arguments
    if (argc < 3 || argc > 3) {
        throw string("args");
    }
    
    // error message and exit for port number that is not a number
    char *hostname = argv[1];
    istringstream ss(argv[2]);
    int portNumber;
    if (!(ss >> portNumber)) {
        throw string("port");
    }
    
    // system calls or bust
    // create client
    RPSClient client(hostname, portNumber);
        
    // play game
    while(true) {
        // get input
        cout << "\nrps> ";
        string m;
        getline(cin, m);
        
        // validate input
        if (m == "q" || m == "r" || m == "p" || m == "s") {
            client.writeToServer(m); // send the move to the server
        } else if (m == "h") {
            instructions();
            continue;
        } else {
            cout << "Invalid input, enter 'h' for help." << endl;
            continue;
        }
        
        // read from server
        string gameResult = client.readFromServer();
        if (gameResult == "Quitting") {
            if (m != "q") {
                cout << "The other player left the game" << endl;
            }
            cout << gameResult << endl;
            break;
        } else if (gameResult == "0") {
            cout << "Tie game!" << endl;
        } else {
            cout <<
            (client.pNumber == gameResult ? "You win!" : "You lost, try again")
            << endl;
        }
    }
    return 0;
}

/***********************************************************************
 * main - takes two parameters: a hostname and a port number. This sends
 *        control of the program to the function go(), and only handles 
 *        errors here.
 **********************************************************************/
int main(int argc, char *argv[]) {
    int rc = 0;
    try {
        rc = go(argc, argv);
    } catch (string ex) {
        if (ex == "args") {
            cout <<
            "Please enter the hostname and port number (in that order)."
            << endl;
            rc = 1;
        }
        if (ex == "port") {
            cout << "invalid portnumber" << endl;
            rc = 2;
        }
        if (ex == "socket") {
            cout << "could not create socket" << endl;
            rc = 3;
        }
        if (ex == "badServer") {
            cout << "no such host at " << argv[1] << endl;
            rc = 4;
        }
        if (ex == "connect") {
            cout << "could not connect to the server" << endl;
            rc = 5;
        }
        if (ex == "readFromServer") {
            cout << "ERROR: could not read from socket" << endl;
            rc = 6;
        }
        if (ex == "writeToServer") {
            cout << "could not write to server" << endl;
            rc = 7;
        }
    }
    return rc;
}