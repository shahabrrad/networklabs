#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdint.h>

typedef struct {
    uint8_t *array;
    int front;
    int rear;
    int size;
    int capacity;
} Queue;

Queue* createQueue(int capacity);
void destroyQueue(Queue* queue);

int isEmpty(Queue* queue);
int isFull(Queue* queue);

void display(Queue* queue);

void enqueue(Queue* queue, int item);
int dequeue(Queue* queue);
int front(Queue* queue);
int rear(Queue* queue);


#endif //QUEUE_H
