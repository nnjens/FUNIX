//#include "disk.h"

#include <stddef.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "fs.h"

void clearBuf(char* buffer)
{
	for(int i = 0; buffer[i]; buffer[i] = '\0', i++);
}

int main(int argc, char** argv)
{

	printf("\n---Testing Mount---\n");
	//Mount uMount tests	
	printf("%d\n", fs_mount("Disk1"));
	//fs_info();	
	fs_umount();
	printf("%d\n", fs_mount("Disk2"));
	//fs_info();	
	fs_umount();
	printf("%d\n", fs_mount("Disk1"));
	fs_info();	
	fs_umount();
	
	printf("%d\n", fs_mount("Disk3"));
	//fs_info();	
	fs_umount();


	//File tests

	printf("\n\n---Testing file create---\n\n");
	fs_mount("Disk3");
	for(int i = 0; i < 129; i++)
	{
		//char fn[4];
		//itoa(i, fn, 10); 
		if(fs_create("MyFile") < 0)
			break;
	}
	fs_ls();
	fs_umount();


	printf("\n\nRemounting....\n");
	fs_mount("Disk3");
	fs_ls();

	fs_create("File2");
	fs_create("File3");
	fs_delete("File2");
	fs_ls();
	
	printf("\n\n---Open and Close---\n");

	int c = fs_open("File");
	printf("%d, %d\n", c, fs_stat(c));	
	int c1 = fs_open("File3");
	fs_lseek(c1, 10);
	printf("%d, %d\n", c1, fs_stat(c1));	
	int c2 = fs_open("MyFile");
	printf("%d, %d\n", c2, fs_stat(c2));	
	int c3 = fs_open("File3");
	printf("%d, %d\n", c3, fs_stat(c3));
	fs_close(c3);
	
	fs_umount();
	
	fs_close(c2);
	fs_close(c1);

	fs_umount();


	printf("\n\n---Testing read/write---\n\n");

	//read write tests	
	char* buffer = malloc(sizeof(char) * 12);
	
	for(int i = 0; i < 12; i++)
		buffer[i] = '@';

	fs_mount("Disk4");
	fs_create("File1");
	fs_create("File2");
	int f1 = fs_open("File1"), f2 = fs_open("File2");
	
	clearBuf(buffer);

	fs_write(f1, "String", 6); //Simple write
	fs_lseek(f1, 0);
	fs_read(f1, buffer, 6);
	printf("%s\n", buffer);
	fs_close(f1);


	fs_write(f2, "hello", 5); //Test offset
	fs_lseek(f2, 0);

	fs_read(f2, buffer, 6); //Error too long
	fs_read(f2, buffer, 5);
	printf("%s\n", buffer); //hello

	fs_write(f2, " World\0", 7);
	fs_lseek(f2, 0); 

	fs_read(f2, buffer, 5);
	printf("%s\n", buffer); //hello

	fs_lseek(f2, 0); 
	fs_read(f2, buffer, 12);
	printf("%s\n", buffer); //hello World

	fs_lseek(f2, 0);
	fs_write(f2, "H", 1); 

	fs_lseek(f2, 0); 
	fs_read(f2, buffer, 12); //Hello World
	printf("%d\n", (int)strlen(buffer));
	printf("%s\n", buffer);
	fs_close(f2);
	fs_umount();

	clearBuf(buffer);
	//Remounting
	printf("\n\n\nRemounting\n\n");
	fs_mount("Disk4");
	fs_ls();	
	f1 = fs_open("File1");
	
	fs_read(f1, buffer, 5);
	fs_read(f1, buffer + 5, 1);
	printf("%d\n", fs_stat(f1));
	printf("%s\n",buffer);	
	fs_close(f1);		
	fs_umount();

}
