/***********************************************************************
* Program:
*    Lab UnixShellT2, Operating Systems
*    Brother Jones, CS 345
* Author:
*    Gage Peterson
* Summary:
*    This is a tiny unix shell made for my operating systems class
*****************************************************************************
* Changes made to my code for the re-submission:
*   - Fixed the `r` command
*   - Made the history better conform it example program
*****************************************************************************/


#include <signal.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define BUFFER_SIZE 255
#define HIST_SIZE 10 // how large the history for the shell should be

using namespace std;

vector<string> history;


void printPrompt()
{
   const char* prompt = "λ > ";
   write(STDOUT_FILENO, prompt, strlen(prompt));
   fflush(stdout);
}


/***********************************************************************
 * Prints the history of the commands
 ***********************************************************************/
void print_hist()
{
   const char* message = "\nhistory\n---------------\n";
   write(STDOUT_FILENO, message, strlen(message));

   int hsize = history.size();
   for (int i = min(HIST_SIZE, hsize); i > 0; i--) 
   {

      char temp[255];
      sprintf(temp, " %d. %s", hsize - i, history.at(hsize - i).c_str());

      // writes needed because it's in a signal handler :P
      write(STDOUT_FILENO, temp, strlen(temp));
   }

   printPrompt();
}

void handle_SIGQUIT( int junk )
{
   if (history.size() > 0) {
      print_hist();
   } else {
      const char* message = "Sorry, no history...\n  > ";
      write(STDOUT_FILENO, message, strlen(message));
   }
}

/**
 * setup() reads in the next command line, separating it into distinct tokens
 * using whitespace as delimiters.
 *
 * setup() modifies the inputBuffer creating a set of null-terminated strings
 * and places pointers into the args[] array that point to the beginning of
 * each argument.  A NULL pointer is placed in the args[] array indicating
 * the end of the argument list.  This is what is needed for using execvp().
 *
 * A ^d input at the beginning of a line, by a user, exits the shell.
 */

void setup(char inputBuffer[], char *args[], int *background)
{
   int length; /* # of characters in the command line */
   int i;      /* loop index for accessing inputBuffer array */
   int start;  /* index where beginning of next command parameter is */
   int ct;     /* index of where to place the next parameter into args[] */

   ct = 0;

   /* read what the user enters on the command line */
   length = read(STDIN_FILENO, inputBuffer, MAX_LINE);  


   start = -1;            /* Used as a flag to indicate that we are looking
                           * for the start of the command or an argument if
                           * it is a -1, or stores the starting position of
                           * the command or argument to be put in args[].
                           */
   if (length == 0)
      exit(0);            /* ^d was entered, end of user command stream */
   if (length < 0)
   {
      perror("error reading the command");
      exit(-1);           /* terminate with error code of -1 */
   }


   if ('r' == inputBuffer[0]) {
      bool redid = false;

      if (inputBuffer[1] == '\n') {
         // redo last command
         string histItem = *(history.end()-1);

         strncpy(inputBuffer,
               histItem.c_str(),
               histItem.length());

         length = histItem.length();

         inputBuffer[histItem.length()-1] = '\n';
         inputBuffer[histItem.length()] = '\0';

         redid = true;
         write(STDOUT_FILENO, inputBuffer, length);
      } else {


         char searchLetter = inputBuffer[2];

         for (int i = history.size() - 1; i >= 0; i--) 
         {
            // look for the letter to redo
            if (history[i][0] == searchLetter) {

               // redo
               string histItem = history.at(i);

               strncpy(inputBuffer,
                     histItem.c_str(),
                     histItem.length());

               length = histItem.length();

               inputBuffer[histItem.length()-1] = '\n';
               inputBuffer[histItem.length()] = '\0';

               redid = true;
               write(STDOUT_FILENO, inputBuffer, length);
               break;
            }
         }
      }

      if (redid == false) {
         printf("Nothing in history starting with that letter.\n");
      }
   }

   string ib = inputBuffer;
   ib[length] = '\0';
   history.push_back(ib);

   /* examine every character in the inputBuffer */
   for (i=0;i<length;i++)
   { 
      switch (inputBuffer[i])
      {
         case ' ':
         case '\t' :               /* argument separators */
            if(start != -1)        /* found the end of the command or arg */
            {
               args[ct] = &inputBuffer[start]; /* set up pointer in args[] */
               ct++;
            }
            inputBuffer[i] = '\0'; /* add a null char; make a C string */
            start = -1;
            break;

         case '\n':                 /* should be the final char examined */
            if (start != -1)        /* if in the command or an argument */
            {
               args[ct] = &inputBuffer[start]; /* set up pointer in args[] */ 
               ct++;
            }
            inputBuffer[i] = '\0';
            args[ct] = NULL; /* no more arguments to this command */
            break;

         default :             /* some other character */
            if (start == -1 && inputBuffer[i] != '&')
               start = i;      /* starting position of the command or arg */
            if (inputBuffer[i] == '&')
            {
               *background  = 1;
               inputBuffer[i] = '\0';
            } else {
               *background = 0;
            }
      }  /* end of switch */
   }  /* end of for loop looking at every character */
   args[ct] = NULL; /* just in case the input line was > 80 */
} 




int main(void)
{
   char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
   int background;             /* equals 1 if a command is followed by '&' */
   char *args[MAX_LINE/2];   /* command line (of 80) has max of 40 arguments */

   struct sigaction handler;
   handler.sa_handler = handle_SIGQUIT;   /* function the handler will call */
   handler.sa_flags = SA_RESTART;
   sigaction(SIGQUIT, &handler, NULL);    /* handler to call for quit signal */

   while (1)                   /* Program terminates normally inside setup */
   {
      background = 0;
      printPrompt();


      setup(inputBuffer, args, &background);       /* get next command */

      int pid = fork();

      if (pid == -1) { // ERROR forking the process
         printf("Error forking the process. Maybe we should have used a spoon...\n");
         break; // quit
      }

      if (pid == 0) { // CHILD process
         int error = execvp(args[0], args) == -1;

         if (error) {
            printf("Sorry, I couldn't find: `%s` Maybe a typo?\n", args[0]);
         } 

         break; // quit

      } else  { // PARENT process


         // Wait or continue based on background
         if (background != 1) {
            waitpid(pid, NULL, 0);
         }

      }
   }
}

