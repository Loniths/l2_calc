#include "str.h"
#include "lista.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct no *No;
struct no{
    dado_t dado;
    No prox;
    No ant;
};

struct lista{
    int tam;
    No sent;
};

static No no_cria(){
    No novo = malloc(sizeof(struct no));
    assert(novo != NULL);
    return novo;
}

Lista l_cria(){
    Lista l = malloc(sizeof(struct lista));
    assert(l != NULL);
    l->sent = no_cria();
    l->sent->prox = l->sent;
    l->sent->ant = l->sent;
    l->sent->dado = NULL;
    l->tam = 0;
    return l;
}

int l_tam(Lista l){
    return l->tam;
}

bool l_vazia(Lista l){
    return l->sent->prox == l->sent;
}

bool l_cheia(Lista l){
    return false;
}

void l_imprime(Lista l){
    No guia = l->sent->prox;
    for(int i = 0; i < l->tam; i++){
        if(i > 0) putchar(' ');
        putchar('[');
        s_imprime(guia->dado);
        putchar(']');
        guia = guia->prox;
    }
}

void l_insere_pos(Lista l, dado_t dado, int pos){
    assert(pos >= 0 && pos <= l->tam);
    No proximo = l->sent->prox;
    for(int i = 0; i < pos; i++) proximo = proximo->prox;
    No novo = no_cria();
    novo->dado = dado;
    novo->prox = proximo;
    novo->ant = proximo->ant;
    proximo->ant->prox = novo;
    proximo->ant = novo;
    l->tam++;
}

dado_t l_dado_pos(Lista l, int pos){
    assert(pos >= 0 && pos < l->tam);
    No guia = l->sent->prox;
    for(int i = 0; i < pos; i++){
        guia = guia->prox;
    }
    return guia->dado;
}

dado_t l_remove_pos(Lista l, int pos){
    assert(pos >= 0 && pos < l->tam);
    No removido = l->sent->prox;
    for(int i = 0; i < pos; i++) removido = removido->prox;
    removido->ant->prox = removido->prox;
    removido->prox->ant = removido->ant;
    dado_t dado = removido->dado;
    free(removido);
    l->tam--;
    return dado;
}

void l_insere_inicio(Lista l, dado_t dado){
    l_insere_pos(l, dado, 0);
}

void l_insere_fim(Lista l, dado_t dado){
    l_insere_pos(l, dado, l->tam);
}

dado_t l_dado_inicio(Lista l){
    return l_dado_pos(l, 0);
}

dado_t l_dado_fim(Lista l){
    return l_dado_pos(l, l->tam - 1);
}

dado_t l_remove_inicio(Lista l){
    return l_remove_pos(l, 0);
}

dado_t l_remove_fim(Lista l){
    return l_remove_pos(1, l->tam - 1);
}

dado_t l_primeiro(Lista l){
    return l_dado_inicio(l);
}

void l_insere(Lista l, dado_t dado){
    l_insere_fim(l, dado);
}

dado_t l_remove(Lista l){
    return l_remove_inicio(l);
}

dado_t l_topo(Lista l){
    return l_dado_inicio(l);
}

void l_empilha(Lista l, dado_t dado){
    l_insere_inicio(l, dado);
}

dado_t l_desempilha(Lista l){
    return l_remove_inicio(l);
}

Lista l_cria_separando(Str s, Str sep){
    Lista l = l_cria();
    int n = s_tam(s);
    int pos = 0;
    while(pos < n){
        int ini = s_busca_nc(s, pos, sep);
        if(ini == -1) break;
        int fim = s_busca_c(s, ini, sep);
        if(fim == -1) break;
        l_insere_fim(l, s_cria_substring(s, ini, fim - ini));
        pos = fim;
    }
    return l;
}