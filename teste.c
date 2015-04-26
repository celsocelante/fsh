#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

int main(void){
	int x = 0;
	int i, pid_manager;
	printf("Hello, I'm the father and my PID is %d\n",getpid());

	int pid = fork(); // cria processo gerente

	/*if(pid > 0)
		wait(); // faz o pai esperar o retorno do filho*/

	if(pid == 0){ // no caso do gerente
		pid_manager = getpid();
		printf("Hello, I'm the manager process, my PID is %d and my father's PID is %d\n",getpid(),getppid());
		
		for(i = 0; i < 5; i++){
			if(getpid() == pid_manager){
				if(fork() == 0){
					// código específico de cada filho
					printf("Hello, I'm the child number %d, my PID is %d and my father's PID is %d\n",i+1,getpid(),getppid());
					execl ("/bin/ping", "google.com", NULL);
					//break;
					//exit(0);

				}
			}
		}
		// execução do processo gerente
		for(;;);
		exit(0);
	}
	// execução do processo pai supremo
	for(;;);
	return 0;
}