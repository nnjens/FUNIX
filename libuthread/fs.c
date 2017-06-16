#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _UTHREAD_PRIVATE
#include "disk.h"
#include "fs.h"

#define FAT_EOC 0xFFFF
#define FREE 0

int allocateBlock(); //Find first unallocated Block 

struct superblock {
	char *sig;
	int16_t diskSize;
	int16_t rootIdx; //Data is always next block 
	int16_t numBlocks;
	int8_t fatSize;
	int8_t padding[4079];
} __attribute__((packed));

struct superblock *sb;

int16_t *FAT; //FAT of current mounted disk

typedef struct file {
	int8_t filename[16];
	int32_t size;
	uint16_t idx;
	int8_t padding[10];
} __attribute__((packed)) file;

file root[128]; //Root directory of active disk

int fdTable[FS_OPEN_MAX_COUNT], offTable[FS_OPEN_MAX_COUNT];

int fs_mount(const char *diskname)
{
	if(block_disk_open(diskname) < 0)
		return -1;
	
	unsigned char buffer[BLOCK_SIZE];
	sb = malloc(sizeof(struct superblock));
	sb->sig = malloc(8);

	block_read(0, buffer);
	
	for(int i = 0; i < FS_OPEN_MAX_COUNT; i++) //Initialize fdTable empty
	{
		fdTable[i] = -1;
		offTable[i] = -1;
	}

	
	if(!buffer[0]) //Unformated Disk, need to create FAT data
	{
		return -1;	
	}
	
	if(strncmp((char*)buffer, "ECS150FS", 8)) //Formated to different FS
	{
		//printf("Error formated to different filesystem\n");
		return -1;
	}
	
	strncpy((char* restrict)sb->sig, (char* const)buffer, 8);
	sb->diskSize = *((int16_t*)(buffer + 8));
	sb->rootIdx = *((int16_t*)(buffer + 10));
	sb->numBlocks = *((int16_t*)(buffer + 14));
	sb->fatSize = *((int16_t*)(buffer + 16));

	//Read in FAT

		
	FAT = malloc(sizeof(int16_t) * sb->numBlocks);	

	for(int i = 0; i < sb->fatSize - 1; i++)
	{
		block_read(1 + i, buffer); //Read each block
		memcpy(&FAT[i * BLOCK_SIZE], buffer, BLOCK_SIZE);
	}
	
	int memsize = ((sb->numBlocks * 2) % BLOCK_SIZE);
	block_read(sb->fatSize, buffer);
	memcpy(&FAT[(sb->fatSize - 1) * BLOCK_SIZE], buffer, memsize);

	//Read in root directory 

	block_read(sb->rootIdx, buffer);
	memcpy(root, buffer, BLOCK_SIZE);

				
	return 0;	
		
}

int fs_umount(void)
{
	//Write changes here
	if(!sb) //No Mounted disk
		return -1;
	
	for(int i = 0; i < FS_OPEN_MAX_COUNT; i++)
		if(offTable[i] >= 0)
		{
			//printf("File %d still open\n", i);
			return -1;
		}

	for(int i = 0; i < sb->fatSize - 1; i++)
		block_write(1 + i, &FAT[i * BLOCK_SIZE]); //Read each block
	
	block_write(sb->fatSize, &FAT[(sb->fatSize - 1) * BLOCK_SIZE]);

	block_write(sb->rootIdx, root);	
	

	if(sb)
		free(sb);
	if(FAT)
		free(FAT);
	
	sb = 0;	
	FAT = 0;
	return block_disk_close();	
}

int fs_info(void)
{
	if(!sb) //No Mounted disk
		return -1;
	printf("FS Info:\n");
	printf("total_blk_count=%d\n", sb->diskSize);
	printf("fat_blk_count=%d\n", sb->fatSize);
	printf("rdir_blk=%d\n", sb->rootIdx);
	printf("data_blk=%d\n", sb->rootIdx + 1);
	printf("data_blk_count=%d\n", sb->numBlocks);
	int count = 0;	
	for(int c = 0; c < sb->numBlocks; c++)
		if(FAT[c])
			count++;
	
	printf("fat_free_ratio=%d/%d\n", sb->numBlocks - count, sb->numBlocks);
	count = 0;
	for(int c = 0; c < 128; c++)
		if(root[c].filename[0] == '\0')
			count++;
	printf("rdir_free_ratio=%d/128\n", count);
	return 0;	
}

