/****************************************************************************
* Program:
*    Lab RockClientT2, Rock/Paper/Scissors with Sockets - Client Code
*    Brother Jones, CS 460
* Author:
*    Your Name
* Summary:
* - Changes made to my code for the re-submission:
*   - Added some comments
*   - Made interface only need the first letter of each command
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
    return toupper(input[0]) == against[0];
}

/**********************************************************************
 * This is a helper method for comparing two characters at the same time
 * This is necicary because I send 2-character messages.
 ***********************************************************************/
bool compareTwo(char const* input, char const* against) {
    return toupper(input[0]) == against[0] &&
           toupper(input[1]) == against[1];
}

/**********************************************************************
 * This will print the results of who won and lost based on what the
 * server sent us.
 ***********************************************************************/
bool printResults(const char *buff) {
    if (compareTwo(buff, "YW")) {
      printf("\t:-D You won!\n");
      return false;
    }
    if (compareTwo(buff, "YL")) {
      printf("\t :-,( Dang! Lost that round\n");
      return false;
    }
    if (compareTwo(buff, "YT")) {
      printf("\t :-/ A tie!\n");
      return false;
    }
    else {
      printf("Other Player Quit\n", buff);
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
  char buffer[256];

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
  printf(" e - Exit the game\n");
  printf("\n");
  printf("**** NOTE: You only have to type the first letter of each command.\n");

  /*************************************
   * Procol (MAIN LOOP)
   *************************************/
  while(1) {


    printf("> ");

    bzero(buffer, 3);

    fgets(buffer, 255, stdin);


    if (n < 0)
      error("ERROR writing to socket");

    if (compareOne(buffer, "P")) {
       strcpy(buffer, "PA");
    } else if (compareOne(buffer, "S")) {
       strcpy(buffer, "SC");
    } else if (compareOne(buffer, "R")) {
       strcpy(buffer, "RO");
    } else if (compareOne(buffer, "E") || compareOne(buffer, "Q")) {
       strcpy(buffer, "EX");
       printf("You quit the game.\n");
    } else {
       printf("Invalid Command, this Means you forfit your turn.\n");
    }

    n = write(sockfd, buffer, strlen(buffer));

    bzero(buffer, 3);

    n = read(sockfd,buffer,3);

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
