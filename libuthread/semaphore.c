#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#define _UTHREAD_PRIVATE
#include "preempt.h"
#include "queue.h"
#include "semaphore.h"
#include "uthread.h"

// semaphore struct
// open to store the number of open resources
// to strore the waiting thread in a semaphore
struct semaphore {
	int open;
	queue_t waiting;
};

// we initialize that and return a pointer to that semaphore
sem_t sem_create(size_t count)
{
	preempt_disable();
	if(count < 0) // making sure not 0 rources are allocated to this semaphore
		return NULL;
	sem_t sem = malloc(sizeof(struct semaphore)); // allocating data a semaphore

	sem->waiting = queue_create(); 
	sem->open = count; // allocating resources for a semaphote
	preempt_enable();
	return sem;
}

int sem_destroy(sem_t sem)
{
	if(!sem)
		return -1;
	queue_destroy(sem->waiting);
	free(sem);
	return 0;
}

int sem_down(sem_t sem)
{
	preempt_disable();
	if(!sem) // if semaphore doesn't exist
		return -1;

	if(sem->open > 0) //Allow resource to be taken
	{
		sem->open--;
		return 0;
	}

	// if resources not present that we block the current thread and enque it in 
	// the waiting queue
	queue_enqueue(sem->waiting, uthread_current());
	uthread_block();
	preempt_enable();	
	
	//Starts running again here when unblocked with access to resource
	return 0;
	//Returns if resource can be taken, or if thread should be blocked	
}

int sem_up(sem_t sem)
{
	if(!sem)
		return -1;
	
	void *top;

	preempt_disable();	
	if(queue_dequeue(sem->waiting, (void**)&top) < 0) //Pop top of waiting list
		sem->open++; //If queue was empty create open slot
	else	
		uthread_unblock(top); //Add next in line back to the ready_q
	preempt_enable();
	return 0;

}

