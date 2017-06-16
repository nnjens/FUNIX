#ifndef _PALLOC_H
#define _PALLOC_H

/*
 * Size of a memory page: 4KiB
 */
#define PAGE_SIZE 4096

/*
 * Page protection type
 */
enum page_protection {
	/* Remove all possible access to page */
	PAGE_NO_ACCESS,
	/* Enable read and write access to page */
	PAGE_RW_ACCESS,
};

/*
 * palloc_configure - Configure the memory page allocator
 * @npages: Number of pages
 *
 * Setup the number of available pages in the system
 */
void palloc_configure(size_t npages);

/*
 * palloc_create - Create the memory page allocator
 *
 * Return: 0 in case of success, or -1 in case of failure
 */
int palloc_create(void);

/*
 * palloc_destroy - Destory the memory page allocator
 *
 * Return: 0 in case of success, or -1 in case of failure
 */
int palloc_destroy(void);

/*
 * palloc_get_pages - Get a set of memory pages
 * @count: Number of pages required
 *
 * Return: Address of a memory area containing @count free pages, or NULL in
 * case of failure
 */
void *palloc_get_pages(size_t count);

/*
 * palloc_free_pages - Free a set of memory pages
 * @ptr: Address of the memory area containing the pages
 * @count: Number of consecutive pages to free
 */
void palloc_free_pages(void *ptr, size_t count);

/*
 * palloc_protect_pages - Protect a set of memory pages
 * @ptr: Address of the memory area to protect
 * @count: Number of consecutive pages to protect
 * @prot: Protection to apply to the set of pages
 *
 * Return: 0 in case of success, or -1 in case of failure
 */
int palloc_protect_pages(void *ptr, size_t count, enum page_protection prot);

#endif /* _PALLOC_H */
