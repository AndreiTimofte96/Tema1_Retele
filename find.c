// Find file in /~

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pwd.h>
#include <unistd.h>
#include <malloc.h>
#define DMAX 1001


char file[DMAX], str[DMAX];
char *path;
struct stat buf;

void Citire(int argc, char *argv[]){

	if (argc != 2){
		printf("Numele fisierului cautat: \n");
		scanf("%s", file);
	}
	else{
		strcpy(file, argv[1]);
	}
}


void Find(char *home){

	DIR *dir;
	struct dirent *curDir;
	struct stat stat_info;

	//Permision Denied Error
	if (strcmp(home, "/home/timi/.cache/dconf") == 0){
		return;
	}

	if ( (dir = opendir(home)) == NULL){
		printf("DIR ERROR: %s\n", strerror(errno));
		printf("%s\n", home);
		return;
	}

	while ( (curDir = readdir(dir)) != NULL){

		if (strcmp(curDir->d_name, file) == 0){

			printf("%s\n", home);

			strcpy(str, home);
			strcat(str, "/");
			strcat(str, file);

			stat(str, &buf);
			int size = buf.st_size;
			int ino = buf.st_ino;
			printf("Size: %d\n", size);
			printf("Inode: %d\n", ino);
		    printf("Last modified time: %s", ctime(&buf.st_mtime));
		    printf("Last status change: %s", ctime(&buf.st_ctime));
			return;
		}

		if (curDir->d_type == DT_DIR){
			if (strcmp(curDir->d_name, ".") != 0 && strcmp(curDir->d_name, "..") != 0 ){
				path = malloc(strlen(curDir->d_name) + strlen(home) + 2);
				sprintf(path, "%s/%s", home, curDir->d_name);
				Find(path);
			}
		}
	}
	closedir(dir);
}


int main(int argc, char *argv[]){

	struct passwd *pw = getpwuid(getuid());
	char *home = pw->pw_dir;
	Citire(argc, argv);
	Find(home);
	return 0;
}

