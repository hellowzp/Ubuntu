#include <stdio.h>
#include <stdlib.h>
#include "myqueue.h"

#define DEBUG
#define QUEUE_INITIAL_CAPACITY 100

typedef unsigned int unint;
typedef int DATATYPE;

struct queue {
	unint front;
	unint rear;
	DATATYPE* data;
};


Queue* QueueCreate() {
	Queue *q = NULL;
	q = malloc(sizeof(Queue));
	q->front = q->rear = 0;
	q->data = malloc(QUEUE_INITIAL_CAPACITY * sizeof(DATATYPE));
	return q;
}

void Enqueue(Queue* queue, DATATYPE element) {
	queue->data[queue->rear] = element;
	queue->rear++;
}

void QueueDestroy(Queue** queue) {
	(*queue)->front = 0;
	(*queue)->rear = 0;
	free((*queue)->data);
	(*queue)->data = NULL;
	free(*queue);
	*queue=NULL;
}

int QueueSize(Queue* queue) {
	return queue->rear - queue->front;
}

DATATYPE* QueueTop(Queue* queue) {
	return &(queue->data[queue->front]);
}

void Dequeue(Queue* queue) {
	if(queue->rear > queue->front)
	    queue->front++;
	else printf("The queue is already empty..\n");
}

int main() {
	Queue* q = QueueCreate();
	Enqueue(q,12);
	printf("queue size:%d top:%d\n",QueueSize(q), *QueueTop(q));
	QueueDestroy(&q);
	return 0;
}



