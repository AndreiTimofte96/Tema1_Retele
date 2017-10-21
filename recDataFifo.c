#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#define FIFO_RECEIVE "FIFO_R"
#define FIFO_SEND "FIFO_S"
#define DMAX 1001

char instruction[DMAX];
char tokens[DMAX][DMAX];
char path[DMAX] = "/home/timi";
char absPath[DMAX] = "/home/timi";
int tokensDim;

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
}


void ReceiveData(){

    int num, fd;
    mknod(FIFO_RECEIVE, S_IFIFO | 0666, 0); // 0666 este read - write

    fd = open(FIFO_RECEIVE, O_RDONLY);
    if ((num = read(fd, instruction, DMAX)) == -1){
        perror("Eroare la citirea din FIFO fiu!");
    }
    else {
        //printf("S-au citit din FIFO %d bytes: \"%s\"\n", num, s);
    }
    close(fd);

}

void SendData(int argc, char *argv[]){

    int fd1;
    //fd1 = open(FIFO_SEND, O_WRONLY);
    fd1 = open(FIFO_SEND, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    dup2(fd1, 1);
    dup2(fd1, 2);
    //close(fd1);
    execvp(argv[0], argv);
}

void Exec(){

    pid_t pid;      /* PID-ul procesului copil */
    int status;     /* starea de terminare a procesului copil */
    char *argv[DMAX];
    int argc;

    argc = tokensDim;   
    for (int i = 0; i < tokensDim; i++)
        argv[i] = tokens[i];
    argv[argc] = NULL;


    if (-1 == (pid = fork())){
        perror("Eroare la fork");
        return;
    }

    if (pid == 0){ //fiu 
        SendData(argc, argv);
    }   
    else{ //parinte
      
        if (wait (&status) < 0){
            perror ("wait()");
      }
        //printf ("Comanda a fost executata.\n"); 
    }
}


int main() 
{ 
    ReceiveData();
    Get_StrTok(instruction);

    /*if (strcmp(tokens[0], "cd") == 0){
            ChangeDirectory();
        }
        else{*/
            Exec(tokensDim, tokens); 
        //}
    
    exit(1);
    return 0;
}

