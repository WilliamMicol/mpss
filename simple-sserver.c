/* This code defines the function for which the simple submit server will run
 *given the Commands data structure defined in simple-sserver-datastructure.h.
 * The goal is to test possible code submited by students, therefore there
 * will be compiling functions for code that requires compiling and testing 
 * functions for testing the compiled code. 
 * Summary of functions:
 *
 * read_commands(): will read and store a list of compile commands from one 
 * file and a list of testing commands from another file inside the Commands
 * data strucutre defined.
 *
 * clear_commands(): will free all dynamically-allocated memory used by the 
 * Commands data strucutre stored using read_commands()
 *
 * compile_program(): will execute the commands of the first list, compiling
 * the student code
 *
 * test_program(): will execute all the commands of the second list, testing 
 * the student code
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "simple-sserver.h"
#include "safe-fork.h"
#include "split.h"
#include <sys/stat.h>
#include <fcntl.h>

static Node *new_node(char command[]);

/* Helper function to create a new node using the command pulled from the file
 * and assigning it to a dynamically allocated char array. It assigns the 
 * next of the new node to NULL.
 * input: string of command
 *
 * output: new Node
 *          NULL if input is NULL
 */
static Node *new_node(char command[]){

  Node *new = NULL;
  new = malloc(sizeof(Node));

  if (command != NULL){
    
    new->command = malloc(strlen(command)+1);
    strcpy(new->command,command);
    new->next = NULL;
    
  }

  return new;

}

/*This function will read two list of commands, the first being the compiling
 * commands for the student test code, and the second being the testing 
 * commands for the student test code. These files will contain a list of 
 * commands. A command is a string that can contain multiple words and
 * possibly two special symbols. If the name passed into the parameters is 
 * NULL or is the name of a file that doesnt exist, then the program will quit
 zx* with an exit status of 1 and not do anything. No command is longer than 
 * 255 characters long. 
 */
Commands read_commands(const char compile_cmds[], const char test_cmds[]){
  Commands new;
  FILE *comp = NULL;
  FILE *test = NULL;
  char string_commands[LINE_MAX];
  Node *comp_node = NULL;
  Node *test_node = NULL;

  new.compile_head = NULL;
  new.test_head = NULL;
  if (compile_cmds == NULL || test_cmds == NULL){
  
    exit(1);

  } else {

    /*only read commands if both files exist*/
    if ((comp = fopen(compile_cmds, "r")) != NULL 
          && (test = fopen(test_cmds, "r")) != NULL){

      comp_node = new.compile_head;
      test_node = new.test_head;
      
      /*Getting each command from the compile list and storing it in a node*/
      while (fgets(string_commands, LINE_MAX, comp) != NULL){


        if (comp_node == NULL){ /*If first node*/

          comp_node = new_node(string_commands);
          new.compile_head = comp_node;

        } else {

          comp_node->next = new_node(string_commands);
          comp_node = comp_node->next;

        }
      }
       
      /*Getting each command from the test list and storing it in a node */ 
      while (fgets(string_commands, LINE_MAX, test) != NULL){

        if (test_node == NULL){ /*If first node*/

          test_node = new_node(string_commands);
          new.test_head = test_node;

        } else {

          test_node->next = new_node(string_commands);
          test_node = test_node->next;

        }
      }

      /*clean up files*/
      fclose(comp);
      fclose(test);

    } else 
      exit(1);

  }

  return new; 

}

/*this function deallocateds all dynamically-allocated memory used by the 
 * Commands data structure passed in as the parameter. If the parameter is 
 * NULL then the function will return without doing anything. 
 */
void clear_commands(Commands *const commands){
  if (commands != NULL){
    
    Node *tracker;
    Node *temp;
    /*free compile list*/

    tracker = commands->compile_head;
    while (tracker != NULL){

      temp = tracker;
      tracker = tracker->next;
      temp->next = NULL;
      free(temp->command);
      free(temp);
      
    }

    /*free test list*/
    tracker = commands->test_head;
  
    while (tracker != NULL){
    
      temp = tracker;
      tracker = tracker->next;
      temp->next = NULL;
      free(temp->command);
      free(temp);

    }
    

  }
  return;
}

