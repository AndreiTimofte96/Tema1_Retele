#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#define LEN 30
#define DMAX 1001
#define FIFO_SEND "FIFO_R"
#define FIFO_RECEIVE "FIFO_S"

char x;
char username[LEN], password[LEN], realUsername[LEN], realPassword[LEN];
char instruction[DMAX], path[DMAX] = "/home/timi";


enum Channel{Pipes, Fifos, Sockets} channel;

/////////////////Autentificare plus setare canal
int SetChannel(int argc, char *argv[]){

	if (argc != 2){
		printf("%s\n", "Eroare, nu ai introdus canalul de comunicare dorit!");
		printf("%s\n", "Optiuni: -p, -f, -s");
		return 0;
	}
	if (strcmp(argv[1], "-p") == 0){
		channel = Pipes;
	}
	if (strcmp(argv[1], "-f") == 0){
		channel = Fifos;
	}
	if (strcmp(argv[1], "-s") == 0){
		channel = Sockets;
	}

	return 1;
}
void ReadCredentials(){

	FILE *df;
	df = fopen("credentials.txt", "r");
	
	fscanf(df, "%s %s", realUsername, realPassword);
}

int RequestCredentials(){

	printf("%s", "Username: ");
	scanf("%s", username);
	scanf("%c", &x);
	printf("%s", "Password: ");
	scanf("%s", password);
	scanf("%c", &x);

	if (strcmp(username, realUsername) == 0 && strcmp(password, realPassword) == 0){
		return 1;
	}
	printf("%s\n", "Wrong credentials!");
	return 0;
}//////////////////////////


//////////////////////Afisare linie + Citire comenzi
void ReplaceString(char *str){

	char auxStr[DMAX];
	if (strstr(str, "/home/timi") != NULL){

		strcpy(auxStr, "~");
		strcat(auxStr, (str+10));
		strcpy(str, auxStr);
	}
}
void Print_NameLine(){

	char absPath[DMAX];
	getcwd(absPath, sizeof(absPath));
	//strcpy(absPath, path);
	ReplaceString(absPath);
	printf("%s%s%s %s %s ", username, "@", username, absPath, "$");
}
void GetInstruction(){
	fgets(instruction, DMAX, stdin);
}////////////////////////////////

///////////////////Canale comunicare
void Fifo(){

    int fd, fd1;
    pid_t pid_fiu;

    if(-1 == (pid_fiu=fork())){
    	perror("Eroare la fork");
    	return;
  	}

  	if (pid_fiu == 0){ // fiu
		
		execl("DataFifo.bin", "DataFifo.bin", NULL);
  	}
	else{ //parinte

		int stat, cod_term;

		//send
		fd = open(FIFO_SEND, O_WRONLY);
		if (write(fd, instruction, strlen(instruction)) == -1){
		        perror("Problema la scriere in FIFO tata!");
		}
		close(fd);

		//receive
	    char received[DMAX]; int length;
	    mknod(FIFO_RECEIVE, S_IFIFO | 0666, 0); // 0666 este read - write
	    fd1 = open(FIFO_RECEIVE, O_RDONLY);
    	if ((length = read(fd1, received, DMAX)) == -1){
        	perror("Eroare la citirea din FIFO!");
    	}
    	received[length] = '\0';
 		printf("%s", received);
    	close(fd1);

 		stat = wait(&cod_term);
	    if ( WIFEXITED(cod_term) ){
	        printf("Rezultat: %d.\n", WEXITSTATUS(cod_term));
	    }
	    else{
	      perror("Eroare\n");
	    }
	}
}

void Execute(){

	switch(channel){
		case Pipes:
			//Pipe();
			break;
		case Fifos:
			Fifo();
			break;
		case Sockets:
			//Socket();
			break;
		default:
			break;
	}
}
/////////////////////////

int main(int argc, char *argv[]){

	
	if (SetChannel(argc, argv) == 0){
		return 0;
	}
	ReadCredentials();
	//while (!RequestCredentials());
	
	while(1){
		Print_NameLine(); 
		GetInstruction();
		Execute();
	}
	return 0;
}