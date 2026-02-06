#ifndef DSUTIL_H
#define DSUTIL_H

#include <stdlib.h>

#define STACK_FULL
#define STACK_APPEND_SUCCESS

// LIFO
typedef struct {
    void *data;
    size_t size;
    size_t capacity;
} Stack;

// TODO!
// Dynamic size stack
typedef struct {
    void *data;
    size_t size;
    size_t capacity;
} DStack;

int stack_append(Stack *s, void *item);
void *stack_pop(Stack *s);




#endif //DSUTIL_H;