int fs_create(const char *filename)
{
	int first = 128;
	if(!sb) //No Mounted disk
		return -1;
	for(int  i = 0; i < 128; i++)	
	{
		if(root[i].filename[0] == '\0' && (first == 128))
			first = i;

		if(!strcmp((char* const)root[i].filename, filename))
		{
			//printf("File %s already exists\n", filename);
			return -1;
		}

	}
	if(first == 128)
	{	
		//printf("Root directory full\n");
		return -1;	
	}
	
	strcpy((char* restrict)root[first].filename, filename);
	root[first].filename[strlen(filename)] = '\0';
	root[first].size = 0;
	root[first].idx = FAT_EOC;
	return 0;

}

int fs_delete(const char *filename)
{
	if(!sb) //No Mounted disk
		return -1;
	for(int i = 0; i < 128; i++)
		if(!strcmp((char* const)root[i].filename, filename))
		{
			strcpy((char* restrict)root[i].filename, "\0");
			root[i].size = 0;
			
			uint16_t next = root[i].idx;

			while(next != FAT_EOC)
			{
				root[i].idx = next;
				next = FAT[root[i].idx];
				FAT[root[i].idx] = 0;
			}

			//FAT[root[i].idx] = 0;
			root[i].idx = FAT_EOC;
			return 0;
		}	
	return -1;
}

int fs_ls(void)
{
	if(!sb) //No mounted Disk
		return -1;
	printf("FS Ls:\n");
	for(int i = 0; i < 128; i++)
		if(root[i].filename[0] != '\0')
			printf("file: %s, size: %d, data_blk: %d\n",root[i].filename, root[i].size, root[i].idx);
	return 0;	
}

int fs_open(const char *filename)
{
	if(!sb) //No Mounted disk
		return -1;
	int exist = 0, fIdx;
	for(fIdx = 0; fIdx < 128; fIdx++)
		if(!strcmp((char* const)root[fIdx].filename, filename))
		{
			exist = 1;
			break;
		}

	if(!exist)
	{
		//printf("File doesn't exist in root\n");
		return -1;
	}

	for(int i = 0; i < FS_OPEN_MAX_COUNT; i++)
	{
		if(fdTable[i] < 0)
		{	
			fdTable[i] = fIdx; //Save file location and set offset
			offTable[i] = 0;
			return i;
		}
	}	
	
	//printf("All file descriptors used\n");
	return -1;	
}

int fs_close(int fd)
{
	if(!sb) //No Mounted disk
		return -1;
	if(fd > FS_OPEN_MAX_COUNT || fd < 0)
		return -1;
	if(fdTable[fd] < 0)
		return -1;
	
	fdTable[fd] = -1;
	offTable[fd] = -1;

	return 0;
}

int fs_stat(int fd)
{
	if(!sb) //No Mounted disk
		return -1;
	if(fd > FS_OPEN_MAX_COUNT || fd < 0)
		return -1;

	if(fdTable[fd] < 0)
		return -1;

	return root[fdTable[fd]].size;	
}

int fs_lseek(int fd, size_t offset)
{
	if(!sb) //No Mounted disk
		return -1;
	if(fd > FS_OPEN_MAX_COUNT || fd < 0)
		return -1;

	if(fdTable[fd] < 0)
		return -1;

	
	offTable[fd] = offset;
	return 0;	
}

