typedef struct proc {
	int pid;
	struct proc* prox;
} Lista;

Lista* lst_cria(void){
	return NULL;
}

Lista* lst_insere(int pid, Lista* lista){
	Lista* novo = (Lista*) malloc(sizeof(Lista));
	novo->pid = pid;
	novo->prox = lista;

	return novo;
}

void lst_libera(Lista* lista){
	Lista* p = lista;
	while(p != NULL){
		Lista* t = p->prox;
		free(p);
		p = t;
	}
}

