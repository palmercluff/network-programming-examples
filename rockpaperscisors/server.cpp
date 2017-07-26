/***********************************************************************
* Program:
*    Lab RockSrvT2, Rock/Paper/Scissors with Sockets - Server Code
*    Brother Jones, CS 460
* Author:
*    Gage Peterson
* Summary:
*     A simple server in the internet domain using TCP
*     The port number is passed as an argument
*****************************************************************************
* Changes made to my code for the re-submission:
*   - I added more comment headers
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

#define BSIZE 256
#define BSIZE_LESS 255

#define ROCK 0
#define PAPER 1
#define SCISSORS 2
#define BAD_MOVE -1

void error(const char *msg) {
    perror(msg);
    exit(1);
}

/**********************************************************************
 * This will compare the first two letters of a cstring. This is used
 * to check agianst the two letter codes that our protocol uses
 ***********************************************************************/
bool compareTwo(char const* input, char const* against) {
    return toupper(input[0]) == against[0] &&
           toupper(input[1]) == against[1];
}

/**********************************************************************
 * This converts the cstring command into an integer.
 ***********************************************************************/
int toInt(char buff[]) {
    if (compareTwo(buff, "RO")) return ROCK;
    if (compareTwo(buff, "PA")) return PAPER;
    if (compareTwo(buff, "SC")) return SCISSORS;
    else                        return BAD_MOVE;
}

/**********************************************************************
 * This is basically the Tic-tac toe logic. It tells who wins by
 * returning a -1 for p1 win, 1 for p2 win and a 0 for a tie.
 * bad moves always loose. Two bad moves results in a tie.
 ***********************************************************************/
int whoWins(int p1, int p2) {
    if (p1 == p2) return 0;

    else if (p2 == BAD_MOVE ||
             p1 == SCISSORS && p2 == PAPER    ||
             p1 == PAPER   && p2 == ROCK     ||
             p1 == ROCK    && p2 == SCISSORS  )
        return -1;
    else 
        return 1;
}

/**********************************************************************
 * This will check if a player exited and if they did it will send
 * a the "BY" command to other players letting them know that the
 * game is over.
 ***********************************************************************/
char maybeEndGame(char const* buff, int p1fd, int p2fd) {
   if (compareTwo(buff, "EX")) {
      int n1, n2;

      n1 = write(p1fd, "BY", 3);
      if (n1 < 0) error("ERROR writing to socket p1\n");

      n2 = write(p2fd, "BY", 3);
      if (n2 < 0) error("ERROR writing to socket p2\n");
   }
}

/*****************************************************************
 * Will check p1 and p2 and write the responses to buff1 and buff2 
 ******************************************************************/
char returnMess(int p1, int p2, int p1fd, int p2fd) {
   int side = whoWins(p1, p2);
   printf("whoWins: %d\n", side);
   const void* wbuff = "YW";
   const void* lbuff = "YL";
   const void* tbuff = "YT";

   int n1, n2;

   if (side == 0) {
        printf("tie!\n");
        n1 = write(p1fd, tbuff, 3);
        n2 = write(p2fd, tbuff, 3);
   }
   else if (side == -1) {
        printf("player 1 wins\n");
        n1 = write(p1fd, wbuff, 3);
        n2 = write(p2fd, lbuff, 3);
   } else {
        printf("player 2 wins\n");
        n1 = write(p1fd,  lbuff, 3);
        n2 = write(p2fd,  wbuff, 3);
   }

   if (n1 < 0) error("ERROR writing to socket p1\n");
   if (n2 < 0) error("ERROR writing to socket p2\n");
}



/**********************************************************************
 * Old faithful main. It just starts things going.
 ***********************************************************************/
int main(int argc,  char *argv[]) {

    int sockfd,  p1fd,  p2fd,  portno;
    socklen_t clilen;
    char buffer1[BSIZE];
    char buffer2[BSIZE];
    struct sockaddr_in serv_addr,  cli_addr;
    int n1, n2;

    /*************************************
     * INIT
     *************************************/
    if (argc < 2) {
        fprintf(stderr, "ERROR,  no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET,  SOCK_STREAM,  0);

    if (sockfd < 0)
        error("ERROR opening socket\n");

    bzero((char *) &serv_addr,  sizeof(serv_addr));

    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd,  (struct sockaddr *) &serv_addr,
                sizeof(serv_addr)) < 0)
        error("ERROR on binding\n");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);


    printf("Waiting for a player 1...\n");
    p1fd = accept(sockfd,
            (struct sockaddr *) &cli_addr,
            &clilen);
    if (p1fd < 0)
        error("Error Trying to accept player 1!\n");

    printf("Waiting for a player 2...\n");
    p2fd = accept(sockfd,
            (struct sockaddr *) &cli_addr,
            &clilen);
    if (p2fd < 0)
        error("Error Trying to accept player 2!\n");


    int round = 0;

    /*************************************
     * MAIN LOOP
     *************************************/
    while(1) {
        printf( "\n------------- ROUND %d ------------\n", ++round);

        bzero(buffer1, BSIZE);
        bzero(buffer2, BSIZE);

        printf("Waiting for Player 1's Move...\n");
        n1 = read(p1fd, buffer1, BSIZE_LESS);
        if (n1 < 0) error("ERROR reading from socket (p1)\n");
        maybeEndGame(buffer1, p1fd, p2fd);

        printf("Waiting for Player 2's Move...\n");
        n2 = read(p2fd, buffer2, BSIZE_LESS);
        if (n2 < 0) error("ERROR reading from socket (p2)\n");
        maybeEndGame(buffer2, p1fd, p2fd);

        if(buffer1[0] == 'E' || buffer2[0] == 'E') {
            printf("Closing the connections\n");
            break;
        }

        printf("(1)-%s v.s %s-(2)\n", buffer1, buffer2);

        int p1 = toInt(buffer1);
        int p2 = toInt(buffer2);

        returnMess(p1, p2, p1fd, p2fd);

    }


    /*************************************
     * Close
     *************************************/
    close(p1fd);
    close(p2fd);

    close(sockfd);

    return 0;
}
