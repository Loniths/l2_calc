#include "calc.h"
#include "lista.h"
#include "str.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef enum{
    Mais_Menos,
    Mul_Div,
    Pot,
    Abre,
    Fecha,
    Igual,
} Categoria;

typedef enum{
    Termina,
    Empilha,
    Opera,
    Descarta,
    Falta_Abrir,
    Falta_Fechar,
} Acao;

static Categoria categoria(Str s){
    unichar c = s_ch(s, 0);
    assert(eh_operador(s));
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

static Acao decide(Categoria topo, Categoria entrada){
    switch(topo){
        case Mais_Menos:
            switch(entrada){
                case Mais_Menos:
                    return Opera;

                case Mul_Div:
                    return Empilha;
                    
                case Pot:
                    return Empilha;

                case Abre:
                    return Empilha;

                case Fecha:
                    return Opera;

                case Igual:
                    return Empilha;
            }

        case Mul_Div:
            switch(entrada){
                case Mais_Menos:
                    return Opera;

                case Mul_Div:
                    return Opera;

                case Pot:
                    return Empilha;

                case Abre:
                    return Empilha;

                case Fecha:
                    return Opera;

                case Igual:
                    return Empilha;
            }

        case Pot:
            switch(entrada){
                case Mais_Menos:
                    return Opera;

                case Mul_Div:
                    return Opera;

                case Pot:
                    return Opera;

                case Abre:
                    return Empilha;

                case Fecha:
                    return Opera;

                case Igual:
                    return Empilha;
            }

        case Abre:
            switch(entrada){
                case Mais_Menos:
                    return Empilha;

                case Mul_Div:
                    return Empilha;

                case Pot:
                    return Empilha;

                case Abre:
                    return Empilha;

                case Fecha:
                    return Descarta;

                case Igual:
                    return Empilha;
            }

        case Igual:
            switch(entrada){
                case Mais_Menos:
                    return Empilha;

                case Mul_Div:
                    return Empilha;

                case Pot:
                    return Empilha;

                case Abre:
                    return Empilha;

                case Fecha:
                    return Opera;
            }
    }
}

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
        l_insere_fim(token, s_cria_substring(txt, ini, i - ini));
    }
    return token;
}

static bool eh_operador(Str s){
    unichar c = s_ch(s, 0);
    if(c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')' || c == '=') return true;
    return false;
}

static bool eh_operando(Str s){
    unichar c = s_ch(s, 0);
    bool digito_ponto = (c >= '0' && c <= '9') || c == '.';
    bool letra_cifrao = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '$';
    return digito_ponto || letra_cifrao;
}


static bool eh_parentesis_aberto(Str s){
    assert(s_tam(s) > 0);
    unichar c = s_ch(s, 0);
    if(c == '(') return true;
    return false;
}

static bool eh_parentesis_fechado(Str s){
    assert(s_tam(s) > 0);
    unichar c = s_ch(s, 0);
    if(c == ')') return true;
    return false;
}

Str calculadora(Str expressao){
    Lista operadores = l_cria();
    bool par_aberto = false;
    Lista operandos = l_cria();
    Lista tokens = tokeniza(expressao);
    while(!l_vazia(tokens)){
        Str s = l_remove(tokens);
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