int fs_write(int fd, void *buf, size_t count)
{
	if(!sb) //No Mounted disk
		return -1;
	if(fd > FS_OPEN_MAX_COUNT || fd < 0)
		return -1;

	if(fdTable[fd] < 0)
	{
		//printf("File not opened\n");
		return -1;
	}

	if(count == 0)
		return 0; //Nothing to write

	file* f = &root[fdTable[fd]];
	uint16_t startBlock = f->idx; //Get file block idx
	
	char* buffer = malloc(sizeof(int8_t) * BLOCK_SIZE); //Get block buffer

	if(startBlock == FAT_EOC) //Empty file
	{
		startBlock = allocateBlock();
		if((int16_t) startBlock < 0)
		{
			return 0; //No space to allocate to the write
		}	
		f->idx = startBlock;
	}

	for(int i = 0; i < (offTable[fd] / BLOCK_SIZE); i++)
	{
		if(FAT[startBlock == FAT_EOC])
		{
			//printf("Offset outside file, setting to last byte\n");
			offTable[fd] = f->size % BLOCK_SIZE;
		}
		startBlock = FAT[startBlock];
	}

	int oldOff = offTable[fd];	
	
	//Modify block and subsequent blocks, allocating when needed
	

	if(offTable[fd] + count > f->size)
		f->size += (count - (f->size - offTable[fd]));	

	while(count > 0)
	{
		block_read(startBlock + sb->rootIdx, buffer);	
		
		if(count + (offTable[fd] % BLOCK_SIZE) > BLOCK_SIZE)//will need multiple blocks
		{
			block_read(startBlock + sb->rootIdx, buffer);

			int len = BLOCK_SIZE - (offTable[fd] % BLOCK_SIZE);
			memcpy(buffer + (offTable[fd] % BLOCK_SIZE), buf, len);
			
			count -= len;
			offTable[fd] += len; //Start at beginning of new block
			
			block_write(startBlock + sb->rootIdx, buffer); 
			
			if((uint16_t)FAT[startBlock] == FAT_EOC) //In last block
				FAT[startBlock] = allocateBlock();

			if(FAT[startBlock] < 0) //allocateBlock failed return total written to this point
				return offTable[fd] - oldOff;
			startBlock = FAT[startBlock]; //Get next block	
		}
		else //Last block
		{
			block_read(startBlock + sb->rootIdx, buffer); //To preserve possible tail
			memcpy(buffer + (offTable[fd] % BLOCK_SIZE), buf, count);
			offTable[fd] += count; //New offset
			count = 0;	
			block_write(startBlock + sb->rootIdx, buffer);
		}
	}
		
	return offTable[fd] - oldOff;	
}

int fs_read(int fd, void *buf, size_t count)
{
	if(!sb) //No Mounted disk
		return -1;
	if(fd > FS_OPEN_MAX_COUNT || fd < 0)
		return -1;

	if(fdTable[fd] < 0)
	{
		//printf("File not opened\n");
		return -1;
	}

	file* f = &root[fdTable[fd]];
	uint16_t startBlock = f->idx; //Get file block idx
	
	char* buffer = malloc(sizeof(int8_t) * BLOCK_SIZE); //Get block buffer

	if(startBlock == FAT_EOC) //Empty file
	{
		//printf("Cannot read from empty file\n");
		return -1;
	}
	
	if(offTable[fd] + count > f->size) 
	{
		count = f->size - offTable[fd]; // Read only to end of file
	}

	for(int i = 0; i < (offTable[fd] / BLOCK_SIZE); i++)
	{
		if(FAT[startBlock == FAT_EOC])
		{
			//printf("Offset outside file cannot read\n");
			return 0;
		}
		startBlock = FAT[startBlock];
	}

	
	//read block and subsequent blocks, allocating when needed

	int itr = 0;	
	while(count > 0)
	{
		block_read(startBlock + sb->rootIdx, buffer);	
		
		if(count + (offTable[fd] % BLOCK_SIZE) > BLOCK_SIZE)//will need multiple blocks
		{
			int len = BLOCK_SIZE - (offTable[fd] % BLOCK_SIZE);
			memcpy(buf + itr, buffer + (offTable[fd] % BLOCK_SIZE), len);

			count -= len;
			itr += len;
			offTable[fd] += len; //Start at beginning of new block

			startBlock = FAT[startBlock]; //Get next block	
		}
		else //Last block
		{
			memcpy(buf + itr, buffer + (offTable[fd] % BLOCK_SIZE), count);
			offTable[fd] += count; //New offset
			itr += count;
			count = 0;	
			
		}
	}
		
	return itr;	
}

int allocateBlock() //Find first unallocated Block 
{
	for(int i = 0; i < sb->numBlocks; i++)
		if(!FAT[i])
		{
			FAT[i] = FAT_EOC;
			return i; 
		}
	return -1;
}
