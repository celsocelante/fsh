#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

// Número máximo de programas divididos por '@'
#define MAX_WORDS 10
// Tamanho máximo da linha a ser lida na shell
#define SIZE_IN 1024
#define DELIMS " \t\r\n"
#define DELIM_F " @ "

// Função que trata o sinal TSTP
void trata_SIGTSTP(int signal){
	printf("Nao adianta tentar suspender... minha familia de processos esta protegida!\n");
}

// Função responsável pelo comando 'cd'
void cd_command(void){

	char *arg = strtok(NULL, DELIMS);
	if (!arg)
		printf("cd: Argument is required\n");

	else 
		if (chdir(arg))
			printf("cd: The directory \"%s\" does not exist\n",arg);
}

// Função responsável pelo comando 'pwd'
void pwd_command(void){
	char cwd[1024];
   	if (getcwd(cwd, sizeof(cwd)) != NULL)
    	printf("pwd: %s\n", cwd);
	else
    	printf("pwd: Cannot get current directory\n");
}

// Função responsável pelo comando 'waita'
void waita_command(void){
	// faz nada
}

// Função responsável pelo comando 'exit'
void exit_command(void){
	exit(0);
}

char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace(*str))
	str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

// Função experimental que cria n filhos e um processo gerente
void exec_children(int n, char** comandos){
	int i, pid_manager;
	char* cmd;
	int pid = fork(); // cria processo gerente

	if(pid == 0){ // no caso do gerente
		setsid(); // o processo gerente vai para background
		pid_manager = getpid();
		
		for(i = 0; i < n; i++){ // iteração para criar n filhos
			if(getpid() == pid_manager){
				if(fork() == 0){ // código específico de cada filho

					char* command; // string para manter o comando completo recebido
					//char [6][256]; // vetor de strings para manter todos os argumentos
					char* cmd;

					printf("Hello, I'm the child number %d, my PID is %d and my father's PID is %d\n",i+1,getpid(),getppid());

					command = (char *) malloc(strlen(comandos[i])+1);
					command = strdup(comandos[i]); // copia o comando completo recebido
					
					cmd = strtok (command," ");
					while (cmd != NULL){
						
						printf ("%s\n",command);
						cmd = strtok (NULL, " ");
						
					}

					exit(0); // encerra o filho

					// usar int execvp(const char *file, char *const argv[]);
				}
			}
		}
		// o processo gerente faz n wait() para os n filhos
		for(i=0; i<n; i++) 		
			wait();
		exit(0);
	}
}

int main(void){
	int count = 0, i;

	char *cmd;
  	char line[SIZE_IN];
  	char command[SIZE_IN];
	char *proc[10];

	while(1){
		printf("fsh> ");

		// se houver erro na captura da linha digitada
		if (!fgets(line, SIZE_IN, stdin))
			exit(1);

		// faz cópia da entrada do usuário
		strcpy(command,line);

	    // interpreta os comandos, primeiro verificando os comandos básicos
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
					cmd = strtok (command,"@");
					while (cmd != NULL)
					{
						cmd = trimwhitespace(cmd);
						proc[count] = (char*) malloc(strlen(cmd)+1);
						strcpy(proc[count],cmd);
						printf ("%s\n",proc[count]);
						count++;
						cmd = strtok (NULL, "@");
// lembrar de parar o while na decima iteracao para evitar invadir espaco de memoria
						
					}

					printf("%d argumentos\n",count);
		   		// caso dos executáveis separados por '@'
		   		exec_children(count,proc);
		   	}
	    }
		count = 0;

	}
	return 0;
}
