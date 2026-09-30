#include "calc.h"
#include "lista.h"
#include "str.h"
#include "dicionario.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include <string.h>


// funcoes necessarias para a tokeniza:

static bool eh_espaco(unichar c){
    if(c == ' ' || c == '\t' || c == '\n') return true;
    return false;
}

static bool eh_digito(unichar c){
    if((c >= '0' && c <= '9') || c == '.') return true;
    return false;
}

static bool eh_caractere(unichar c){
    if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '$' || c == '_') return true;
    return false;
}

static bool eh_continuacao(unichar c){
    if(eh_caractere(c) || (c >= '0' && c <= '9')) return true;
    return false;
}

Lista tokeniza(Str txt){
    Lista token = l_cria();
    int tam = s_tam(txt);
    int i = 0;
    while(i < tam){
        unichar c = s_ch(txt, i);
        if(eh_espaco(c)){
            i++;
            continue;
        }
        int ini = i;
        if(eh_digito(c)){
            while(i < tam && eh_digito(s_ch(txt, i))) i++;
        }
        else if(eh_caractere(c)){
            while(i < tam && eh_continuacao(s_ch(txt, i))) i++;
        }
        else i++;
        l_insere(token, s_cria_substring(txt, ini, i - ini));
    }
    return token;
}


// funcoes necessarias para a calculadora:


// apartir daq vc vai ver mtas recebendo a Str, e dps convertendo pra unichar
// fiz isso pra ficar mais comodo quando eu chamar elas na calculadora

static bool eh_operador(Str s){
    unichar c = s_ch(s, 0);
    if(c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '(' || c == ')' || c == '=') return true;
    return false;
}

static bool eh_operando(Str s){
    unichar c = s_ch(s, 0);
    bool digito = (c >= '0' && c <= '9') || c == '.';
    bool letra = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '$';
    return digito || letra;
}

static bool eh_parentesis_aberto(Str s){
    unichar c = s_ch(s, 0);
    if(c == '(') return true;
    return false;
}

static bool eh_parentesis_fechado(Str s){
    unichar c = s_ch(s, 0);
    if(c == ')') return true;
    return false;
}

static bool eh_variavel(Str s){
    unichar c = s_ch(s, 0);
    if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '$') return true;
    return false;
}


// tabela:

typedef enum{
    Mais_Menos,
    Mul_Div,
    Pot,
    Abre,
    Fecha,
    Igual,
} Categoria;

typedef enum{
    Empilha,
    Opera,
    Descarta,
} Acao;

static Categoria categoria(Str s){
    assert(eh_operador(s));
    unichar c = s_ch(s, 0);
    switch(c){
        case '+':
        case '-':
            return Mais_Menos;

        case '*':
        case '/':
            return Mul_Div;

        case '^':
            return Pot;

        case '(':
            return Abre;

        case ')':
            return Fecha;

        case '=':
            return Igual;
    }
}

static Acao decide(Categoria topo, Categoria novo){
    switch(topo){
        case Mais_Menos:
            switch(novo){
                case Mais_Menos: return Opera;
                case Mul_Div: return Empilha;
                case Pot: return Empilha;
                case Abre: return Empilha;
                case Fecha: return Opera;
                case Igual: return Empilha;
            }

        case Mul_Div:
            switch(novo){
                case Mais_Menos: return Opera;
                case Mul_Div: return Opera;
                case Pot: return Empilha;
                case Abre: return Empilha;
                case Fecha: return Opera;
                case Igual: return Empilha;
            }

        case Pot:
            switch(novo){
                case Mais_Menos: return Opera;
                case Mul_Div: return Opera;
                case Pot: return Opera;
                case Abre: return Empilha;
                case Fecha: return Opera;
                case Igual: return Empilha;
            }

        case Abre:
            switch(novo){
                case Mais_Menos: return Empilha;
                case Mul_Div: return Empilha;
                case Pot: return Empilha;
                case Abre: return Empilha;
                case Fecha: return Descarta;
                case Igual: return Empilha;
            }

        case Igual:
            switch(novo){
                case Mais_Menos: return Empilha;
                case Mul_Div: return Empilha;
                case Pot: return Empilha;
                case Abre: return Empilha;
                case Fecha: return Opera;
                case Igual: return Empilha;
            }
    }
}

// func do dicionario

static Dicionário variaveis = NULL;

static bool chave_igual(chave_t a, chave_t b){
    if(strcmp((char *)a, (char *)b) == 0) return true;
    return false;
}

static bool chave_menor(chave_t a, chave_t b){
    if(strcmp((char *)a, (char *)b) < 0) return true;
    return false;
}

static Dicionário cria_variaveis(){
    if(variaveis == NULL) variaveis = dic_cria(chave_menor, chave_igual);
    return variaveis;
}

static void atualiza_variavel(Dicionário dic, Str chave, Str valor){
    char *chave_nova = s_strc(chave);
    char *valor_novo = s_strc(valor);
    valor_t anterior = dic_busca(dic, chave_nova);
    dic_insere(dic, chave_nova, valor_novo);
    if(anterior != VALOR_NÃO_EXISTE){
        free(chave_nova);
        free(anterior);
    }
}

static char *busca_variavel(Dicionário dic, Str busca){
    char *chave = s_strc(busca);
    valor_t v = dic_busca(dic, chave);
    free(chave);
    return (char *)v;
}

static bool obtem_valor(Str operando, Dicionário dic, double *valor, Str *erro){
    unichar c = s_ch(operando, 0);
    if(eh_digito(c)){
        *valor = s_número(operando);
        return true;
    }
    char *v = busca_variavel(dic, operando);
    if(v == NULL){
        *erro = s_cria("#ERRO variável indefinída");
        return false;
    }
    Str valor_str = s_cria(v);
    *valor = s_número(valor_str);
    s_destroi(valor_str);
    return true;
}

