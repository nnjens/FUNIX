#include <assert.h>
#include <signal.h>
#include <sys/mman.h>

#define _UTHREAD_PRIVATE
#include "bitmap.h"
#include "palloc.h"
#include "preempt.h"

struct palloc {
	size_t npages;	/* Number of physical pages */
	unsigned char* memory_block;
	bitmap_t memory_status;
};

/* Pool of memory pages */
static struct palloc ppool;

void palloc_configure(size_t npages)
{
	ppool.npages = npages;
}

int palloc_create(void)
{
	ppool.memory_status = bitmap_create(ppool.npages); //Create bitmap of pages
	ppool.memory_block = mmap(NULL, ppool.npages, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	return 0;
}

int palloc_destroy(void)
{
	munmap(ppool.memory_block, ppool.npages);
	bitmap_destroy(ppool.memory_status);
	return 0;
}

void *palloc_get_pages(size_t count)
{
	long unsigned int start;
	if(!bitmap_find_region(ppool.memory_status, count, &start))
		return NULL;
	return ppool.memory_block + (start * PAGE_SIZE);	
}

void palloc_free_pages(void *ptr, size_t count)
{
	bitmap_clr(ppool.memory_status, (ptr - (void*)ppool.memory_block), count);
}

int palloc_protect_pages(void *ptr, size_t count, enum page_protection prot)
{
	if(prot == PAGE_NO_ACCESS)
		mprotect(ptr, count, PROT_NONE);
	
	if(prot == PAGE_RW_ACCESS)
		mprotect(ptr, count, PROT_WRITE);
	return 0;
}

