#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

#include <sys/types.h>

enum NODE_TYPE {
    JOB,
    PROCESS
};

typedef struct node_s {
    enum NODE_TYPE node_type;
    void *node_ptr;
    struct node_s *next;
    struct node_s *previous;
} NODE;

typedef struct {
    int size;
    NODE *head;
    NODE *tail;
} LIST;

/**
* Initializes an empty list
*
* @return   the newly created list
*/
LIST* init_list();

/**
* Pushes a new node onto the list
*
* @param list   the list to push the node onto
* @param type   the type of the node that is being pushed
* @param data   the data to be stored in the node
*/
void push(LIST *list, enum NODE_TYPE type, void *data);

/**
* Removes the tail node of the list
*
* @param list   the list to pop the node from
* @return       the data stored within the tail node
*/
void* pop(LIST *list);

/**
* Gets the position of the node in the list
*
* @param list   the list to search
* @param node   the node to find
* @return       the position of the node within the list
*/
int get_pos_of_node(LIST *list, NODE *node);

/**
* Gets the data of the node at the specified position in the list
*
* @param list   the list to search
* @param pos    the position which to grab data
* @return       the data at the position
*/
void* get(LIST *list, int pos);

/**
* Deletes the specified node from the list
*
* @param list   the list that contains the node
* @param node   the node to delete
* @return       the data contained in the deleted node
*/
void* del_node(LIST *list, NODE *node);

/**
* Delete node based on position
*
* @param list   the list that contains the node
* @param pos    the position of the node to delete
* @return       the data contained in the deleted node
*/
void* del(LIST *list, int pos);

/**
* Free the memory taken up by the list
*
* @param list   the list to free
*/
void free_list(LIST *list);

#endif

