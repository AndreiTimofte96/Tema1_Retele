#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/socket.h>
#define LEN 30
#define DMAX 10001
#define FIFO_SEND "/home/timi/Documents/Retele/Tema1_Week5/FIFO_S"
#define FIFO_RECEIVE "/home/timi/Documents/Retele/Tema1_Week5/FIFO_R"

char x;
int isLogin = 0, logged = 0, tokensDim;
char username[LEN], password[LEN], realUsername[LEN], realPassword[LEN];
char instruction[DMAX], received[DMAX];
char tokens[DMAX][DMAX], path[DMAX] = "/home/timi", absPath[DMAX];
int sockp[2], sockPath[2], pipe_T[2], pipe_F[2];
int sz;
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
	df = fopen("/home/timi/Documents/Retele/Tema1_Week5/credentials.txt", "r");
	
	fscanf(df, "%s %s", realUsername, realPassword);
}
void RequestCredentials(){

	printf("%s", "Username: ");
	scanf("%s", username);
	scanf("%c", &x);
	printf("%s", "Password: ");
	scanf("%s", password);
	scanf("%c", &x);

	strcat(instruction, username);
	strcat(instruction, " ");
	strcat(instruction, password);

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
    	strcpy(path, "/home/timi");
    }
    else{
    	strcat(path, "/");
    	strcat(path, tokens[1]);
	}
    chdir(path);
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

	strcpy(absPath, path);
	ReplaceString(absPath);
	printf("%s%s%s %s %s ", username, "@", username, absPath, "$");
}
void GetInstruction(){
	fgets(instruction, DMAX, stdin);
}

void Son(){

	int fd_W, fd_R, path_R, path_W;
    pid_t pid_nepot;
    char *argv[DMAX];
    int argc, status;

	//primesc date de la tata
	close(sockPath[0]);
	path_R = sockPath[1];
	switch(channel){
		case Pipes:
			fd_R= pipe_T[0];
			break;
		case Fifos:
			mknod(FIFO_SEND, S_IFIFO | 0666, 0); // 0666 este read - write
			fd_R = open(FIFO_SEND, O_RDONLY);
			break;
		case Sockets:
			close(sockp[0]);
			fd_R = sockp[1];
			break;
		default:
			break;
	}

	int length;
	read(path_R, &sz, sizeof(int));
	if ((length = read(path_R, path, sz)) == -1){
    	perror("Eroare la citirea din FIFO fiu!");
	}

	read(fd_R, &sz, sizeof(int));
	if ((length = read(fd_R, instruction, sz)) == -1){
    	perror("Eroare la citirea din FIFO fiu!");
	}
	
	chdir(path);
	Get_StrTok(instruction);

	
  	//scriu date catre tata
    path_W = sockPath[1];
    switch(channel){
  		case Pipes:
  			fd_W = pipe_F[1];
  			break;
  		case Fifos:	
			fd_W = open(FIFO_RECEIVE, O_WRONLY);
			break;
		case Sockets:
			fd_W = sockp[1];
			break;
		default:
			break;
	}

	if (strcmp(tokens[0], "cd") == 0){
		ChangeDirectory();
		getcwd(path, sizeof(path));

		sz = strlen(path);
		write(path_W, &sz, sizeof(sz));
		if (write(path_W, path, sz) == -1){
			perror("Problema la scriere in FIFO tata! 1");
		}
		close(path_W);
		return;
	}
	else{	
		getcwd(path, sizeof(path));

		sz = strlen(path);
		write(path_W, &sz, sizeof(sz));
		if (write(path_W, path, sz) == -1){
	    	perror("Problema la scriere in FIFO tata! 2");
		}
		close(path_W);
	}

    argc = tokensDim;   
    for (int i = 0; i < tokensDim; i++)
        argv[i] = tokens[i];
    argv[argc] = NULL;


    
    
    if (-1 == (pid_nepot = fork())){
        perror("Eroare la fork");
        return;
    }
    if (pid_nepot == 0){ //NEPOT -> redirectez datele catre canal
		
		dup2(fd_W, 1); 
		dup2(fd_W, 2);		

		if (strcmp(argv[0], "myfind") == 0){
    		execl("/home/timi/Documents/Retele/Tema1_Week5/find.bin", "find.bin", argv[1], NULL);
    		execl("/home/timi/Documents/Retele/Tema1_Week5/stat.bin", "stat.bin", argv[1], NULL);
    		return;
		}

		if (strcmp(argv[0], "mystat") == 0){
    		execl("/home/timi/Documents/Retele/Tema1_Week5/stat.bin", "stat.bin", argv[1], NULL);
    		return;
		}

		if (strcmp(tokens[0], "login") == 0){
			ReadCredentials();
			logged = 0;
			if (strcmp(tokens[1], realUsername) == 0 && strcmp(tokens[2], realPassword) == 0){
				logged = 1;
			}
			if (write(fd_W, &logged, sizeof(logged)) == -1){
	        	perror("Problema la scriere in FIFO tata! 3");
			}
			close(fd_W);
  		}

		execvp(argv[0], argv);
		close(fd_W);
    }   
    else{ //FIU
    	
        if (wait (&status) < 0){
            perror ("wait()");
    	}

    	if (strcmp(tokens[0], "quit") == 0){
        kill(getppid(), SIGINT);
        exit(1);
  	}
    }
 	exit(1);
}

