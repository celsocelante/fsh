#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

// Número máximo de programas divididos por '@'
#define MAX_WORDS 10
#define MAX_ARGS 5
// Tamanho máximo da linha a ser lida na shell
#define SIZE_IN 1024
#define DELIMS " \t\r\n"
#define DELIM_F " @ "

// Função que trata o sinal SIGTSTP de cada filho
void trata_SIGTSTP(int signal){
	printf("Nao adianta tentar suspender... minha familia de processos esta protegida!\n");
	kill(getppid(),SIGUSR1);
}

/* quando há tentativa de bloquear um filho, o mesmo envia SIGUSR1 para o pai, 
que deve tratar esse sinal exibindo uma mensagem "Não adianta..." */
void trata_SIGUSR1(int signal){
	printf("Nao adianta tentar suspender... minha familia de processos esta protegida!\n");
}

// Função responsável pelo comando 'cd'
void cd_command(void){

	char *arg = strtok(NULL, DELIMS);
	if (!arg)
		printf("cd: Argumento obrigatorio\n");

	else 
		if (chdir(arg))
			printf("cd: O diretorio \"%s\" nao existe\n",arg);
}

// Função responsável pelo comando 'pwd'
void pwd_command(void){
	char cwd[1024];
   	if (getcwd(cwd, sizeof(cwd)) != NULL)
    	printf("pwd: %s\n", cwd);
	else
    	printf("pwd: Nao foi possivel obter o diretorio atual\n");
}

// Função responsável pelo comando 'waita'
void waitz_command(void){
	while(waitpid(-1, NULL, WNOHANG) > 0)
		printf("Morreu um zumbi!\n");
}

// Função responsável pelo comando 'exit'
void exit_command(void){
	// mata todos os filhos e encerra a execução
	while(waitpid(-1, NULL, WNOHANG) > 0);
	kill(0, SIGINT);
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
		signal(SIGINT, SIG_DFL);
		setsid(); // o processo gerente vai para background

		// caso um filho receba sinal de bloqueio, o mesmo envia SIGUSR1 para o pai
		signal(SIGUSR1,trata_SIGUSR1);

		pid_manager = getpid();
		
		for(i = 0; i < n; i++){ // iteração para criar n filhos
			if(getpid() == pid_manager){
				if(fork() == 0){ // código específico de cada filho
					signal(SIGTSTP,trata_SIGTSTP);

					char* command; // string para manter o comando completo recebido
					char* cmd;

					//printf("Hello, I'm the child number %d, my PID is %d and my father's PID is %d\n",i+1,getpid(),getppid());

					//command = (char *) malloc(strlen(comandos[i])+1);
					command = strdup(comandos[i]); // copia o comando completo recebido. PODE VAZAR MEMÓRIA! VERIFICAR!
					char* arg[MAX_ARGS + 1];
					int cont = 0;
					
					cmd = strtok (command," ");
					while (cmd != NULL){
						arg[cont] = cmd;
						cont++;
						cmd = strtok (NULL, " ");
						
					}
					arg[cont] = NULL;

					execvp(arg[0], arg);

					// suspende o processo em caso de erro
					printf("Erro de execucao\n");
					
					exit(0);
					//pause();
				}
			}
		}
		// código responsável por matar todos os filhos caso ao menos um seja encerrado
		pid = waitpid(-1, NULL, 0); // o processo gerente espera o retorno de ao menos um filho
		
		if(pid > 0)
			killpg(0, SIGINT); // mata todos os filhos caso um deles morra

		exit(0);
	}
}

int main(void){
	signal(SIGINT, SIG_IGN);
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
		   	} else if(strcmp(cmd,"waitz") == 0){
		   		waitz_command();
		   	} else if (strcmp(cmd, "exit") == 0) {
		    	exit_command();
		   	} else {
					cmd = strtok (command,"@");
					while (cmd != NULL) {
						cmd = trimwhitespace(cmd);
						proc[count] = (char*) malloc(strlen(cmd)+1);//FREE!!!!!!!!!!!!!
						strcpy(proc[count],cmd);
						count++;
						cmd = strtok (NULL, "@");
						
						if(count >= MAX_WORDS){
							break;
						}
						
					}
		   		// caso dos executáveis separados por '@'
		   		exec_children(count,proc);
		   		
		   	}
	    }
		count = 0;
		/* espera certo tempo para pedir entrada de novo
		isso evita sobreposição de "fsh> " */
		usleep(5000);

	}
	return 0;
}
