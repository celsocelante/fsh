#include <stdio.h>

typedef struct proc {
	int pid;
	struct proc* prox;
} Proc;

Proc* criaProc(){
	return NULL;
}

Proc* addProc(int pid, Proc* lista){
	Proc* item;
	item = (Proc*) malloc(sizeof(Proc));
	item->pid = pid;
	item->prox = lista;

	return item;
}

