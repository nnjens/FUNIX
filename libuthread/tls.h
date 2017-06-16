#ifndef _TLS_H
#define _TLS_H

#include <stddef.h>

/*
 * tls_create - Create a new TLS area
 * @npages: Number of required pages
 *
 * Return: 0 in case of success, or -1 in case of failure
 */
int tls_create(size_t npages);

/*
 * tls_destory - Destroy TLS area
 *
 * Return: 0 in case of success, or -1 in case of failure
 */
int tls_destroy(void);

/*
 * tls_alloc - Allocate memory from TLS
 * @size: Size of memory to allocate in bytes
 *
 * Return: Address on the allocated memory area, or NULL in case of failure
 */
void *tls_alloc(size_t size);

/*
 * tls_free - Free memory
 * @ptr: Address of memory area to be freed
 *
 * Return: 0 in case of success, or -1 in case of failure
 */
int tls_free(void *ptr);

#ifdef _UTHREAD_PRIVATE

/*
 * tls_open - Open the TLS area of the calling thread
 */
void tls_open(void);

/*
 * tls_close - Close the TLS area of the calling thread
 */
void tls_close(void);

#endif /* UTHREAD_PRIVATE */

#endif /* _TLS_H */
