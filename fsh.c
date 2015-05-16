#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include "lista.h"

// Número máximo de programas divididos por '@'
#define MAX_WORDS 10
// Número máximo de argumentos por programa
#define MAX_ARGS 5
// Tamanho máximo da linha a ser lida na shell
#define SIZE_IN 1024
#define DELIMS " \t\r\n"
#define DELIM_COMANDOS "@"
#define WAIT_ANY -1

// Variável global que conta os processos gerentes vivos
int qtd_gerentes = 0;

// Ponteiro global para a lista de gerentes
Lista* lista_gerentes;

// Função que trata o sinal SIGTSTP de cada gerente
void trata_SIGTSTP_gerente(int signal){
	printf("Nao adianta tentar suspender... minha familia de processos esta protegida!\n");
}

void trata_SIGUSR1_gerente(int sig){
	killpg(0, SIGKILL);
}

// Tratamento de sinal SIGTSTP da fsh: repassa o sinal para todos os gerentes
void trata_SIGTSTP_fsh(int signal){
	Lista* p;
	// Repassa o sinal SIGTSTP para todos os processos gerentes
	for(p = lista_gerentes; p != NULL; p = p->prox){
        kill(p->pid, SIGTSTP);
	}
}

// Tratamento do signal SIGINT, que somente aceita caso a fsh nao tenha nenhum filho
void trata_SIGINT(int sig){
	// Verifica a quantidade de processos gerentes vivos
    if(qtd_gerentes == 0){
    	signal(SIGINT, SIG_DFL);
        raise(SIGINT);
    }
}

// Decrementa o contador de processos gerentes vivos
void trata_SIGCHLD(int signal){
	qtd_gerentes = qtd_gerentes - 1;
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
    	printf("%s\n", cwd);
	else
    	printf("pwd: Nao foi possivel obter o diretorio atual\n");
}

// Função responsável pelo comando 'waitz'
void waitz_command(void){
	while(waitpid(WAIT_ANY, NULL, WNOHANG) > 0)
		printf("Morreu um zumbi!\n");
}

// Função responsável pelo comando 'exit'
void exit_command(void){
	Lista* p;

	// Envia comando de encerramento para todos os gerentes, que tratam e matam seus filhos
	for(p = lista_gerentes; p != NULL; p = p->prox){
        kill(p->pid, SIGUSR1);
	}

	// Libera todos os filhos zumbis
	while(waitpid(WAIT_ANY, NULL, WNOHANG) > 0);

	lst_libera(lista_gerentes);
	exit(0);
}

char *trimwhitespace(char *str){
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

	if(pid > 0){
		// adiciona pid do processo gerente a lista de pids (filhos da fsh)
		lista_gerentes = lst_insere(pid,lista_gerentes);

		// Incrementa contador global de processos gerentes vivos
		qtd_gerentes++;
	}

	if(pid == 0){ // no caso do gerente

		// tratador de sinal SIGTSTP dos processos gerentes
		signal(SIGTSTP, trata_SIGTSTP_gerente);

		// tratador de sinal SIGKILL (enviado pelo comando 'exit' da fsh)
		signal(SIGUSR1, trata_SIGUSR1_gerente);

		// o processo gerente vai para background (e seus filhos)
		setsid();

		pid_manager = getpid();
		
		for(i = 0; i < n; i++){ // iteração para criar n filhos
			if(getpid() == pid_manager){
				if(fork() == 0){ // código específico de cada filho
					char* command; // string para manter o comando completo recebido
					char* cmd;

					//command = (char *) malloc(strlen(comandos[i])+1);
					command = strdup(comandos[i]); // copia o comando completo recebido. PODE VAZAR MEMÓRIA! VERIFICAR!
					char* arg[MAX_ARGS + 1];

					// contador de argumentos
					int cont = 0;
					
					// separa os argumentos do executável por espaço
					cmd = strtok (command," ");
					while (cmd != NULL){
						arg[cont] = cmd;
						cont++;
						cmd = strtok (NULL, " ");
						
					}
					// Seta NULL para último argumento + 1
					arg[cont] = NULL;

					// Bloqueando sinal SIGTSTP
					sigset_t mascara;
					sigemptyset(&mascara);
					sigaddset(&mascara, SIGTSTP);
					sigprocmask(SIG_BLOCK, &mascara, NULL);

					// substitui o código do executável
					execvp(arg[0], arg);

					printf("Erro de execucao\n");
					
					// Encerra o processo em caso de erro
					exit_command();
				}
			}
		}
		// Responsável por matar todos os filhos caso ao menos um seja encerrado
		if(waitpid(WAIT_ANY, NULL, 0) > 0)
			// envia SIGKILL a todos do grupo
			killpg(0, SIGKILL);

		exit_command();
	}
}

int main(void){
	int count = 0, i;

	char *cmd;
  	char line[SIZE_IN];
  	char command[SIZE_IN];
	char *proc[10];

	// Cria lista de processos gerentes
	lista_gerentes = lst_cria();

	// Tratadores de sinais da fsh
	signal(SIGTSTP, trata_SIGTSTP_fsh);
	signal(SIGCHLD, trata_SIGCHLD);
	signal(SIGINT, trata_SIGINT);

	while(1){
		printf("fsh> ");

		// se houver erro na captura da linha digitada
		if (!fgets(line, SIZE_IN, stdin))
			exit_command();

		// faz cópia da entrada do usuário
		strcpy(command,line);

	    // interpreta os comandos, primeiro verificando os comandos básicos
	    if ((cmd = strtok(line, DELIMS))) {

			if (strcmp(cmd, "cd") == 0) {
				cd_command();
			} else if(strcmp(cmd,"pwd") == 0){
		   		pwd_command();
		   	} else if(strcmp(cmd,"waitz") == 0 || strcmp(cmd,"waita") == 0){
		   		waitz_command();
		   	} else if (strcmp(cmd, "exit") == 0) {
		    	exit_command();
		   	} else {
					cmd = strtok (command, DELIM_COMANDOS);
					while (cmd != NULL) {
						cmd = trimwhitespace(cmd);
						proc[count] = (char*) malloc(strlen(cmd)+1);//FREE!!!!!!!!!!!!!
						strcpy(proc[count],cmd);
						count++;
						cmd = strtok (NULL, DELIM_COMANDOS);
						
						if(count >= MAX_WORDS){
							break;
						}
						
					}
		   		// Chama a função que cria o processo gerente e seus filhos
		   		exec_children(count,proc);
		   		
		   	}
	    }

	    // Reinicia o contador de executáveis
		count = 0;
		/* espera certo tempo para pedir entrada de novo
		isso evita sobreposição de "fsh> " */
		usleep(5000);

	}
	exit_command();
	return 0;
}