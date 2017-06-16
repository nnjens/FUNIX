#ifndef _BITMAP_H
#define _BITMAP_H

#include <stddef.h>


//#ifdef _UTHREAD_PRIVATE //Commented out for testing 

/*
 * bitmap_t - Bitmap type
 *
 * A bitmap is an array data structure that compactly stores bits. Each bit
 * typically represents the availabibity of a resource.
 */
typedef struct bitmap* bitmap_t;

/*
 * bitmap_create - Allocate a bitmap
 * @nbits: Number of bits that the bitmap should contained
 *
 * Return: Pointer to bitmap (fully initialized to 0), or NULL in case of
 * failure
 */
bitmap_t bitmap_create(size_t nbits);


//DELETE

void printmap(bitmap_t map);


/*
 * bitmap_destroy - Deallocate a bitmap
 * @bitmap: Bitmap to deallocate
 *
 * Return: 0 if @bitmap was successfully destroyed, or -1 in case of failure
 */
int bitmap_destroy(bitmap_t bitmap);

/*
 * bitmap_empty - Check whether bitmap is empty
 * @bitmap: Bitmap to check
 *
 * Return: 1 if @bitmap is empty (none of the bits are set to 1) or 0 otherwise
 */
int bitmap_empty(bitmap_t bitmap);

/*
 * bitmap_full - Check whether bitmap is full
 * @bitmap: Bitmap to check
 *
 * Return: 1 if @bitmap is full (all of the bits are set to 1) or 0 otherwise
 */
int bitmap_full(bitmap_t bitmap);

/*
 * bitmap_any - Check whether bitmap contains a set bit in a specified range
 * @bitmap: Bitmap to check
 * @pos: Starting position in bitmap
 * @nbits: Number of bits to check
 *
 * Return: 1 if @bitmap contains at least one set bit, in the range of @nbits
 * starting a @pos, or 0 otherwise
 */
int bitmap_any(bitmap_t bitmap, size_t pos, size_t nbits);

/*
 * bitmap_none - Check whether bitmap doesn't contain any set bit in a specified
 * range
 * @bitmap: Bitmap to check
 * @pos: Starting position in bitmap
 * @nbits: Number of bits to check
 *
 * Return: 1 if @bitmap doesn't contain any set bit, in the range of
 * @nbits starting a @pos, or 0 otherwise
 */
int bitmap_none(bitmap_t bitmap, size_t pos, size_t nbits);

/*
 * bitmap_all - Check whether bitmap contains only set bits in a specified range
 * @bitmap: Bitmap to check
 * @pos: Starting position in bitmap
 * @nbits: Number of bits to check
 *
 * Return: 1 if @bitmap contains only set bits, in the range of @nbits starting
 * a @pos, or 0 otherwise
 */
int bitmap_all(bitmap_t bitmap, size_t pos, size_t nbits);

/*
 * bitmap_set - Set all the bits in a specified range
 * @bitmap: Bitmap
 * @pos: Starting position in bitmap
 * @nbits: Number of bits to check
 *
 * Return: 0 in case of success, or -1 in case of failure
 */
int bitmap_set(bitmap_t bitmap, size_t pos, size_t nbits);

/*
 * bitmap_clr - Clear all the bits in a specified range
 * @bitmap: Bitmap
 * @pos: Starting position in bitmap
 * @nbits: Number of bits to check
 *
 * Return: 0 in case of success, or -1 in case of failure
 */
int bitmap_clr(bitmap_t bitmap, size_t pos, size_t nbits);

/*
 * bitmap_set_one - Set specified bit
 * @bitmap: Bitmap
 * @pos: Position in bitmap
 *
 * Return: 0 in case of success, or -1 in case of failure
 */
int bitmap_set_one(bitmap_t bitmap, size_t pos);

/*
 * bitmap_clr_one - Clear specified bit
 * @bitmap: Bitmap
 * @pos: Position in bitmap
 *
 * Return: 0 in case of success, or -1 in case of failure
 */
int bitmap_clr_one(bitmap_t bitmap, size_t pos);

/*
 * bitmap_find_region - Find available region and set corresponding bits
 * @bitmap: Bitmap
 * @count: Number of available consecutive bits to find
 * @pos: Address of variable where the starting position is received
 *
 * Return: 1 if @pos was set with the starting position of a now set range of
 * @count bits, 0 if no region was available, and -1 in case of failure
 */
int bitmap_find_region(bitmap_t bitmap, size_t count, size_t *pos);

//#else
//#error "Private header, can't be included from applications directly"
//#endif

#endif /* _BITMAP_H */
