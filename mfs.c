/*
  Name: Aniv Surana
  ID: 1001912967
*/

// The MIT License (MIT)
// 
// Copyright (c) 2016 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE


#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>

// #define BLOCK_SIZE 1024
// #define NUM_BLOCKS 65536
// #define BLOCKS_PER_FILE 1024
// #define NUM_FILES 256

// uint8_t data[NUM_BLOCKS][BLOCK_SIZE];

// // directory
// struct directoryEntry
// {
//     char filename[64];
//     short in_use;
//     int32_t inode;
// };

// struct directoryEntry * directory;

// // inode
// struct inode
// {
//     int32_t blocks[BLOCKS_PER_FILE];
//     short in_use;
// };

// struct inode *inodes;
// FILE * fp;


#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports four arguments

#define MAX_HIST_ARGUMENTS 15   // History can contain no more than 15 commands
#define MAX_PIDS 15             // To hold the pids for the 15 history commands

// void init()
// {
//     directory = (struct directoryEntry*) &data[0][0];
//     inodes = (struct inode*)&data[20][0];
//     int i;
//     for(i = 0; i < NUM_FILES; i++)
//     {
//         directory[i].in_use = 0;
//         directory[i].inode = -1;
//         memset(directory[i].filename,0 , 64);
//         int j;
//         for(j = 0; j < NUM_BLOCKS; j++)
//         {
//             inodes[i].blocks[j] = -1;
//             inodes[i].in_use = 0;
//         }
//     }
// }

// void createfs(char *filename)
// {
//     fp = fopen(filename,"w");

//     memset(data, 0, NUM_BLOCKS * BLOCK_SIZE);

//     fclose(fp);
// }
int main()
{

    char * command_string = (char*) malloc( MAX_COMMAND_SIZE );
    
    // fp = NULL;

    // init();

    // history
    char *history[MAX_HIST_ARGUMENTS];
    int hist_index = 0;
    int hist_count = 0;

    // pids
    pid_t pid_list[MAX_PIDS];
    int pid_index = 0;
    int pid_count = 0;




    // allocate memory for each index of history
    for(int i = 0; i < MAX_HIST_ARGUMENTS; i++)
    {
        history[i] = (char*) malloc( MAX_COMMAND_SIZE );
    }

  while( 1 )
  {
    // Print out the msh prompt
    printf ("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (command_string, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
    {
      token[i] = NULL;
    }

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr = NULL;                                         
                                                           
    char *working_string  = strdup( command_string );                

    // we are going to move the working_string pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *head_ptr = working_string;
    int nf_count = 0;

    // Check if the first character in the command (in case of !)
    if(working_string[0] == '!')
    {
      int command_number = atoi(&working_string[1]);

      // checking if command_number corresponds to a valid command in history
      if(command_number < hist_count && command_number > -1)
      {
        strcpy(working_string,history[command_number]);
      }
      else
      {
        // not found in history and increments the counter variable
        printf("Command not in history.\n");
        nf_count++; 
      }
    }
    if (working_string[0] == '\n' || working_string[0] == ' ' || working_string[0] == '\t') 
    {
      continue;
    }
    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_string, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // if( strcmp( "createfs",token[0]) == 0)
    // {
    //     if(token[1] == NULL)
    //     {
    //         printf("ERROR: No filename specified\n");
    //         continue;
    //     }
    //     createfs(token[1]);
    // }
    

    // Now print the tokenized input as a debug check

    // int token_index  = 0;
    // for( token_index = 0; token_index < token_count; token_index ++ ) 
    // {
    //   printf("token[%d] = %s\n", token_index, token[token_index] );  
    // }

    /*


        My Shell's Functionality:
    
    
    */

    if(token[0] != NULL)
    {
      // duplicating the input string
      char *line = strdup(command_string);

      history[hist_index] = line;

      // incrementing each time an element is added to history
      hist_count++;
      hist_index++;



      // making sure number of arguments in history doesn't exceed the max
      if(hist_count > MAX_HIST_ARGUMENTS)
      {
        hist_count = 0;
      }
      // resetting history index
      if(hist_index > MAX_HIST_ARGUMENTS-1)
      {

        hist_index = 0;
      }

      // exit the shell if user enters quit or exit
      if(strcmp(token[0],"quit") == 0 || strcmp(token[0],"exit") == 0)
      {
        exit(0);
      }

      pid_t pid = fork();

      // storing pid of child process in list
      if(strcmp(token[0], "cd") == 0 || strcmp(token[0],"history") == 0)
      {
        pid_list[pid_index] = -1;
      }
      else
      {
        pid_list[pid_index] = pid;
      }
      pid_index++;
      pid_count++;


      // making sure number of pids doesn't exceed the max
      if(pid_index > MAX_PIDS - 1)
      {
        pid_index = 0;
      }
      if(pid_count > MAX_PIDS)
      {
        pid_count = MAX_PIDS;
      }

      // check for the parent process
      // return value for the parent is a non-zero value
      if(pid != 0)
      {
        if(strcmp(token[0],"cd") == 0)
        {
          // token[1] is the directory to be changed to
          chdir(token[1]);
        }
      }
      
      // check for the child process
      // child process will receive the value 0 when forked
      if(pid == 0)
      {
        int valid = execvp(token[0], &token[0]);
        if(valid == -1)
        {
          // output if the command cannot be executed
          if(strcmp(token[0],"history") != 0 && strcmp(token[0],"cd") != 0 && nf_count < 1)
          {
            printf("%s: Command not found.\n",token[0]);
          }
          break;
        }
      }

      else
      {
        // waiting for the child process to complete
        int status;
        wait(&status);
      }
    }
    
    // displaying the history
    if(strcmp(token[0],"history") == 0)
      {
        printf("\n");
        printf("HISTORY: \n");

        // flag variable to check if -p is entered by the user
        int print_pids = 0;
        if(token[1] != NULL && strcmp(token[1],"-p") == 0)
        {
          print_pids = 1;
        }
        // printing based on number of commands in history
        for(int i =0; i < hist_count; i++)
        {
          // eliminating the newline character to make it easier to print
          char *commandtoprint = strtok(history[i],"\n");

          if(print_pids == 1 && pid_list[i] != 0)
          {
            // printing pid in [] before the command
            printf("%2d. [%5d] %s",i,pid_list[i],commandtoprint);
          }
          else
          {
            printf("%2d. %s",i,history[i]);
          }
          printf("\n");
        }
        printf("\n");
      }

   

    // Cleanup allocated memory
    for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
    {
      if( token[i] != NULL )
      {
        free( token[i] );
      }
    }

    free( head_ptr );

  }

  free( command_string );

  // free memory allocated for history
  for(int i = 0; i < MAX_HIST_ARGUMENTS; i++)
  {
    free(history[i]);
  }

  return 0;
  // e2520ca2-76f3-90d6-0242ac120003
}