#include <assert.h>
#include <stdlib.h>

#define _UTHREAD_PRIVATE
#include "bitmap.h"
#include "preempt.h"

/* Divide @n by @d and round up to the nearest @d unit */
#define DIV_ROUND_UP(n, d)	(((n) + (d) - 1) / (d))

#define BITS_PER_BYTE		8
#define BITS_TO_BYTES(nr)	DIV_ROUND_UP(nr, BITS_PER_BYTE)

#define BIT(nr)			(1 << (nr))
#define BIT_MASK(nr)		(1 << ((nr) % BITS_PER_BYTE))
#define BIT_WORD(nr)		((nr) / BITS_PER_BYTE)

static inline void set_bit(int nr, volatile unsigned char *map)
{
	unsigned char mask = BIT_MASK(nr);
	unsigned char *p = ((unsigned char*)map) + BIT_WORD(nr);
	sigset_t level;

	preempt_save(&level);
	*p |= mask;
	preempt_restore(&level);
}

static inline void clr_bit(int nr, volatile unsigned char *map)
{
	unsigned char mask = BIT_MASK(nr);
	unsigned char *p = ((unsigned char*)map) + BIT_WORD(nr);
	sigset_t level;

	preempt_save(&level);
	*p &= ~mask;
	preempt_restore(&level);
}

struct bitmap {
	int size;
	unsigned char* bitString;
};

bitmap_t bitmap_create(size_t nbits)
{
	if(nbits < 0)
		return NULL;
	
	struct bitmap *map = malloc(sizeof(struct bitmap));
	
	if(!map)
		return NULL; //Malloc failed

	map->size = nbits;
	map->bitString = malloc(sizeof(BITS_TO_BYTES(nbits)));

	//Initialize to all zeros

	for(int i = 0; i < (BITS_TO_BYTES(nbits)); i++)
		map->bitString[i] = 0x00;

	return map;
}

int bitmap_destroy(bitmap_t bitmap)
{
	free(bitmap->bitString);
	free(bitmap);
	return 1;
}

int bitmap_empty(bitmap_t bitmap)
{
	for(int i = 0; i < BITS_TO_BYTES(bitmap->size); i++)
	{
		if((int)bitmap->bitString[i])
			return 0; //Non-zero byte
	}
	return 1;
}

int bitmap_full(bitmap_t bitmap)
{
	for(int i = 0; i < BITS_TO_BYTES(bitmap->size) - 1; i++)
		if((int)bitmap->bitString[i] != 0xFF) //Not full byte
			return 0;
	
	unsigned char byte = bitmap->bitString[BITS_TO_BYTES(bitmap->size) - 1];
	
	for(int i = 0; i < bitmap->size % BITS_PER_BYTE; i++, byte = byte >> 1)
		if(!(0x01 & byte))
			return 0; //Zero bit 

	return 1;
		
}

int bitmap_any(bitmap_t bitmap, size_t pos, size_t nbits)
{
	int word = BIT_WORD(pos), index = pos % BITS_PER_BYTE; //Sets the word and the index defined as the start
	
	for(int i = 0; i < nbits; i++) { //Step through n bits
		if(!(index + i % BITS_PER_BYTE))
			word++; //Next word
		if(bitmap->bitString[word] & BIT_MASK(index + i)) //Bit at index + i is a 1
			return 1;
	} 
	return 0;
}

int bitmap_none(bitmap_t bitmap, size_t pos, size_t nbits)
{
	return !bitmap_any(bitmap, pos, nbits); //Not any is none
}

int bitmap_all(bitmap_t bitmap, size_t pos, size_t nbits)
{
	int word = BIT_WORD(pos), index = pos % BITS_PER_BYTE; //Sets the word and the index defined as the start
	
	for(int i = 0; i < nbits; i++) { //Step through n bits
		if(!(index + i % BITS_PER_BYTE))
			word++; //Next word
		if(!(bitmap->bitString[word] & BIT_MASK(index + i))) //Bit at index + i is a 0 
			return 0;
	} 
	return 1;
}

int bitmap_set(bitmap_t bitmap, size_t pos, size_t nbits)
{
	if(pos + nbits > bitmap->size)
		return 0;
	for(int i = 0; i < nbits; i++) //One at a time, can reconfigure to do chunks if I have time 
		bitmap_set_one(bitmap, pos + i);
	return 1;
}

int bitmap_clr(bitmap_t bitmap, size_t pos, size_t nbits)
{
	if(pos + nbits > bitmap->size)
		return 0;
	for(int i = 0; i < nbits; i++)
		bitmap_clr_one(bitmap, pos + i);
	return 1;
}

int bitmap_set_one(bitmap_t bitmap, size_t pos)
{
	if(pos > bitmap->size)
		return 0;
	bitmap->bitString[pos / BITS_PER_BYTE] = bitmap->bitString[pos/BITS_PER_BYTE] | BIT_MASK(pos); 
	return 1;
}

int bitmap_clr_one(bitmap_t bitmap, size_t pos)
{
	if(pos > bitmap->size)
		return 0;
	bitmap->bitString[pos / BITS_PER_BYTE] = bitmap->bitString[pos/BITS_PER_BYTE] & ~(BIT_MASK(pos)); 
	return 1;
}

int bitmap_find_region(bitmap_t bitmap, size_t count, size_t *pos)
{
	int word = -1, start = 0, c = 1;

	for(int i = 0; i < bitmap->size; i++) {
		if(c == count) //Found enough space
		{
			bitmap_set(bitmap, start, count);	
			*pos = start;
			return 1;
		}
		if(!(i % BITS_PER_BYTE))
			word++; //Next word in bitstring
		if(bitmap->bitString[word] & BIT_MASK(i)) { //Not enough space, start counting over
			c = 1;	
			start = i + 1;
		}
		else
			c++;
	}

	return 0;

}


//TESTING DELETE
#include <stdio.h>
void printMap(bitmap_t map)
{
	int word = -1;
	for(int i = 0; i < map->size; i++)
	{	
		if(i % 8 == 0)
			word++;
		printf("%d", (map->bitString[word] & (1 << (i % 8)))?1:0);
	}
	printf("\n");

}