/*This function should execute the compilation commands that are stored in
 * the Commands data structure that is passed in as the parameter. Commands 
 * in the data structure will be executed in the order with which the appear
 * in the file. If the commands execute successfully, then it will return 
 * SUCCESSFUL_COMPILATION. If there are errors, then it should stop executing 
 * commands and return FAILED_COMPILATION. These are defined in 
 * simple-sserver.h. If there are no compilation commands then it will return
 * SUCCESSFUL_COMPILATION. 
 */

int compile_program(Commands commands){
  
  char **the_commands;
  int compilation = 0;
  int error_checker = 0;
  int input;
  int output;
  int i;
  
  Node *tracker = commands.compile_head;

  while (tracker != NULL){

    pid_t pid;
    int status;
    pid = safe_fork();
  
    if (pid > 0){

      wait(&status);

      /*if any exit status other than 0*/
      if (WEXITSTATUS(status) != 0){
        error_checker = 1;
      }
  
    } else {
      
      if (pid == 0){
        the_commands = split(tracker->command);
        i = 0;
        while (the_commands[i] != NULL && strcmp(the_commands[i], "\0") != 0){

          if (strcmp(the_commands[i],"<") == 0){
            /*handling case of input redirction*/
            input = open(the_commands[i+1], O_RDONLY);
            dup2(input, STDIN_FILENO);

            free(the_commands[i]);
            the_commands[i] = NULL;
            free(the_commands[i+1]);
            the_commands[i+1] = NULL;

            i+=1;
          } else if (strcmp(the_commands[i], ">") == 0){
            /*handling case of output redirection*/
            output = open(the_commands[i+1], (FILE_FLAGS), (FILE_MODE));
            dup2(output, STDOUT_FILENO);

            free(the_commands[i]);
            the_commands[i] = NULL;
            free(the_commands[i+1]);
            the_commands[i+1] = NULL;

            i+=1;
          }
          i+=1;
        }
        execvp(the_commands[0], the_commands);
        free(the_commands);
      }
  
    }
    
    /*If an error has occured, then leave the loop*/
    if (error_checker == 1){
      tracker = NULL;
    } else{
      tracker = tracker->next;
    }

  }
  
  /*based on the error_checker variable set on an error, set the return value
 */

  if (error_checker == 0)
    compilation = SUCCESSFUL_COMPILATION;
  else
    compilation = FAILED_COMPILATION;


  return compilation;
}

/*This function should execute the list of test commands in the Commands data
 * sturcture passed as the parameter in order as which it appears. The return
 * value of this function will be the amount of tests properly run with the
 * compiled student test code.
 */

int test_program(Commands commands){

  char **the_commands;
  int tests_passed = 0;
  int i;
  int input;
  int output;
 
  /*Starting from the test list head*/

  Node *tracker = commands.test_head;

  while (tracker != NULL){

    pid_t pid;
    pid = safe_fork();

    if (pid > 0){
      
      int status;
      wait(&status);

      if (status == 0){
        tests_passed++;
      } 

    } else {

      if (pid == 0){
        the_commands = split(tracker->command);
        i = 0;
        while (the_commands[i] != NULL && strcmp(the_commands[i],"\0") != 0){

          /*The case of input redirection*/ 
          if (strcmp(the_commands[i],"<") == 0){

            input = open(the_commands[i+1], O_RDONLY);
            dup2(input, STDIN_FILENO);

            /*Setting the freed strings to NULL allows the exec command
             *to work the same as it acts as the end of the parameter list*/
            free(the_commands[i]);
            the_commands[i] = NULL;
            free(the_commands[i+1]);
            the_commands[i+1] = NULL;

            i+=1;

          } else if (strcmp(the_commands[i], ">") == 0){
            /* The case of output redirection*/
            output = open(the_commands[i+1], (FILE_FLAGS), (FILE_MODE));
            dup2(output, STDOUT_FILENO);

            free(the_commands[i]);
            the_commands[i] = NULL;
            free(the_commands[i+1]);
            the_commands[i+1] = NULL;

            /*necessary to increase by 1 incase there is input redirection*/
            i+=1;
          }

          i+=1;
        }
        
        /*Use the same child to execute the commands but the input/output
         * is changed based on the character
         */ 
        execvp(the_commands[0], the_commands);
        free(the_commands);
      }

    }

    tracker = tracker->next;

  }

  return tests_passed;


}
