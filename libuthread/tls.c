#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define _UTHREAD_PRIVATE
#include "palloc.h"
#include "uthread.h"
#include "tls.h"

typedef struct tls_node tls_node;

struct tls_node {
	tls_node* next;
	int start, size; //Start index and length of allocated memory
} ;

typedef struct tls_block {
	unsigned char* tls_mem;
	int tls_size;
	tls_node* allocated_mem;	

} tls_block;

int tls_create(size_t npages)
{
	
	tls_block *tlsb = malloc(sizeof(tls_block));	
	tlsb->tls_mem =	palloc_get_pages(npages);
	tlsb->tls_size = npages * PAGE_SIZE;
	tlsb->allocated_mem = NULL;

	if(!tlsb->tls_mem)
		return -1;

	uthread_set_tls(tlsb);
	return 0;
}

int tls_destroy(void)
{
	tls_block *tlsb = (tls_block*) uthread_get_tls();	
	
	if(!tlsb)
		return 1;

	palloc_free_pages(tlsb->tls_mem, tlsb->tls_size / PAGE_SIZE);

	tls_node *itr = tlsb->allocated_mem;

	while(itr && itr->next)
	{
		tlsb->allocated_mem = itr;
		itr = itr->next;
		if(tlsb->allocated_mem) 
			free(tlsb->allocated_mem);
	} //Free allocated memory list
	if(tlsb)
		free(tlsb);
	return 0;
}

void tls_open(void)
{
	
	tls_block *tlsb = uthread_get_tls();
	if(!tlsb)
		return;
	if(!tlsb->tls_mem)
		return;
	palloc_protect_pages(tlsb->tls_mem, tlsb->tls_size / PAGE_SIZE, PAGE_RW_ACCESS);
}

void tls_close(void)
{
	tls_block *tlsb = uthread_get_tls();

	if(!tlsb)
		return;
	if(!tlsb->tls_mem)
		return;
	palloc_protect_pages(tlsb->tls_mem, tlsb->tls_size / PAGE_SIZE, PAGE_NO_ACCESS);
}

void *tls_alloc(size_t size)
{
	tls_block *tlsb = uthread_get_tls();
	
	int start = 0;	
	tls_node* itr;		
	itr = tlsb->allocated_mem;
	if(itr && itr->start > size - 1) //Supplanting first
	{
		tls_node *new = malloc(sizeof(tls_node)); 
		new->start = start;
		new->size = size;
		new->next = itr;
		tlsb->allocated_mem = new;

		return &(tlsb->tls_mem[start]);

	}

	for(int c = 0; itr && itr->next && (itr->next->start - itr->start <= size); itr = itr->next, c++);

	if(!itr) {
		if(size - 1 < tlsb->tls_size)
		{
			tls_node *new = malloc(sizeof(tls_node)); //Add first allocation
			new->start = start;
			new->size = size;
			new->next = NULL;
			tlsb->allocated_mem = new;
			return &(tlsb->tls_mem[start]);
		}
		else
			return NULL; //Requesting too much space
	}
	if(!itr->next) {
		if(size + itr->start < tlsb->tls_size)
		{
			
			start = itr->start + itr->size;

			tls_node *new = malloc(sizeof(tls_node)); 
			new->start = start;
			new->size = size;
			new->next = NULL;
			itr->next = new;
			return &(tlsb->tls_mem[start]);
		}
		else
			return NULL;
	
	//Itr and itr next exist, we are in a free space between 2 blocks
	}
	start = itr->start + itr->size;

	tls_node *new = malloc(sizeof(tls_node)); 
	new->start = start;
	new->size = size;
	new->next = itr->next;
	itr->next = new;

	return &(tlsb->tls_mem[start]);
}

int tls_free(void *ptr)
{
	tls_block *tlsb = uthread_get_tls();
	int start = (unsigned char*)ptr - tlsb->tls_mem; //Find start index

	tls_node* itr = tlsb->allocated_mem;

	if(itr->start == start) //First node is target
	{
		tlsb->allocated_mem = itr->next;
		free(itr);
		return 0;
	}

	for(; itr && itr->next && itr->next->start != start; itr = itr->next);  //Finds node before target node
	if(!itr->next) //Last node ie target not in list
		return -1;

	//Target is itr->next, remove from list and free
	itr->next = itr->next->next;
	free(itr->next);
	return 0;
}
