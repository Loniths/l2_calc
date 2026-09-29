#include "calc.h"
#include "lista.h"
#include "str.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool eh_espaco(unichar c){
    if(c == ' ' || c == '\t' || c == '\n') return true;
    return false;
}

bool eh_digito(unichar c){
    if((c >= '0' && c <= '9') || c == '.') return true;
    return false;
}

bool eh_caractere(unichar c){
    if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '$' || c == '_') return true;
    return false;
}

bool eh_continuacao(unichar c){
    if(eh_caractere || (c >= '0' && c <= '9')) return true;
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
        l_insere_fim(token, s_cria_substring(txt, ini, i - ini));
    }
    return token;
}

static bool eh_operador(Str s){
    unichar c = s_ch(s, 0)
    if(c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')' || c == '=') return true;
    return false;
}

static bool eh_operando(Str s){
    int tam = s_tam(s);
    unichar c = s_ch(s, 0);
    if(tam > 1){
        if((c >= '0' && c <= '9') && s_ch(s, 1) == '.') return true;
    }
    else{
        if(c >= '0' && c <= '9') return true;
    }
    return false;
}

static int precedencia(Str s){
    assert(s_tam(s) > 0);
    unichar c = s_ch(s, 0);
    switch(c){
        case '+':
        case '-':
            return 1;
        
        case '*':
        case '/': return 2;

        default: return 0;
    }
}

static bool eh_parentesis_aberto(Str s){
    assert(s_tam(s) > 0);
    unichar c = s_ch(s, 0);
    if(c == '(') return true;
    return false;
}

static bool eh_parentesis_fechado(Str s){
    assert(s_tam(s) > 0);
    unichar c - s_ch(s, 0);
    if(c == ')') return true;
    return false;
}

Str calculadora(Str expressao){
    Lista operadores = l_cria();
    bool par_aberto = false;
    Lista operandos = l_cria();
    Lista tokens = tokeniza(expressao);
    while(!l_vazia(tokens)){
        Str s = l_desempilha(tokens);
        if(eh_operador(s)){
            if(eh_parentesis_aberto(s)){
                par_aberto = true;
                l_empilha(operadores, s);
            } 
            else if(eh_parentesis_fechado(s)){
                if(!par_aberto)
            }
        }
        else if(eh_operando){
            l_empilha(operandos, s);
        }
    }
}