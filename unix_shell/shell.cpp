/***********************************************************************
 * Program:
 *    Lab UnixShellT2, Operating Systems
 *    Brother Jones, CS 345
 * Author:
 *    Palmer Cluff
 * Summary:
 *    A C++ program that provides a Unix history feature. Able to run
 *    processes in the background. Stores the last 10 entered commands.
 *    This is the second submission.
 ************************************************************************
 *
 * Changes made to my code for the re-submission:
 *
 *   - I changed the file from C to C++ to make this lab easier for me.
 *   - I added some code that my peers had in their code that halped me
 *      solve my history feature problemss.
 *   - I completely removed my runCommand() function and did a lot of
 *      editing in main().
 *   - I imporved upon the style of my comments, including adding comments
 *      in places that I should.
 *
 *****************************************************************************/

/* Include the necessary libraries */
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <signal.h>
#include <cstring>
#include <unistd.h>
#include <wait.h>
#include <sstream>
#include <cstdlib>

/* 80 chars per line, per command, should be enough. */
#define MAX_LINE 80

/* Size of message printed in signal handler */
#define BUFFER_SIZE 50

/* The size of the history buffer */
#define HIST_SIZE 10

/* Maximum number of commands until count rolls over. */
#define MAX_NUM_CMDS 1000

using namespace std;

// A global array of strings and a count of the number of valid strings in it.
string commands[HIST_SIZE];
int goodStringCount = 0;
bool goodCmd = true;

/*************************************************************************
* Display the contents of the history buffer of commands with the number
* of the command listed on the beginning of the line.
**************************************************************************/
void displayHistory()
{
   int start;
   
   if (goodStringCount > HIST_SIZE)
      start = goodStringCount - HIST_SIZE;
   else
      start = 0;

   cout << "\n";
   for (int i = start; i < goodStringCount; ++i)
   {
        cout << " " << setw(3) << i << "  " << commands[i % HIST_SIZE] << endl;
   }
   
   return;
}

/*************************************************************************
 * Called when the user presses "^\" and calls displayHistory() then
 * setting goodCmd to false.
 ************************************************************************/
void handle_SIGQUIT( int junk )
{
   // Display the history
   displayHistory();
   goodCmd = false;
   return; 
}

/*************************************************************************
* add2History() adds a command to the history buffer and increments the
* number of commands that have been entered up to a maximum count.  If the
* history buffer is full, the oldest command is overwritten.    
**************************************************************************/
void add2History (string input)
{
   commands[goodStringCount % HIST_SIZE] = input;
   goodStringCount++;  

   // Roll over the count if it hits the maximum allowed for printing 
   if (goodStringCount >= MAX_NUM_CMDS)
      goodStringCount = HIST_SIZE;    /* Leave a full buffer of commands */
}
      
/*************************************************************************
* findCommandInHistory() searches backward for the first command that
* begins with a particular character (char c) in the history buffer.
* If a string is found that begins with the character, the string is
* passed back to the calling function.  If the character being looked
* for is a NULL then the most recent command is returned.  In both of
* the preceeding cases, the return value is set to true.  If no matching
* string is found, the return value is set to false.
**************************************************************************/
bool findCommnandInHistory(char c, string& stringFound)
{
   int temp = goodStringCount;
   stringFound = "";
   
   // If the character being looked for is a null, then return the most
   // recent command, if there was one.
   if (c == '\0')
   {
      if (goodStringCount)
      {
         stringFound = commands[ (goodStringCount - 1) % HIST_SIZE ];
         return true;
      }
      else
      {
         return false; 
      }
   }
   
   // Look back through the history for a command staring with the letter
   // given in the first argument to this function.
   for (temp; temp >= 0 && temp != temp - HIST_SIZE; --temp)
   {            
      if (commands[temp % HIST_SIZE][0] == c)
      {
         stringFound = commands[temp % HIST_SIZE];
         return true;
      }
   }
         
   return false;
}

