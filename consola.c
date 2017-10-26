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
#define FIFO_SEND "FIFO_S"
#define FIFO_RECEIVE "FIFO_R"

char x;
char username[LEN], password[LEN], realUsername[LEN], realPassword[LEN];
char instruction[DMAX], path[DMAX] = "/home/timi";
char tokens[DMAX][DMAX], path[DMAX], absPath[DMAX], string[DMAX];
int tokensDim;

enum Channel{Pipes, Fifos, Sockets} channel;


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
}
void Get_StrTok(char *instr){

    char * p;
    tokensDim = 0;
    p = strtok (instr, " \n");
    while (p != NULL){  
        
        strcpy(tokens[tokensDim++], p);
        p = strtok (NULL, " \n");
    }
}

void ChangeDirectory(){

    if (tokensDim == 1){

        chdir("/home/timi");
        return;
    }

    strcat(path, "/");
    strcat(path, tokens[1]);
    chdir(path);
    printf("%s\n", path);
}

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
}

void Fifo(){

    int fd_W, fd_R;
    pid_t pid_fiu, pid_nepot;

    if(-1 == (pid_fiu=fork())){
    	perror("Eroare la fork");
    	return;
  	}

  	if (pid_fiu == 0){ // fiu 

  		//primesc date de la tata
    	mknod(FIFO_SEND, S_IFIFO | 0666, 0); // 0666 este read - write
    	fd_R = open(FIFO_SEND, O_RDONLY);
    	if (read(fd_R, instruction, DMAX) == -1){
        	perror("Eroare la citirea din FIFO fiu!");
    	}
    	close(fd_R);

    	Get_StrTok(instruction);
    	if (strcmp(tokens[0], "quit") == 0){
	        kill(getppid(), SIGINT);
	        exit(1);
	    }
	    /*if (strcmp(tokens[0], "cd") == 0){
	        ChangeDirectory();
	        exit(1);
	    }*/
    	char *argv[DMAX];
    	int argc;

	    argc = tokensDim;   
	    for (int i = 0; i < tokensDim; i++)
	        argv[i] = tokens[i];
	    argv[argc] = NULL;

	    if (-1 == (pid_nepot = fork())){
	        perror("Eroare la fork");
	        return;
	    }

	    if (pid_nepot == 0){ //nepot -> redirectez datele catre canal
	
    		fd_W = open(FIFO_RECEIVE, O_WRONLY);
    		dup2(fd_W, 1);
    		dup2(fd_W, 2);
    		if (strcmp(argv[0], "myfind") == 0){
        		execl("find.bin", "find.bin", argv[1], NULL);
        		return;
    		}
    		execvp(argv[0], argv);
    		close(fd_W);
	    }   
	    else{ //fiu  	
	      	int status;
	        if (wait (&status) < 0){
	            perror ("wait()");
	    	}
	    }
	    exit(1);
  	}
	else{ //parinte

		int stat, cod_term;

		//send
		fd_W = open(FIFO_SEND, O_WRONLY);
		if (write(fd_W, instruction, strlen(instruction)) == -1){
		        perror("Problema la scriere in FIFO tata!");
		}
		close(fd_W);

		//receive
	    char received[DMAX]; int length;
	    mknod(FIFO_RECEIVE, S_IFIFO | 0666, 0); // 0666 este read - write
	    fd_R= open(FIFO_RECEIVE, O_RDONLY);
    	if ((length = read(fd_R, received, DMAX)) == -1){
        	perror("Eroare la citirea din FIFO!");
    	}
    	received[length] = '\0';
 		printf("%s", received);
    	close(fd_R);

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
	//ReadCredentials();
	//while (!RequestCredentials());
	
	while(1){
		Print_NameLine(); 
		GetInstruction();
		Execute();
	}
	return 0;
}