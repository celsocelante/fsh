/* 
Grupo: Celso Ademar Celante Junior
	   Igor Silva Epitácio Pereira

Disciplina: Sistemas Operacionais
Ano/período: 2015/1
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

// Número máximo de programas divididos por '@'
#define MAX_WORDS 10

// Número máximo de argumentos por programa
#define MAX_ARGS 5

// Tamanho máximo da linha a ser lida na shell
#define SIZE_IN 1024

// Tamanho máximo de cada comando
#define SIZE_CMD 100

// Delimitadores do strtok
#define DELIMS " \t\r\n"
#define DELIM_COMANDOS "@"

// A ser usada junto a função waitpid()
#define WAIT_ANY -1

// Para a função exit_command, que permite encerramento seguro, seja reconhecida por todos
void exit_command(void);

// Estrutura dos nós da lista encadeada
typedef struct proc {
	int pid;
	struct proc* prox;
} Lista;

// Função de criação da lista encadeada
Lista* lst_cria(void){
	return NULL;
}

// Função de inserção na lista encadeada
Lista* lst_insere(int pid, Lista* lista){
	Lista* novo = (Lista*) malloc(sizeof(Lista));
	novo->pid = pid;
	novo->prox = lista;

	return novo;
}

// Função que libera a lista encadeada da memória
void lst_libera(Lista* lista){
	Lista* p = lista;
	while(p != NULL){
		Lista* t = p->prox;
		free(p);
		p = t;
	}
}

// Variável global que conta os processos gerentes vivos
int qtd_gerentes = 0;

// Ponteiro global para a lista de gerentes
Lista* lista_gerentes;

// Função que trata o sinal SIGTSTP de cada gerente
void trata_SIGTSTP_gerente(int signal){
	printf("Nao adianta tentar suspender... minha familia de processos esta protegida!\n");
}

// Trata o sinal que o processo gerente recebo da fsh para encerrar todos os seus filhos
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
void trata_SIGINT_fsh(int sig){
	// Verifica a quantidade de processos gerentes vivos
    if(qtd_gerentes == 0){
    	signal(SIGINT, SIG_DFL);
        raise(SIGINT);
        //exit_command(); // seria mais seguro, pois liberaria memória do monte
    }
}

// Decrementa o contador de processos gerentes vivos
void trata_SIGCHLD_fsh(int signal){
	qtd_gerentes = qtd_gerentes - 1;
}

// Função responsável pelo comando 'cd'
void cd_command(void){
	char* arg = strtok(NULL, DELIMS);
	if (!arg)
		printf("cd: Argumento obrigatorio\n");
	else if (chdir(arg))
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
	int pid = 0;
	while((pid = waitpid(WAIT_ANY, NULL, WNOHANG)) > 0)
		printf("Processo zumbi com PID %d liberado!\n",pid);
}

// Função responsável pelo comando 'exit'
void exit_command(void){
	Lista* p;

	// Envia comando de encerramento (no caso, SIGURS1) para todos os gerentes, que tratam e matam seus filhos
	for(p = lista_gerentes; p != NULL; p = p->prox){
        kill(p->pid, SIGUSR1);
	}

	// Libera a lista de gerentes global
	lst_libera(lista_gerentes);

	// Libera todos os filhos zumbis, assim como o waitz(a)
	while(waitpid(WAIT_ANY, NULL, WNOHANG) > 0);

	// Finalmente encerra a fsh
	exit(0);
}

char* trimstring(char* str){
	char* fim;
	// Remove os espaços iniciais
	while(isspace(*str))
		str++;

	// Se a string for vazia, retornar a própria
	if(*str == 0)
		return str;

	// Corta espaços inúteis 
	fim = str + strlen(str) - 1;

	while(fim > str && isspace(*fim))
		fim--;

	// Insere o terminal nulo da string no fim
	*(fim+1) = '\0';

	return str;
}

// Função que cria n filhos e um processo gerente
void exec_children(int n, char comandos[MAX_WORDS][SIZE_CMD]){
	int i, pid_manager;
	char* cmd;
	int pid = fork(); // cria processo gerente

	// Caso o fork() dê erro...
	if(pid < 0){
		printf("Problema no fork! Encerrando...\n");
		exit(1);
	}

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
				int pid = fork();

				// Caso o fork() dê erro...
				if(pid < 0){
					printf("Problema no fork()! Encerrando...\n");
					exit(1);
				}

				if(pid == 0){ // código específico de cada filho
					char command[SIZE_CMD]; // string para manter o comando completo recebido
					char* cmd;

					strcpy(command,comandos[i]); // copia o comando completo recebido
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

					// Ignora o sinal SIGTSTP
					signal(SIGTSTP, SIG_IGN);

					// substitui o código do executável
					execvp(arg[0], arg);

					printf("Erro de execucao. Minha familia sera morta!\n");
					
					// Encerra o processo em caso de erro
					exit(0);
				}
			}
		}
		/* Resto do código do processo gerente
		   Responsável por matar todos os filhos caso ao menos um seja encerrado */

		if(waitpid(WAIT_ANY, NULL, 0) > 0)
			// envia SIGKILL a todos do grupo
			killpg(0, SIGKILL);

		exit(0);
	}
}

int main(void){
	// Contadores
	int count = 0, i;

	// Ponteiro para o strtok
	char *cmd;

	// Linha de comandos inserida pelo usuário
  	char line[SIZE_IN];

  	// Cópia da linha de comandos inseria pelo usuário (evita problemas do strtok)
  	char command[SIZE_IN];

  	// Vetor de comandos
	char proc[MAX_WORDS][SIZE_CMD];

	// Cria lista de processos gerentes
	lista_gerentes = lst_cria();

	// Tratadores de sinais da fsh
	signal(SIGTSTP, trata_SIGTSTP_fsh); // repassa o CTRL+Z para os processos gerentes
	signal(SIGCHLD, trata_SIGCHLD_fsh); // decrementa o contador de filhos/gerentes vivos
	signal(SIGINT, trata_SIGINT_fsh); // determina se aceita ou não o CTRL+C

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
		   	// A especificação está confusa, não dá para saber qual é o nome do comando, então a fsh aceita ambos
		   	} else if(strcmp(cmd,"waitz") == 0 || strcmp(cmd,"waita") == 0){
		   		waitz_command();
		   	} else if (strcmp(cmd, "exit") == 0) {
		    	exit_command();
		   	} else {
		   			// "Quebra" a string por delimitador "@"
					cmd = strtok (command, DELIM_COMANDOS);
					while (cmd != NULL) {
						// Remove o espaço em excesso das extremidades
						cmd = trimstring(cmd);

						// Preenche o vetor de comandos executáveis com cada exec. e seus args
						strcpy(proc[count],cmd);

						count++;
						cmd = strtok (NULL, DELIM_COMANDOS);
						
						// Garante que só 10 comandos, no máximo, serão lidos
						if(count >= MAX_WORDS){
							break;
						}
						
					}
		   		// Chama a função que cria o processo gerente e seus filhos
		   		exec_children(count,proc);
		   		
		   	}
	    }

	    // Reinicia o contador de comandos executáveis
		count = 0;

		/* espera certo tempo para pedir entrada de novo
		isso evita sobreposição de "fsh> " */
		usleep(6000);

	}
	// Encerra a fsh, liberando filhos e recursos
	exit_command();
}