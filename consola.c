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
int isLogin = 0, logged = 0, tokensDim, isCd = 0, firstCd = 0;
char username[LEN], password[LEN], realUsername[LEN], realPassword[LEN];
char instruction[DMAX], received[DMAX];
char tokens[DMAX][DMAX], path[DMAX] = "/home/timi", absPath[DMAX];
int sockp_T[2], sockp_F[2], sockPath_T[2], sockPath_F[2];

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
    char sonPath[DMAX], pipeRead[DMAX];
    int pfd[2]; 	

	//primesc date de la tata
	close(sockPath_T[0]);
	path_R = sockPath_T[1];

	switch(channel){
		case Pipes:
			break;
		case Fifos:
			mknod(FIFO_SEND, S_IFIFO | 0666, 0); // 0666 este read - write
			fd_R = open(FIFO_SEND, O_RDONLY);
			break;
		case Sockets:
			close(sockp_T[0]);
			fd_R = sockp_T[1];
			break;
		default:
			break;
	}

	if (read(fd_R, instruction, DMAX) == -1){
    	perror("Eroare la citirea din FIFO fiu!");
	}

	if (read(path_R, sonPath, DMAX) == -1){
    	perror("Eroare la citirea din FIFO fiu!");
	}

	strcpy(path, sonPath);
	chdir(path);
	close(fd_R);
	close(path_R);
	
	Get_StrTok(instruction);
	if (strcmp(tokens[0], "quit") == 0){
        kill(getppid(), SIGINT);
        exit(1);
  	}

    argc = tokensDim;   
    for (int i = 0; i < tokensDim; i++)
        argv[i] = tokens[i];
    argv[argc] = NULL;

    //Trimit date catre tata
    switch(channel){
  			case Pipes:
  				break;
  			case Fifos:	
  				fd_W = open(FIFO_RECEIVE, O_WRONLY);
    			break;
    		case Sockets:
    			close(sockp_F[1]);
    			fd_W = sockp_F[0];
    			break;
    		default:
    			break;
	}
	close(sockPath_F[1]);
    path_W = sockPath_F[0];

	if (strcmp(tokens[0], "cd") == 0){
        ChangeDirectory();
        getcwd(path, sizeof(path));
        if (write(path_W, path, sizeof(path)) == -1){
	       	perror("Problema la scriere in FIFO tata! 1");
		}
		close(path_W);
		return;
	}
	else{
		getcwd(path, sizeof(path));
		if (write(path_W, path, sizeof(path)) == -1){
        	perror("Problema la scriere in FIFO tata! 2");
		}
		close(path_W);
	}

	 //cream pipeul de comunicare intre Son1 si Son
    if (pipe (pfd) == -1){
      fprintf (stderr, "pipe\n");
      exit (1);
    }

    if (-1 == (pid_nepot = fork())){
        perror("Eroare la fork");
        return;
    }

    if (pid_nepot == 0){ //NEPOT -> redirectez datele catre canal
		
		dup2(pfd[1], 1); 
		dup2(pfd[1], 2);
		close (pfd[0]);
     
		if (strcmp(argv[0], "myfind") == 0){
    		execl("/home/timi/Documents/Retele/Tema1_Week5/find.bin", "find.bin", argv[1], NULL);
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
			if (write(pfd[1], &logged, sizeof(logged)) == -1){
	        	perror("Problema la scriere in FIFO tata! 3");
			}
			close (pfd[1]);
  		}

		execvp(argv[0], argv);
		close(pfd[1]);
    }   
    else{ //FIU
    	
		if (wait (&status) < 0){
            perror ("wait()");
    	}

    	close(pfd[1]);
    	//read(pfd[0], pipeRead, DMAX);

    	char ch;
    	int index;
    	while( read(pfd[0], &ch, 1) != 0){
      		if(index < DMAX){
        		pipeRead[index++] = ch;
      		}
    	}
    	//pipeRead[(index == DMAX) ? DMAX-1 : index ] = '\0';

    	close(pfd[0]);
    	write(fd_W, pipeRead, sizeof(pipeRead));
		close(fd_W);
	}
    exit(1);
}

void Parent(){

	int fd_W, fd_R, path_W, path_R;
	int stat, cod_term, length;

	//trimite data catre fiu

	close(sockPath_T[1]);
	path_W = sockPath_T[0];

	switch(channel){
  		case Pipes:
				break;
			case Fifos:	
				fd_W = open(FIFO_SEND, O_WRONLY);
			break;
		case Sockets:
			close(sockp_T[1]);
			fd_W = sockp_T[0];
			break;
		default:
			break;
	}

	if (write(fd_W, instruction, sizeof(instruction)) == -1){
	        perror("Problema la scriere in FIFO tata! 4 ");
	}

	if (write(path_W, path, sizeof(path)) == -1){
	        perror("Problema la scriere in FIFO tata! 5");
	}
	
	close(path_W);
	close(fd_W);

	//receive
    //citeste date de la fiu
	close(sockPath_F[0]);
	path_R = sockPath_F[1];

    switch(channel){
  		case Pipes:
				break;
			case Fifos:	
				mknod(FIFO_RECEIVE, S_IFIFO | 0666, 0); // 0666 este read - write
    		fd_R= open(FIFO_RECEIVE, O_RDONLY);
			break;
		case Sockets:
			close(sockp_F[0]);
			fd_R = sockp_F[1];
			break;
		default:
			break;
	}

	//primesc pathul
	if (read(path_R, &path, DMAX) == -1){
       	perror("Eroare la citirea din FIFO!");
   	}
   	//printf("%s\n", path);
   	close(path_R);

    if (isLogin == 1){

    	if ((length = read(fd_R, &logged, sizeof(int))) == -1){
        	perror("Eroare la citirea din FIFO!");
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
    
    	if ((length = read(fd_R, received, DMAX)) == -1){
        	perror("Eroare la citirea din FIFO!");
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

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp_T) < 0 ) {		
        perror("Err... socketpair"); 
        exit(1); 
    }
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp_F) < 0 ) {		
        perror("Err... socketpair"); 
        exit(1); 
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockPath_T) < 0 ) {		
        perror("Err... socketpair"); 
        exit(1); 
    }
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockPath_F) < 0 ) {		
        perror("Err... socketpair"); 
        exit(1); 
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
