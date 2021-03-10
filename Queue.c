#include <stdio.h>
#include <stdlib.h>

#include "Queue.h"

void init_q(struct Queue *q) {
    q->list = malloc(sizeof(struct LinkedList));
    if (q->list == NULL) {
        perror("Not enough memory to initialize the queue!");
        return;
    }

    init_list(q->list);
}

int get_size_q(struct Queue *q) {
    /* TODO */
    return q->list->size;
}

int is_empty_q(struct Queue *q) {
    /* TODO */
    if(q->list->size == 0)
    	return 1;
    return 0;
}

void* front(struct Queue *q) {
    /* TODO */
    if(q->list->head == NULL)
    	return NULL;
    return q->list->head->data;
}

void dequeue(struct Queue *q) {
    /* TODO */
    struct Node *first;
    first = remove_first(q->list);
    free(first);
}

void dequeue_improved(struct Queue *q) {
	struct Node *first;
    first = remove_first(q->list);
    free(first->data);
    free(first);
}

void enqueue(struct Queue *q, void *new_data) {
    /* TODO */
    add_last(q->list, new_data);
}

void enqueue_improved(struct Queue *q, void *new_data, int new_data_bytes) {
	add_last_improved(q->list, new_data, new_data_bytes);
}

void clear_q(struct Queue *q) {
    /* TODO */
    while(q->list->size > 0)
		dequeue(q);
}

void purge_q(struct Queue *q) {
    /* TODO */
    free_list(q->list);
    free(q);
}

void purge_q_improved(struct Queue *q) {
	free_list_improved(q->list);
    free(q);
}
