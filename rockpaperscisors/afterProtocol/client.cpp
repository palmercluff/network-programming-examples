/***********************************************************************
* Program:
*    Lab RPSPclient, RPSP Rock/Paper/Scissors Protocol - Client Code
*    Brother Jones, CS 460
* Author:
*    Gage Peterson
* Summary:
*  Is a rock paper Scissors client that adheres to the protocol made in class
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

/**********************************************************************
 * This is a helper that writes an error out to the console and 
 * exits the program.
 ***********************************************************************/
void error(const char *msg)
{
  perror(msg);
  exit(0);
}

/**********************************************************************
 * This is a helper method for comparing one character at the same time
 ***********************************************************************/
bool compareOne(char const* input, char const* against) {
    return tolower(input[0]) == tolower(against[0]);
}

/**********************************************************************
 * This will print the results of who won and lost based on what the
 * server sent us.
 ***********************************************************************/
bool printResults(const char *buff) {
    if (compareOne(buff, "W")) {
      printf("\t:-D You won!\n");
      return false;
    }
    if (compareOne(buff, "L")) {
      printf("\t :-,( Dang! Lost that round\n");
      return false;
    }
    if (compareOne(buff, "T")) {
      printf("\t :-/ A tie!\n");
      return false;
    }
    if (compareOne(buff, "X")) {
      printf("You flick your hand in some weird motion that nobody understands. I don't think that round counts\n");
      return false;
    }
    else {
      printf("Game over.\n\n");
      return true;
    }
}

int main(int argc, char *argv[])
{
  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  /*************************************
   * Setup
   *************************************/
  char buffer[2];

  if (argc < 3) {
    fprintf(stderr,"usage %s hostname port\n", argv[0]);
    exit(0);
  }

  portno = atoi(argv[2]);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0)
    error("ERROR opening socket");

  server = gethostbyname(argv[1]);

  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
  }


  bzero((char *) &serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;

  bcopy((char *)server->h_addr,
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);

  serv_addr.sin_port = htons(portno);

  int con_status = connect(sockfd,
          (struct sockaddr *) &serv_addr,
          sizeof(serv_addr));

  if (con_status < 0)
    error("ERROR connecting");

  system("clear");
  printf("COMMANDS:\n");
  printf(" r - play Rock\n");
  printf(" p - play Paper\n");
  printf(" s - play Scissors\n");
  printf(" q - Quit the game\n");
  printf("\n");
  printf("**** NOTE: you only have to type one letter of each command, however more is ok too.\n");

  /*************************************
   * Procol (MAIN LOOP)
   *************************************/
  while(1) {


    printf("> ");

    bzero(buffer, 2);

    fgets(buffer, 256, stdin);


    if (n < 0)
      error("ERROR writing to socket");

    if (compareOne(buffer, "P")) {
       buffer[0] = 'p';
    } else if (compareOne(buffer, "S")) {
       buffer[0] = 's';
    } else if (compareOne(buffer, "R")) {
       buffer[0] = 'r';
    } else if (compareOne(buffer, "Q")) {
       buffer[0] = 'q';
       printf("You quit the game.\n");
    } else {
       printf("Invalid command. This round is invalid.\n");
    }

    printf("\n... Waiting for server...\n");

    n = send(sockfd, buffer, 1, 0);


    if (buffer[0] == 'q')
       break;

    bzero(buffer, 1);

    n = read(sockfd,buffer, 1);

    if (n < 0)
      error("ERROR reading from socket");

    if (printResults(buffer)) break;
  }

  /*************************************
   * Teardown
   *************************************/

  close(sockfd);

  return 0;
}