void Parent(){

	int fd_W, fd_R, path_W, path_R;
	int stat, cod_term, length;

	//trimite data catre fiu
	close(sockPath[1]);
	path_W = sockPath[0];
	switch(channel){
  		case Pipes:
  			fd_W= pipe_T[1];
			break;
		case Fifos:	
			fd_W = open(FIFO_SEND, O_WRONLY);
			break;
		case Sockets:
			close(sockp[1]);
			fd_W = sockp[0];
			break;
		default:
			break;
	}

	sz = strlen(path);
	write(path_W, &sz, sizeof(sz));
	if (write(path_W, path, sz) == -1){
	        perror("Problema la scriere in FIFO tata! 5");
	}

	sz = strlen(instruction);
	write(fd_W, &sz, sizeof(sz));
	if (write(fd_W, instruction, sz) == -1){
	        perror("Problema la scriere in FIFO tata! 4 ");
	}

	//receive
    //citeste date de la fiu
	//close(sockPath_F[0]);
	path_R = sockPath[0];
    switch(channel){
  		case Pipes:
  			fd_R= pipe_F[0];
			break;
		case Fifos:	
			mknod(FIFO_RECEIVE, S_IFIFO | 0666, 0); // 0666 este read - write
    		fd_R= open(FIFO_RECEIVE, O_RDONLY);
			break;
		case Sockets:
			fd_R = sockp[0];
			break;
		default:
			break;
	}

	//primesc pathul
	read(path_R, &sz, sizeof(int));
	if (read(path_R, path, sz) == -1){
       	perror("Eroare la citirea din FIFO10!");
   	}
   	//printf("%s\n", path);
   	close(path_R);

    if (isLogin == 1){

    	if ((length = read(fd_R, &logged, sizeof(int))) == -1){
        	perror("Eroare la citirea din FIFO11!");
    	}
    	
 		if (logged){
 			printf("%s\n", "Welcome!");
 		}
 		else{
 			printf("%s\n", "Wrong Credentials!");
 			strcpy(username, "");
 		}
    }
    else{
    	

    	//read(fd_R, &sz, sizeof(int));
    	if ((length = read(fd_R, received, DMAX)) == -1){
        	perror("Eroare la citirea din FIFO12!");
    	}

    	received[length] = '\0';
 		printf("%s", received);
 	}
	close(fd_R);

	stat = wait(&cod_term);
    if ( WIFEXITED(cod_term) ){
    //	   printf("Rezultat: %d.\n", WEXITSTATUS(cod_term));
    }
    else{
      perror("Eroare\n");
    }
}

void Execute(){

    int fd_W, fd_R;
    pid_t pid_fiu;


    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockPath) < 0 ) {		
	        perror("Err... socketpair"); 
	        exit(1); 
	}

    if(channel == Sockets){

	    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0 ) {		
	        perror("Err... socketpair"); 
	        exit(1); 
	    }   
    }

    if(channel == Pipes){
		/* creare pipe intern */
		  if (-1 == pipe(pipe_T) ){
		    perror("Eroare la crearea canalului intern!");
		    exit(1);
		  }

		  if (-1 == pipe(pipe_F) ){
		    perror("Eroare la crearea canalului intern!");
		    exit(1);
		  }
    }

    if(-1 == (pid_fiu=fork())){
    	perror("Eroare la fork");
    	return;
  	}

  	if (pid_fiu == 0){
  		Son();
  	}
	else{ 
		Parent();
	}		
}

void LoginProtocol(){

	if (logged == 1){
		Execute();
	}
	else{ 
		if (strstr(instruction, "login") != NULL){
			isLogin = 1;
			RequestCredentials(); 
			Execute();
			isLogin = 0;
		}
		else{
			if (strstr(instruction, "quit") != NULL){
				Execute();
			}	
			else{
				printf("%s\n", "Access Denied");
			}
	 	}
	 }
}

int main(int argc, char *argv[]){

	if (SetChannel(argc, argv) == 0){
		return 0;
	}

	while(1){
		Print_NameLine(); 
		GetInstruction();
		//LoginProtocol();		
		Execute();

	}
	return 0;
}
/*
de facut in continuare: 
1) stergerea fisierelor fifo - optional - marti
2) prefixare comenzi dimensiunea lor - marti
3) cd - duminica - DONE
4) adaugare pipeuri - marti
5) my_stat - duminica - DONE 
6) my_stat de adaugat la find. - duminica - DONE
*/
