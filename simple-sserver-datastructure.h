/* The simple-sserver-datastructure will define commands which will store the
 * Input data that is read to two lists of data: compiling and testing.
 * It is a singly linked list with only a head node as the commands stored
 * will be exectued linearly every time. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Definition of the Node which createds the linked list to keep track of the
 * linear executed comments. Each node will have one command in it.
 */
typedef struct node {

  char *command;
  struct node *next;

} Node;

/*The linked list that contains two head nodes, one to keep track of the 
 * compile commands and one to keep track of the test commands
 */
typedef struct _Commands {

  struct node *compile_head;
  struct node *test_head;

} Commands;

