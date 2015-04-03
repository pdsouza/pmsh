#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../linked_list.h"

#define NUM_NODES 10

int main(int argc, char **argv) {
    //init_list tests
    LIST *l = init_list();
    assert(l->size == 0);
    assert(l->head == NULL);
    assert(l->tail == NULL);

    //push tests
    int i;

    for (i = 0; i < NUM_NODES; i++) {
        push(l, (i % 2 == 0) ? JOB : PROCESS, (void *)&i);
    }
    assert(l->size == NUM_NODES);
    assert(l->head != NULL);
    assert(l->tail != NULL);

    //get_pos_of_node tests
    assert(get_pos_of_node(l, l->head) == 0);
    assert(get_pos_of_node(l, l->tail) == NUM_NODES - 1);

    //get tests
    assert(get(l, 0) == l->head->node_ptr);
    assert(get(l, NUM_NODES - 1) == l->tail->node_ptr);

    //del tests
    void *h = l->head->node_ptr;
    void *n1 = del(l, 0);
    assert(n1 == h);
    assert(l->size == NUM_NODES - 1);

    void *t = l->tail->node_ptr;
    void *n2 = del(l, NUM_NODES - 2);
    assert(n2 == t);
    assert(l->size == NUM_NODES - 2);

    void *h2 = l->head->next->node_ptr;
    void *n3 = del(l, 1);
    assert(n3 == h2);
    assert(l->size == NUM_NODES - 3);

    //pop tests
    void *t1 = l->tail->node_ptr;
    void *p1 = pop(l);
    assert(p1 == t1);
    assert(l->size == NUM_NODES - 4);
    
    void *t2 = l->tail->node_ptr;
    void *p2 = pop(l);
    assert(p2 == t2);
    assert(l->size == NUM_NODES - 5);
    
    void *t3 = l->tail->node_ptr;
    void *p3 = pop(l);
    assert(p3 == t3);
    assert(l->size == NUM_NODES - 6);

    void *t4 = l->tail->node_ptr;
    void *p4 = pop(l);
    assert(p4 == t4);
    assert(l->size == NUM_NODES - 7);

    void *t5 = l->tail->node_ptr;
    void *p5 = pop(l);
    assert(p5 == t5);
    assert(l->size == NUM_NODES - 8);

    void *t6 = l->tail->node_ptr;
    void *p6 = pop(l);
    assert(p6 == t6);
    assert(l->size == NUM_NODES - 9);

    void *d = l->head->node_ptr;
    void *a = del(l, 0);
    assert(a == d);
    assert(l->size == 0);
    assert(del(l, 0) == (void *)NULL);

    printf("All tests pass!\n");
    return 0;
}

