#include "calc.h"
#include "lista.h"
#include "str.h"
#include "dicionario.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>


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
    if(c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')' || c == '=') return true;
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
