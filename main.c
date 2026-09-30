#include "calc.h"
#include "str.h"
#include "lista.h"

#include <stdio.h>
#include <stdlib.h>

int main(){
    char *entrada_arq = "entrada.txt";
    char *saida_arq = "saida.txt";
    Str counteudo_entrada = s_cria_de_arquivo(entrada_arq);
    if(s_tam(counteudo_entrada) == 0){
        fprintf(stderr, "não conseguiu ler \"%s\"\n", entrada_arq);
        s_destroi(counteudo_entrada);
        return 1;
    }
    Str quebra_linha = s_cria("\n");
    Lista linhas = l_cria_separando(counteudo_entrada, quebra_linha);
    Lista saida = l_cria();
    int n_linhas = l_tam(linhas);
    for(int i = 0; i < n_linhas; i++){
        Str linha = l_dado_pos(linhas, i);
        Str resultado = calculadora(linha);
        l_insere(saida, resultado);
    }
    Str conteudo_saida = s_cria_unindo(saida, quebra_linha);
    s_grava_arquivo(conteudo_saida, saida_arq);
    s_destroi(counteudo_entrada);
    s_destroi(quebra_linha);
    s_destroi(conteudo_saida);
    l_destroi(linhas);
    l_destroi(saida);
    calc_destroi_dicionario();
    return 0;
}
