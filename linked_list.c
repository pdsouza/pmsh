#include <stdlib.h>
#include "linked_list.h"
#include "jobs.h"

/**
* Initializes an empty list
*
* @return   the newly created list
*/
LIST* init_list() {
    LIST *l = (LIST *)malloc(sizeof(LIST));
    l->size = 0;
    l->head = NULL;
    l->tail = NULL;
    return l;
}

/**
* Pushes a new node onto the list
*
* @param list   the list to push the node onto
* @param type   the type of the node that is being pushed
* @param data   the data to be stored in the node
*/
void push(LIST *list, enum NODE_TYPE type, void *data) {
    NODE *node = (NODE *)malloc(sizeof(NODE));
    node->node_type = type;
    node->node_ptr = data;

    if (list->size == 0) {
        node->previous = NULL;
        node->next = NULL;
        list->head = node;
    } else {
        node->previous = list->tail;
        node->next = NULL;
        list->tail->next = node;
    }

    list->tail = node;
    list->size++;
    return;
}

/**
* Removes the tail node of the list
*
* @param list   the list to pop the node from
* @return       the data stored within the tail node
*/
void* pop(LIST *list) {
    void *v = (void *)NULL;

    if (list->size > 1) {
        NODE *node = list->tail;
        list->tail = list->tail->previous;
        list->tail->next = NULL;
        node->next = NULL;
        node->previous = NULL;
        list->size--;

        v = node->node_ptr;
        free(node);
    } else if (list->size == 1) {
        NODE *node = list->tail;
        node->next = NULL;
        node->previous = NULL;
        list->size--;

        v = node->node_ptr;
        free(node);
        list->head = NULL;
        list->tail = NULL;
    }

    return v;
}

/**
* Gets the position of the node in the list
*
* @param list   the list to search
* @param node   the node to find
* @return       the position of the node within the list
*/
int get_pos_of_node(LIST *list, NODE *node) {
    int pos = 0;

    if (list->size > 0) {
        NODE *curr = list->head;

        while (curr != node) {
            if (curr->next != NULL) {
                curr = curr->next;
                pos++;
            } else {
                return -1;
            }
        }
    } else {
        return -1;
    }

    return pos;
}

/**
* Gets the data of the node at the specified position in the list
*
* @param list   the list to search
* @param pos    the position which to grab data
* @return       the data at the position
*/
void* get(LIST *list, int pos) {
    int i = 0;
    void *v = (void *)NULL;
    
    if (list->size > 0 && pos < list->size) {
        NODE *node = list->head;

        for (i = 0; i < pos; i++) {
            node = node->next;
        }
       
        v = node->node_ptr;
    }

    return v;
}

/**
* Gets the node at the specified position
*
* @param list   the list to search
* @param pos    the position to get
* @return       the node at the position
*/
static NODE* get_node(LIST *list, int pos) {
    int i = 0;
    
    if (list->size > 0 && pos < list->size) {
        NODE *node = list->head;

        for (i = 0; i < pos; i++) {
            node = node->next;
        }

        return node;
    } else {
        return (NODE *)NULL;
    }
}

/**
* Deletes the specified node from the list
*
* @param list   the list that contains the node
* @param node   the node to delete
* @return       the data contained in the deleted node
*/
void* del_node(LIST *list, NODE *node) {
    if (node != (NODE *)NULL) {
        if (list->size == 0) {
            return (void *)NULL;
        } else if (list->size == 1) {
            list->head = NULL;
            list->tail = NULL;
        } else {
            if (node->previous == NULL) {
                list->head = node->next;
                list->head->previous = node->previous;
            } else {
                node->previous->next = node->next;
            }

            if (node->next == NULL) {
                list->tail = node->previous;
                list->tail->next = node->next;
            } else {
                node->next->previous = node->previous;
            }
        }

        node->next = NULL;
        node->previous = NULL;
        list->size--;
        return (void *)node->node_ptr;
    } else {
        return (void *)NULL;
    }
}

/**
* Delete node based on position
*
* @param list   the list that contains the node
* @param pos    the position of the node to delete
* @return       the data contained in the deleted node
*/
void* del(LIST *list, int pos) {
    NODE *node = get_node(list, pos);
    void* v = del_node(list, node);
    free(node);
    return v;
}

/**
* Free the memory taken up by the list
*
* @param list   the list to free
*/
void free_list(LIST *list) {
    if (list->size == 0) {
        free(list);
        return;
    }

    NODE *node = list->head;

    while (node != NULL) {
        if (node->node_type == PROCESS) {
            free(node->node_ptr);
        } else if (node->node_type == JOB) {
            int i = 0;
            int a = 0;
            job *j = (job *)node->node_ptr;

            for (i = 0; i < j->numcmds; i++) {
                free(j->fin[i]);
                free(j->fout[i]);
                
                while (j->cmds[i][a] != (char *)NULL) {
                    free(j->cmds[i][a]);
                    a++;
                }
                a = 0;

                free(j->cmds[i]);
            }

            free(j->fin);
            free(j->fout);
            free(j->cmds);
            free_list(j->pid_list);
            free(j);
        }

        NODE *temp = node->next;
        free(node);
        node = temp;
    }

    free(list);
    return;
}