// operacoes

static bool opera_igual(Lista operandos, Dicionário dic, Str *erro){
    if(l_tam(operandos) < 2){
        *erro = s_cria("#ERRO faltam operandos para =");
        return false;
    }
    Str valor_token = l_desempilha(operandos);
    Str chave_token = l_desempilha(operandos);
    if(!eh_variavel(chave_token)){
        *erro = s_cria("#ERRO lado esquerdo do = não contém variável");
        s_destroi(valor_token);
        s_destroi(chave_token);
        return false;
    }
    double valor_num;
    if(!obtem_valor(valor_token, dic, &valor_num, erro)){
        s_destroi(valor_token);
        s_destroi(chave_token);
        return false;
    }
    Str valor_str = s_cria_número(valor_num);
    atualiza_variavel(dic, chave_token, valor_str);
    l_empilha(operandos, valor_str);
    s_destroi(valor_token);
    s_destroi(chave_token);
    return true;
}

static bool opera(Str operador, Lista operandos, Dicionário dic, Str *erro){
    unichar c = s_ch(operador, 0);
    if(c == '='){
        return opera_igual(operandos, dic, erro);
    }
    if(l_tam(operandos) < 2){
        *erro = s_cria("#ERRO faltam operandos");
        return false;
    }
    Str b_token = l_desempilha(operandos);
    Str a_token = l_desempilha(operandos);
    double a, b;
    bool ok = obtem_valor(a_token, dic, &a, erro);
    if(ok) ok = obtem_valor(b_token, dic, &b, erro);
    double r = 0;
    if(ok){
        switch(c){
            case '+': 
                r = a + b;
                break;

            case '-': 
                r = a - b;
                break;

            case '*':
                r = a * b;
                break;

            case '/':
                if(b == 0){
                    *erro = s_cria("#ERRO imposível dividir por zero");
                    ok = false;
                }
                else r = a / b;
                break;
            
            case '^':
                r = pow(a, b);
                break;

            default:
                *erro = s_cria("#ERRO operação invalida");
                ok = false;
                break;
        }
    }
    s_destroi(a_token);
    s_destroi(b_token);
    if(!ok) return false;
    l_empilha(operandos, s_cria_número(r));
    return true;
}

Str calculadora(Str expressao){
    Dicionário variaveis = cria_variaveis();
    Lista tokens = tokeniza(expressao);
    Lista operadores = l_cria();
    Lista operandos = l_cria();
    Str erro = NULL;
    Str token_atual = NULL;
    bool tem_token = false;
    bool fim_entrada = false;
    while(erro == NULL){
        if(!tem_token){
            if(l_vazia(tokens)){
                fim_entrada = true;
                token_atual = NULL;
            }
            else{
                token_atual = l_remove(tokens);
                fim_entrada = false;
            }
            tem_token = true;
        }
        if(!fim_entrada && eh_operando(token_atual)){
            l_empilha(operandos, token_atual);
            tem_token = false;
            continue;
        }
        if(!fim_entrada && !eh_operador(token_atual)){
            erro = s_cria("#ERRO token inválido");
            s_destroi(token_atual);
            break;
        }
        bool pilha_vazia = l_vazia(operadores);
        if(pilha_vazia && fim_entrada){
            break;
        }
        if(pilha_vazia){
            if(eh_parentesis_fechado(token_atual)){
                erro = s_cria("#ERRO falta (");
                s_destroi(token_atual);
                break;
            }
            l_empilha(operadores, token_atual);
            tem_token = false;
            continue;
        }
        if(fim_entrada){
            Str topo = l_topo(operadores);
            if(eh_parentesis_aberto(topo)){
                erro = s_cria("#ERRO falta )");
                break;
            }
            Str op = l_desempilha(operadores);
            bool ok = opera(op, operandos, variaveis, &erro);
            s_destroi(op);
            if(!ok) break;
            continue;
        }
        Categoria topo = categoria(l_topo(operadores));
        Categoria entrada = categoria(token_atual);
        Acao acao = decide(topo, entrada);
        if(acao == Empilha){
            l_empilha(operadores, token_atual);
            tem_token = false;
        }
        else if(acao == Descarta){
            Str descartar = l_desempilha(operadores);
            s_destroi(descartar);
            s_destroi(token_atual);
            tem_token = false;
        }
        else{
            Str op = l_desempilha(operadores);
            bool ok = opera(op, operandos, variaveis, &erro);
            s_destroi(op);
            if(!ok){
                s_destroi(token_atual);
                break;
            }
        }
    }
    Str resultado;
    if(erro != NULL){
        resultado = erro;
    }
    else if(l_tam(operandos) != 1){
        resultado = s_cria("#ERRO inválido");
    }
    else{
        Str sobrou = l_desempilha(operandos);
        double valor;
        Str erro_final = NULL;
        if(obtem_valor(sobrou, variaveis, &valor, &erro_final)){
            resultado = s_cria_número(valor);
        }
        else{
            resultado = erro_final;
        }
        s_destroi(sobrou);
    }
    l_destroi(tokens);
    l_destroi(operadores);
    l_destroi(operandos);
    return resultado;
}

static void libera_tupla(chave_t chave, valor_t valor){
    free(chave);
    free(valor);
}

void calc_destroi_dicionario(){
    if(variaveis != NULL){
        dic_para_todos(variaveis, libera_tupla);
        dic_destroi(variaveis);
        variaveis = NULL;
    }
}
