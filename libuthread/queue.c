#include <stdint.h>
#include <stdlib.h>

#include "queue.h"

typedef struct queueNode
{
	struct queueNode* next;
	void* data;

} queueNode;

typedef struct queue {
	queueNode* head;
	queueNode* tail;
	int size;
} queue;

queue_t queue_create(void)
{
	queue_t q = malloc(sizeof(struct queue));
	q->head = NULL;
	q->tail = NULL;
	q->size = 0;
	return q;
}

int queue_destroy(queue_t queue)
{
	if(!queue)
		return -1;
	if(queue->size)
		return -1; //According to piazza queue should be empty
	free(queue);
	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
	if(!queue || !data)
		return -1;
	queueNode* newQ = malloc(sizeof(struct queueNode));
	newQ->data = data;
	newQ->next = NULL;

	if(queue->tail) //Non-empty queue
		queue->tail->next = newQ;
	else
		queue->head = newQ;

	queue->tail = newQ;

	queue->size++;
	
	return 1;
}

int queue_dequeue(queue_t queue, void **data)
{
	if (!(queue->head))
		return -1;
	void* ptr = (queue->head->data);
	*data = ptr;
	queueNode *tmp = queue->head->next;
	free(queue->head);
	queue->head = tmp;	
	queue->size--;
	if(!queue->head)
		queue->tail = NULL;	
	return 0;
}

int queue_delete(queue_t queue, void *data)
{
	queueNode *itr = queue->head, *toFree;
	if(queue->head->data == data)
	{
		toFree = queue->head;
		queue->head = queue->head->next;
		free(toFree->data);	
		free(toFree);
		queue->size--;
		return 0; //Head is removed from queue	
	}
	while(itr->next)
	{
		if(itr->next->data == data) //found it
		{
			free(itr->next->data); //Delete data
			itr->next = itr->next->next; //Skip node

			if(!itr->next)
				queue->tail = itr; //We deleted tail
			queue->size--;
			return 0;

		}
		
		itr = itr->next;
	}
	
	return -1;
}

int queue_iterate(queue_t queue, queue_func_t func)
{
	queueNode* itr = queue->head, *prev;
			
	while(itr)
	{
		func(itr->data);
		if(!itr->data)
		{
			if(prev)
				prev->next = itr->next;
			else
				queue->head = itr->next;
			free(itr);			
	
		}
		prev = itr;
		itr = itr->next;
	}

	return 0;
}

int queue_length(queue_t queue)
{
	if(queue)	
		return queue->size;
	return -1;
}

