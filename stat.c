// Find file in /~

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <pwd.h>
#include <unistd.h>
#include <malloc.h>
#define DMAX 31


char file[DMAX];

void Citire(int argc, char *argv[]){

	if (argc != 2){
		printf("Numele fisierului cautat: \n");
		scanf("%s", file);
	}
	else{
			strcpy(file, argv[1]);
	}
}

int main(int argc, char *argv[]){

	Citire(argc, argv);
	struct stat buf;          
	stat(file, &buf);
	int size = buf.st_size;
	int ino = buf.st_ino;

	printf("Size: %d\n", size);
	printf("Inode: %d\n", ino);
    printf("Last modified time: %s", ctime(&buf.st_mtime));
    printf("Last status change: %s", ctime(&buf.st_ctime));


	return 0;
}

