#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define SIZE_IN 1024
#define MAX_WORDS 10

char input[SIZE_IN] = {'\0'};

void getText(char *string){
	printf("fsh> ");
	fgets(string, sizeof(char)*SIZE_IN, stdin);
	sscanf(string, "%[^\n]", string);
}

int main(){
	int count = 0;
	int i;
	char *buffer, *words[MAX_WORDS], *ptrTok;
	char cwd[SIZE_IN];

	getcwd(cwd, sizeof(cwd));

	while(1){
		// Texto de espera de entrada do fsh
		getText(input);

		// Operações internas da shell
		if(strcmp(input,"cd") == 0){

		} else if(strcmp(input,"pwd") == 0){
			printf("%s\n",cwd);
		} else if(strcmp(input,"waita") == 0){

		} else if(strcmp(input,"exit") == 0){
			return 0;
		} else {
			buffer = strdup(input);
		    if (!buffer)
		    	return 1;

		    while((ptrTok = strsep(&buffer, " ")) && count < MAX_WORDS)
	    		words[count++] = ptrTok;

		    // imprime todos os comandos de entrada
		    for (i = 0; i < count; i++)
		    	printf("%s\n", words[i]);

		    // Zera o contador de comandos separados por '@'
		    count = 0;

		    free(buffer);
		}
		
	}
	return 0;
}