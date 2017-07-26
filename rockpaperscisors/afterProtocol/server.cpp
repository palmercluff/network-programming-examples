/***********************************************************************
* Program:
*    Lab RPSPserver, RPSP Rock/Paper/Scissors Protocol - Server Code
*    Brother Jones, CS 460
* Author:
*    Gage Peterson
* Summary:
*     A simple rock paper scisors server.
*     This sends single character commands, is persistant (doesn't close
*     at the end of the games) and adhere's to the protocol made in class.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

#define BSIZE 255

#define ROCK 0
#define PAPER 1
#define SCISSORS 2
#define QUIT 3
#define BAD_MOVE -1

void error(const char *msg) {
   perror(msg);
   exit(1);
}

/**********************************************************************
 * This is a helper method for comparing one character at the same time
 ***********************************************************************/
bool compareOne(char const* input, char const* against) {
   return tolower(input[0]) == tolower(against[0]);
}


/**********************************************************************
 * This converts the cstring command into an integer.
 ***********************************************************************/
int toInt(char buff[]) {
   if (compareOne(buff, "r")) return ROCK;
   if (compareOne(buff, "p")) return PAPER;
   if (compareOne(buff, "s")) return SCISSORS;
   if (compareOne(buff, "q")) return QUIT;
   else                       return BAD_MOVE;
}

/**********************************************************************
 * This is basically the Tic-tac toe logic. It tells who wins by
 * returning a -1 for p1 win, 1 for p2 win and a 0 for a tie.
 * bad moves always loose. One bad moves results in a tie.
 ***********************************************************************/
int whoWins(int p1, int p2) {
   if (p1 == p2) return 0;

   else if (p1 == SCISSORS && p2 == PAPER    ||
         p1 == PAPER   && p2 == ROCK     ||
         p1 == ROCK    && p2 == SCISSORS  )
      return -1;
   else 
      return 1;
}

/*****************************************************************
 * Will check p1 and p2 and write the responses to buff1 and buff2 
 *
 * Returns if the game should continue
 ******************************************************************/
bool returnMess(int p1, int p2, int p1fd, int p2fd) {
   int side = whoWins(p1, p2);
   const void* w = "w"; // win
   const void* l = "l"; // loss
   const void* t = "t"; // tie
   const void* x = "x"; // bad input from some player
   const void* d = "d"; // bad input from some player

   int n1, n2;

   if (p1 == QUIT || p2 == QUIT) {
      printf("One of the players quit the game!\n");
      n1 = send(p1fd, d, 1, 0);
      n2 = send(p2fd, d, 1, 0);

      return false; // end the game
   }
   else if (p1 == BAD_MOVE || p2 == BAD_MOVE) {
      printf("Bad input form one of the players...\n");
      n1 = send(p1fd, x, 1, 0);
      n2 = send(p2fd, x, 1, 0);
   }
   else if (side == 0) {
      printf("tie!\n");
      n1 = send(p1fd, t, 1, 0);
      n2 = send(p2fd, t, 1, 0);
   }
   else if (side == -1) {
      printf("player 1 wins\n");
      n1 = send(p1fd, w, 1, 0);
      n2 = send(p2fd, l, 1, 0);
   } else {
      printf("player 2 wins\n");
      n1 = send(p1fd,  l, 1, 0);
      n2 = send(p2fd,  w, 1, 0);
   }

   if (n1 < 0) error("ERROR writing to socket p1\n");
   if (n2 < 0) error("ERROR writing to socket p2\n");

   return true;
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

   printf("SERVER STARTED!\n\n");

   printf("    _______\n");
   printf("---'   ____)\n");
   printf("      (_____)\n");
   printf("      (_____)\n");
   printf("      (____)\n");
   printf("---.__(___)\n");
   printf("\n");
   printf("    _______\n");
   printf("---'   ____)____\n");
   printf("          ______)\n");
   printf("          _______)\n");
   printf("         _______)\n");
   printf("---.__________)\n");
   printf("\n");
   printf("    _______\n");
   printf("---'   ____)____\n");
   printf("          ______)\n");
   printf("       __________)\n");
   printf("      (____)\n");
   printf("---.__(___)\n");
   printf("\n");

   while(1) {
      printf("\n\n======= STARTING A NEW GAME ========\n");

      bool shouldContinue = true;

      // Player 1
      printf("Waiting for a player 1 to connect...\n");
      p1fd = accept(sockfd,
            (struct sockaddr *) &cli_addr,
            &clilen);
      if (p1fd < 0)
         error("Error Trying to accept player 1!\n");

      // Player 2
      printf("Waiting for a player 2 to connect...\n");
      p2fd = accept(sockfd,
            (struct sockaddr *) &cli_addr,
            &clilen);
      if (p2fd < 0)
         error("Error Trying to accept player 2!\n");


      int round = 0;

      /*************************************
       * MAIN LOOP
       *************************************/
      while(shouldContinue) {
         printf( "\n------------- ROUND %d ------------\n", ++round);

         bzero(buffer1, BSIZE);
         bzero(buffer2, BSIZE);

         printf("Waiting for Player 1's Move...\n");
         n1 = recv(p1fd, buffer1, BSIZE, 0);
         if (n1 < 0) error("ERROR reading from socket (p1)\n");

         printf("Waiting for Player 2's Move...\n");
         n2 = recv(p2fd, buffer2, BSIZE, 0);
         if (n2 < 0) error("ERROR reading from socket (p2)\n");


         printf("(1)-%d v.s %d-(2)\n", (int)buffer1[0], (int)buffer2[0]);

         int p1 = toInt(buffer1);
         int p2 = toInt(buffer2);

         shouldContinue = returnMess(p1, p2, p1fd, p2fd);
      }


      /*************************************
       * Close
       *************************************/
      close(p1fd);
      close(p2fd);
   }

   close(sockfd);

   return 0;
}
