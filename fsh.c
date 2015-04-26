#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define MAX_WORDS 10
#define SIZE_IN 1024
#define DELIMS " \t\r\n"
#define DELIM_F " @ "

char* escape_white(char* str){
	while(isspace(str))
		str++;

	return str;
}

void cd_command(void){

	char *arg = strtok(NULL, DELIMS);
	if (!arg)
		printf("cd: Argument is mandatory\n");

	else 
		if (chdir(arg))
			printf("cd: The directory \"%s\" does not exist\n",arg);
}

void pwd_command(void){
	char cwd[1024];
   	if (getcwd(cwd, sizeof(cwd)) != NULL)
    	printf("pwd: %s\n", cwd);
	else
    	printf("pwd: Cannot get current directory\n");
}

void waita_command(void){
}

void exit_command(void){
	exit(0);
}

void exec_children(int n){
	int i, pid_manager;

	int pid = fork(); // cria processo gerente

	if(pid == 0){ // no caso do gerente
		pid_manager = getpid();
		
		for(i = 0; i < 5; i++){
			if(getpid() == pid_manager){
				if(fork() == 0){
					// código específico de cada filho
					//printf("Hello, I'm the child number %d, my PID is %d and my father's PID is %d\n",i+1,getpid(),getppid());
					execl("/bin/ls", "ls", NULL);
					//for(;;);
				}
			}
		}
		// execução do processo gerente
		wait();
	}
}

int main(void){
	int count = 0, i;

	char *cmd;
  	char line[SIZE_IN];
  	char command[SIZE_IN];

	while(1){
		printf("fsh> ");
		if (!fgets(line, SIZE_IN, stdin))
			exit(1);

		strcpy(command,line);

	    // Parse and execute command
	    if ((cmd = strtok(line, DELIMS))) {

			if (strcmp(cmd, "cd") == 0) {
				cd_command();
			} else if(strcmp(cmd,"pwd") == 0){
		   		pwd_command();
		   	} else if(strcmp(cmd,"waita") == 0){
		   		waita_command();
		   	} else if (strcmp(cmd, "exit") == 0) {
		    	exit_command();
		   	} else {
		   		exec_children(5);
		   	}
	    }

	}
	return 0;
}