/****************************************************************************
* Program:
*    Lab RockSrvT2, Rock/Paper/Scissors with Sockets - Server Code
*    Brother Jones, CS 460
* Author:
*    Nick Nelson
* Summary:
*    Take 2 for the rock paper scissors server.
*
*****************************************************************************
* Changes made to my code for the re-submission:
*   - made it object oriented, created a server class
*   - the only thing that is the same is the protocol
*   - the while loop that drives the game is also mostly the same
*****************************************************************************/

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sstream>
#include <string.h>

using namespace std;

#define BUFFER_SIZE 1024

/***********************************************************************
 * RPSServer - a class to hold the server variables and methods
 **********************************************************************/
class RPSServer {
private:
    struct sockaddr_in serverAddr,
    p1Addr,
    p2Addr;
    socklen_t p1Len,
    p2Len;
    int serverFD, // file descriptor for server
    p1FD,         // file descriptor for player 1
    p2FD,         // file descriptor for player 2
    portNumber,
    pCount,       // who is connected
    opCount;      // for verbocity and ease of reading
    char buffer[BUFFER_SIZE];
    
public:
    RPSServer(int portNumber) throw (const string);
    ~RPSServer();
    string whoWon(char p1, char p2);
    string readFromPlayer(int player) throw (const string);
    void writeToPlayer(int player, string msg) throw (const string);
    int quit(int player);
};

/***********************************************************************
 * RPSServer - constructor, create the player sockets and listen for
 *             connections.
 **********************************************************************/
RPSServer::RPSServer(int thePortNumber) throw (const string) {
    portNumber = thePortNumber;
    pCount = 0;
    opCount = 0;
    
    // create a socket for the server
    int serverFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFD < 0) {
        throw string("socket");
    }
    
    // clear the server address buffer
    bzero((char *) &serverAddr, sizeof(serverAddr));
    
    // set the server address object
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(portNumber);
    
    // bind the port to the socket
    // the first part is the fix listed at the end of this section:
    // http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#bind
    // so that you can imediately run the server again on the same port
    int yes = 1;
    if (setsockopt(serverFD,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &yes,
                   sizeof(int)) < 0) {
        throw string("setsockopt");
    }
    if (bind(serverFD,
             (struct sockaddr *) &serverAddr,
             sizeof(serverAddr)) != 0) {
        throw string("bind");
    }
    
    // listen for two connections
    listen(serverFD, 2);
    
    // game protocol basically begins here
    // get connection one
    cout << opCount++ << ": waiting for player " << pCount + 1 << endl;
    p1FD = accept(serverFD,
                  (struct sockaddr*) &p1Addr,
                  &p1Len); // get player one file descriptor
    if (p1FD == -1) {
        throw string("p1");
    }
    cout << opCount++ << ": player " << ++pCount << " has connected!" << endl;
    // tell player one to wait
    string msg = "You are player 1! You must wait for another player to join.";
    writeToPlayer(1, msg);
    
    // get connection two
    cout << opCount++ << ": waiting for player " << pCount + 1 << endl;
    p2FD = accept(serverFD,
                  (struct sockaddr*) &p2Addr,
                  &p2Len); // get player two file descriptor
    if (p2FD == -1) {
        throw string("p2");
    }
    cout << opCount++ << ": player " << ++pCount << " has connected!" << endl;
    // tell player 1 that player 2 has joined,
    // and tell player 2 that the game is ready
    writeToPlayer(1, "Player 2 has joined!");
    writeToPlayer(2, "You are player 2!");
}

/***********************************************************************
 * RPSServer - deconstructor
 **********************************************************************/
RPSServer::~RPSServer() {
    close(serverFD);
    close(p1FD);
    close(p2FD);
}

/***********************************************************************
 * writeToPlayer - send a message to the players
 **********************************************************************/
void RPSServer::writeToPlayer(int player, string msg) throw (const string) {
    cout << opCount++ << ": sending message to player " << player <<
    ": " << msg << endl;
    
    if (write(player == 1 ? p1FD : p2FD, msg.c_str(), msg.length() + 1) < 0) {
        throw string("writeToPlayer");
    }
    
    cout << opCount++ << ": message sent to player " << player << endl;
}