/****************************************************************************
 * setup() reads in the next command line, separating it into distinct tokens
 * using whitespace as delimiters.
 *
 * setup() modifies the inputBuffer creating a set of null-terminated strings
 * and places pointers into the args[] array that point to the beginning of
 * each argument.  A NULL pointer is placed in the args[] array indicating
 * the end of the argument list.  This is what is needed for using execvp().
 *
 * A ^d input at the beginning of a line, by a user, exits the shell.
 ***************************************************************************/
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
            }
      }  /* end of switch */
   }  /* end of for loop looking at every character */
   args[ct] = NULL; /* just in case the input line was > 80 */
} 

/****************************************************************
 * This is the main function where everything starts. A signal
 * handler is set up first. Then from in a infinite loop the user
 * can continue to enter in commands to be executed in a process.
 * The commands are stored in a history buffer so that the user
 * can look back at which commands have been run.
 ****************************************************************/
int main(void)
{
   // For creating a new process
   char inputBuffer[MAX_LINE]; // Buffer to hold the command entered 
   int background;             // Equals 1 if a command is followed by '&' 
   char *args[MAX_LINE/2];     // Command line (of 80) has max of 40 arguments  
   int pid;                    // Process ID 

   // Set up the signal handler 
   struct sigaction handler;
   handler.sa_handler = handle_SIGQUIT;   // Function the handler will call
   handler.sa_flags = SA_RESTART;         /* Restarts when interup happens so
                                             that a -1 is not returned */
   sigaction(SIGQUIT, &handler, NULL);

   // These varables are used with the history feature
   string tmpString;
   int length = -1;
   string input;
   
   // Program terminates normally inside setup 
   while (1)                  
   {
      length = -1;
      goodCmd = true;

      // Stay here until a command is entered
      while(length < 0)
      {
         background = 0;
         printf(" COMMAND-> ");
         fflush(stdout);

         // Get next command 
         setup(inputBuffer, args, &background); 
         
         // Create child
         pid = fork();

         if (pid < 0) // Error occurred 
         {
            printf ("Fork Failed\n");
            return 1;
         }
         else if (pid == 0)
         {
            // Execute command
            execvp(args[0], args);

            // Should not get to this point if the command is valid

            // Convert char to string for comparison
            stringstream ss;
            string s;
            ss << args[0];
            ss >> s;

            // Don't print this if a 'r' is entered
            if (s != "r") 
            {
               printf("command not found\n");
            }
            exit(1);  
         }
         if (background != 1) // Wait until child is complete
         {
            waitpid(pid, NULL, 0);
         }

         // Get the command/input to add to history
         input = "";
         bool firstTime = true;
         int j = 0;
         for (char *i = args[j]; args[j] != '\0'; j++)
         {
            if (firstTime)
            {
               input += args[j];
               firstTime = false;
            }
            else
            {
               input = input + " " + args[j];
            }
         }
         
         // Get command length
         length = input.length();
         
         // removes the newline and makes this a c-string 
         inputBuffer[length] = '\0';
      }
              
      if (length == 0)
      {
         // If the user just entered a newline, then the length of the
         // input will be 0 and we just want to give the prompt again.
         // (Ignoring the newline input.)
         goodCmd = false;
      }
      // Handles the case when an 'r' is entered
      else if (inputBuffer[0] == 'r' &&
               (inputBuffer[1] == '\0' || inputBuffer[1] == ' ') )
      {         
         // If an 'r' was given with no argument the most recent command
         // should be executed.  This is indicated with a null.

         // If just 'r' is entered  
         if (length == 1)
         {
            inputBuffer[2] = '\0';
         }
         
         // If an 'r' command was given and a matching command is found in the
         // history, then put the command to be repeated in the buffer.
         goodCmd = findCommnandInHistory(inputBuffer[2], tmpString);

         // Matching command  
         if (goodCmd)
         {
            // Copy inputBuffer to tmpString
            strncpy (inputBuffer, tmpString.c_str(), MAX_LINE);           

            // Execute command and add it to history
            system(tmpString.c_str());
            add2History(tmpString.c_str());
            goodCmd = false;
         }
         else // No matching command
         {
            goodCmd = false;
            cout << "No matching command in history\n";
            cout.flush();   
         }
      }

      // Add the command to the history
      if (goodCmd)
      {
         add2History(input); 
      }
   } // end While(1)
}
