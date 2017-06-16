#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define _UTHREAD_PRIVATE
#include "context.h"
#include "palloc.h"
#include "preempt.h"
#include "queue.h"
#include "tls.h"
#include "uthread.h"

// declaring variable for thread state
#define RUNNING 2
#define READY 1
#define BLOCKED 0

// global variable to store tcb in a queue
queue_t ready_q; 

// global index variable to intialize t_id
int t_id_index;

typedef struct uthread_tcb {
	int state;
    	int t_id;
    	uthread_ctx_t *uctx;
   	void *top_of_stack; 
	void *tlsb; //Tls block data structure
}uthread_tcb;

// global initial uthread_tcb
uthread_tcb *active_tcb; 

void uthread_yield(void)
{
	preempt_disable();
	uthread_tcb *utcb = active_tcb; //Temporarily save active pointer  

	queue_enqueue(ready_q, utcb); //Put current thread back on queue

	if (queue_dequeue(ready_q, (void **)&active_tcb) < 0) //Pop top of queue into active 
		exit(1);

	while(active_tcb->state == BLOCKED) //Ignore blocked threads
	{
		queue_enqueue(ready_q, active_tcb);
		queue_dequeue(ready_q, (void **)&active_tcb);
	}
	tls_close();
	uthread_ctx_switch(utcb->uctx, active_tcb->uctx); //Context switch
	tls_open();
	preempt_enable();
	
}

int uthread_create(uthread_func_t func, void *arg)
{
	preempt_disable();
	uthread_tcb *utcb = malloc(sizeof(uthread_tcb));

	queue_enqueue(ready_q, utcb);

	// initialize the thread control block
	utcb->state = READY;
	utcb->t_id = ++t_id_index;
	utcb->top_of_stack = uthread_ctx_alloc_stack();
	utcb->uctx = malloc(sizeof(uthread_ctx_t));
	
	//Add tls block to uthread tcb 

	return uthread_ctx_init(utcb->uctx, utcb->top_of_stack, func, arg);
}

void uthread_exit(void)
{
	preempt_disable();
	uthread_tcb *utcb = active_tcb; //Temporarily save active pointer  
	if(queue_dequeue(ready_q, (void**)&active_tcb) < 0)
		return;
	
	uthread_ctx_switch(utcb->uctx, active_tcb->uctx); //Context switch
	preempt_enable();
}

void uthread_block(void)
{
	preempt_disable();
	uthread_tcb *utcb = active_tcb; //Temporarily save active pointer  
	active_tcb->state = BLOCKED; //Semaphore will save this tcb in own thread
	//uthread_yield(); //Might need to change this so that we dont add the active to the readyq

	if (queue_dequeue(ready_q, (void **)&active_tcb) < 0) //Pop top of queue into active
		exit(1);
	uthread_ctx_switch(utcb->uctx, active_tcb->uctx); //Context switch
	preempt_enable();
	return;
}

void uthread_unblock(struct uthread_tcb *uthread)
{
	preempt_disable();
	uthread->state = READY;
	queue_enqueue(ready_q, uthread); //Adds uthread back to running queue with a ready status
	preempt_enable();
}

struct uthread_tcb *uthread_current(void)
{	
	return active_tcb;
}

void *uthread_get_tls(void)
{
	return active_tcb->tlsb;
}

void uthread_set_tls(void *tls)
{
	active_tcb->tlsb = tls;
}


void uthread_start(uthread_func_t start, void *arg)
{
	// initialize t_id_index
	t_id_index = 0; 
	ready_q = queue_create();
	//Initilize active thread to new idle thread
	//Context handled at first switch by context.h

	//preempt_start(); //Start preemption signals
	preempt_disable();	

	active_tcb = malloc(sizeof(uthread_tcb));
	active_tcb->state = RUNNING;
	active_tcb->t_id = 0;
	active_tcb->uctx = malloc(sizeof(uthread_ctx_t));

	palloc_create(); //Palloc configured externally before uthread called

	if(!ready_q)
		exit(1);

	if (uthread_create(start, arg) != 0){
		exit(1);
	}

	while(queue_length(ready_q) > 0){ //Resume idle thread
		
		uthread_yield();
	}

	//printf("At idle thread with empty queue\n");
	exit(0);

}

void uthread_mem_config(size_t npages)
{
	palloc_configure(npages);
}