/***********************************************************************
 * readFromPlayer - read the input from the players
 **********************************************************************/
string RPSServer::readFromPlayer(int player) throw (const string) {
    bzero(buffer, BUFFER_SIZE);
    cout << opCount++ << ": reading from player " << player << endl;
    
    if (read(player == 1 ? p1FD : p2FD, buffer, BUFFER_SIZE) < 0) {
        throw string("readFromPlayer");
    }
    
    string reply = string(buffer);
    cout << opCount++ << ": message received from player " << player <<
    ": " << reply << endl;
    bzero(buffer, BUFFER_SIZE);
    return reply;
}

/***********************************************************************
 * quit - takes the parameter of which player quit in order to display
 **********************************************************************/
int RPSServer::quit(int player) {
    cout << opCount++ << ": recieved quit message from player ";
    cout << player << " - quitting" << endl;
    writeToPlayer(1, "Quitting");
    writeToPlayer(2, "Quitting");
    return 0;
}

/***********************************************************************
* whoWon - determine the winner
 **********************************************************************/
string whoWon(string p1, string p2) {
    string winner;
    if (p1 == p2) {
        winner = "0";
    } else if ((p1 == "r" && p2 == "s") ||
               (p1 == "p" && p2 == "r") ||
               (p1 == "s" && p2 == "p")) {
        winner = "1";
    } else {
        winner = "2";
    }
    return winner;
}

/***********************************************************************
 * go - control the whole program, throw all errors back to main
 **********************************************************************/
int go(int argc, char *argOne) throw (const string) {
    // error message for wrong number of arguments
    if (argc < 2 || argc > 2) {
        throw string("args");
    }
    
    // get portnumber, error message if not a number
    int portNumber;
    istringstream ss(argOne);
    if (!(ss >> portNumber)) {
        throw string("port");
    }
        
    // system calls or bust
    // initialize server with the RPSServer constructor
    // after the constructor is done executing there should be two
    // players ready to play the game
    RPSServer server(portNumber);
    
    // play game
    while(true) { // infinite loop to run game until the players quit
        // get both players moves
        string pOneMove = server.readFromPlayer(1);
        string pTwoMove = server.readFromPlayer(2);
        
        if (pOneMove == "q" || pTwoMove == "q") {
            return server.quit(pOneMove == "q" ? 1 : 2);
        }
        
        // determine winner
        string winner = whoWon(pOneMove, pTwoMove);
        
        // tell players who won
        server.writeToPlayer(1, winner);
        server.writeToPlayer(2, winner);
        
        // go back to top of loop for new game
    }
}

/***********************************************************************
 * main - takes one parameter: a port number. sends control to the
 *        function go(), and only handles errors here.
 **********************************************************************/
int main(int argc, char *argv[]) {
    int rc;
    try {
        rc = go(argc, argv[1]);
    } catch (string ex) {
        if (ex == "args") {
            cout <<
            "ERROR: Please enter the port number and no other arguments."
            << endl;
            rc = 1;
        }
        if (ex == "port") {
            cout << "ERROR: Invalid port number " << endl;
            rc = 2;
        }
        if (ex == "socket") {
            cout << "ERROR: an error occurred, socket failed to open" << endl;
            rc = 3;
        }
        if (ex == "setsockopt") {
            cout << "ERROR: setsockopt(SO_REUSEADDR) failed" << endl;
            rc = 4;
        }
        if (ex == "bind") {
            cout << "ERROR: could not bind to port " << argv[1] << endl;
            rc = 5;
        }
        if (ex == "p1") {
            cout << "ERROR: could not accept player 1" << endl;
            rc = 6;
        }
        if (ex == "p2") {
            cout << "ERROR: could not accept player 2" << endl;
            rc = 7;
        }
        if (ex == "writeToPlayer") {
            cout << "ERROR: could now send message to player" << endl;
            rc = 8;
        }
    }
    
    return rc;
}
