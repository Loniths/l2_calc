// includes, constantes e declarações {{{1
#include "str.h"
#include "utf8.h"
#include "lista.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MIN_ALLOC 8    // alocação mínima

struct str {
  int espaco;
  int espaco_usado;
  int caracteres;
  byte *string;
};

// A memória para conter os bytes de uma string deve ser alocada e/ou
//   realocada conforme a necessidade, cuidando para que a quantidade
//   de memória alocada seja sempre:
//   - nula (não alocada) se a string for vazia, ou
//   - não inferior ao necessário para armazenar os bytes da codificação utf8;
//   - não inferior à alocação mínima;
//   - não superior ao triplo do número de bytes necessários
//     (exceto quando for o mínimo);
//   - uma potência de 2.

// funções auxiliares {{{1

// verifica se a string cad está de acordo com a especificação
// aborta o programa se não tiver
static bool pot_2(int num){
  if(num == 0) return false;
  return ((num & (num - 1)) == 0);
}

static void s_ok(Str_c s)
{
  assert(s != NULL);
  if(s->caracteres == 0){
    assert(s->espaco == 0);
    assert(s->espaco_usado == 0);
    assert(s->string == NULL);
  }
  else{
    assert(s->espaco >= s->espaco_usado);
    assert(s->espaco >= MIN_ALLOC);
    assert(s->espaco <= 3 * s->espaco_usado || s->espaco == MIN_ALLOC);
    assert(pot_2(s->espaco));
    assert(s->string != NULL);
  }
}

//...

static int nbytes(char const *string){
  for(int i = 0;; i++){
    if(string[i] == '\0') return i;
  }
}


static void s_inicia_vazia(Str s){
  s->espaco = 0;
  s->espaco_usado = 0;
  s->caracteres = 0;
  s->string = NULL;
}

static int prox_pot_2(int num){
  int prox = MIN_ALLOC;
  while(prox < num){
    prox *= 2;
  }
  return prox;
}
// operações de criação e destruição {{{1

Str s_cria(char const *strC)
{
  Str s = malloc(sizeof(*s));
  assert(s != NULL);
  if(strC == NULL){
    s_inicia_vazia(s);
    return s;
  }
  int bytes = nbytes(strC);
  if(bytes == 0){
    s_inicia_vazia(s);
    return s;
  }
  int caracteres = u8_conta_unichar_nos_bytes(bytes, (byte *)strC);
  if(caracteres == -1){
    s_inicia_vazia(s);
    return s;
  }
  s->espaco_usado = bytes;
  s->caracteres = caracteres;
  s->espaco = prox_pot_2(bytes);
  s->string = malloc(s->espaco);
  assert(s->string != NULL);
  memcpy(s->string, strC, bytes);
  return s;
}

void s_destroi(Str s)
{
  s_ok(s);
  free(s->string);
  free(s);
}

static int s_nbytes_unichar(byte *s, int tam){
  byte *fim = u8_avanca_unichar(s, tam);
  return fim - s;
}

static void s_aloca(Str s, int tam){
  s->espaco = prox_pot_2(tam);
  s->string = malloc(s->espaco);
  if(s->string == NULL) exit(1);
}

void s_substring(Str s, Str_c sb, int pos, int tam);

Str s_cria_substring(Str_c s, int pos, int tam)
{
   Str nova = s_cria("");
   int posicao = pos >= 0 ? pos : s->caracteres + pos + 1;
   s_substring(nova, s, posicao, tam);
   return nova;
}

Str s_cria_cópia(Str_c s)
{
  return s_cria_substring(s, 0, -1);
}

int s_tam(Str_c s);

// Retorna uma nova string com o conteúdo do arquivo chamado nome.
// Retorna uma string vazia em caso de erro.
Str s_cria_de_arquivo(char *nome)
{
  Str s = s_cria("");
  FILE *arq = fopen(nome, "rb");
  if(arq == NULL) return s;
  fseek(arq, 0, SEEK_END);
  long tamanho =  ftell(arq);
  if(tamanho == 0){
    fclose(arq);
    return s;
  }
  rewind(arq);
  s_aloca(s, tamanho);
  s->espaco_usado = tamanho;
  fread(s->string, 1, tamanho, arq);
  fclose(arq);
  s->caracteres = u8_conta_unichar_nos_bytes(tamanho, s->string);
  if(s->caracteres < 0){
    free(s->string);
    s_inicia_vazia(s);
    return s;
  }
  return s;
}

// operações de acesso {{{1

int s_tam(Str_c s)
{
  s_ok(s);
  return u8_conta_unichar_nos_bytes(s->espaco_usado, s->string);
}

char *s_strc(Str_c s)
{
  s_ok(s);
  char *string = malloc(s->espaco_usado + 1);
  assert(string != NULL);
  memcpy(string, s->string, s->espaco_usado);
  string[s->espaco_usado] = '\0';
  return string;
}

unichar s_ch(Str_c s, int pos)
{
  s_ok(s);
  if(pos >= s->caracteres) return UNI_INV;
  int posicao = pos >= 0 ? pos : s->caracteres + pos + 1;
  byte *caractere = u8_avanca_unichar(s->string, posicao);
  unichar codigo;
  int restantes = s->espaco_usado - (caractere - s->string);
  u8_unichar_nos_bytes(restantes, caractere, &codigo);
  return codigo;
}


// operações de busca e comparação {{{1

bool s_igual(Str_c s, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  if(s->caracteres != sb->caracteres) return false;
  if(s->espaco_usado != sb->espaco_usado) return false;
  if(memcmp(s->string, sb->string, s->espaco_usado) == 0) return true;
  return false;
}

static bool s_char_em(unichar c, Str_c s){
  for(int i = 0; i < s->caracteres; i++){
    if(c == s_ch(s, i )) return true;
  }
  return false;
}

int s_busca_c(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  int posicao = pos >= 0 ? pos : s->caracteres + pos + 1;
  if(posicao < 0) posicao = 0;
  if(posicao > s->caracteres) posicao = s->caracteres;
  for(int i = posicao; i < s->caracteres; i++){
    if(s_char_em(s_ch(s, i), sb)) return i;
  }
  return -1;
}


int s_busca_nc(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  int posicao = pos >= 0 ? pos : s->caracteres + pos + 1;
  if(posicao < 0) posicao = 0;
  if(posicao > s->caracteres) posicao = s->caracteres;
  for(int i = posicao; i < s->caracteres; i++){
    if(!s_char_em(s_ch(s, i), sb)) return i;
  }
  return -1;
}

int s_busca_rc(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  int posicao = pos >= 0 ? pos : s->caracteres + pos + 1;
  if(posicao < 0) posicao = 0;
  if(posicao > s->caracteres) posicao = s->caracteres;
  for(int i = posicao - 1; i >= 0; i--){
    if(s_char_em(s_ch(s, i), sb)) return i;
  }
  return -1;
}

int s_busca_rnc(Str_c s, int pos, Str_c sb)
{
  s_ok(s);
  s_ok(sb);
  int posicao = pos >= 0 ? pos : s->caracteres + pos + 1;
  if(posicao < 0) posicao = 0;
  if(posicao > s->caracteres) posicao = s->caracteres;
  for(int i = posicao - 1; i >= 0; i--){
    if(!s_char_em(s_ch(s, i), sb)) return i;
  }
  return -1;
}

int s_busca_s(Str_c s, int pos, Str_c buscada)
{
  s_ok(s);
  s_ok(buscada);
  int posicao = pos >= 0 ? pos : s->caracteres + pos + 1;
  if(posicao < 0) posicao = 0;
  if(posicao > s->caracteres) posicao = s->caracteres;
  if(buscada->caracteres == 0) return posicao;
  for(int i = posicao; i < s->caracteres; i++){
    Str substring = s_cria_substring(s, i, buscada->caracteres);
    if(s_igual(substring, buscada)){
      s_destroi(substring);
      return i;
    }
    s_destroi(substring);
  }
  return -1;
}


// operações de alteração {{{1

void s_substitui(Str s, int pos, int tam, Str_c sb)
{
  s_ok(s);
  Str vazia = NULL;
  if(sb == NULL){
    vazia = s_cria("");
    sb = vazia;
  }
  s_ok(sb);
  int posicao = pos >= 0 ? pos : s->caracteres + pos + 1;
  int tamanho = tam >= 0 ? (posicao + tam - 1) : (s->caracteres - 1);
  if(posicao < 0) posicao = 0;
  if(posicao > s->caracteres) posicao = s->caracteres;
  if(tamanho > s->caracteres - 1) tamanho = s->caracteres - 1;
  if(tamanho < posicao - 1) tamanho = posicao - 1;
  int tam_removido = tamanho - posicao + 1;
  byte *inicio = u8_avanca_unichar(s->string, posicao);
  byte *fim = (tam_removido > 0) ? u8_avanca_unichar(inicio, tam_removido) : inicio;
  int bytes_antes = inicio - s->string;
  int bytes_depois = s->espaco_usado - (fim - s->string);
  int novos_bytes = bytes_antes + sb->espaco_usado + bytes_depois;
  int novos_caracteres = s->caracteres - tam_removido + sb->caracteres;
  byte *nova = NULL;
  int novo_espaco = 0;
  if(novos_bytes > 0){
    novo_espaco = prox_pot_2(novos_bytes);
    nova = malloc(novo_espaco);
    assert(nova != NULL);
    memcpy(nova, s->string, bytes_antes);
    if(sb->espaco_usado > 0){
      memcpy(nova + bytes_antes, sb->string, sb->espaco_usado);
    }
    if(bytes_depois > 0){
      memcpy(nova + bytes_antes + sb->espaco_usado, s->string + (fim - s->string), bytes_depois);
    }
  }
  free(s->string);
  if(novos_bytes == 0){
    s_inicia_vazia(s);
  }
  else{
    s->string = nova;
    s->espaco = novo_espaco;
    s->espaco_usado = novos_bytes;
    s->caracteres = novos_caracteres;
  }
  if(vazia != NULL) s_destroi(vazia);
}

void s_substring(Str s, Str_c sb, int pos, int tam){
  s_ok(s);
  s_ok(sb);
  if(tam == 0 || pos >= sb->caracteres){
    s_inicia_vazia(s);
    return;
  }
  int posicao = pos >= 0 ? pos : sb->caracteres + pos + 1;
  byte *inicio = u8_avanca_unichar(sb->string, posicao);
  int nbytes;
  if(tam == -1){
    nbytes = sb->espaco_usado - (inicio - sb->string);
    s->caracteres = sb->caracteres - posicao;
  }
  else{
    int caracteres = sb->caracteres - posicao;
    if(tam > caracteres) tam = caracteres;
    nbytes = s_nbytes_unichar(inicio, tam);
    s->caracteres = tam;
  }
  s_aloca(s, nbytes);
  s->espaco_usado = nbytes;
  memcpy(s->string, inicio, nbytes);
}

void s_copia(Str s, Str_c sb)
{
  s_substring(s, sb, 0, -1);
}

void s_insere(Str s, int pos, Str_c sb)
{
  s_substitui(s, pos, 0, sb);
}

void s_insere_c(Str s, int pos, unichar c)
{
  s_ok(s);
  byte utf8[4];
  int nbytes = u8_converte_pra_utf8(c, utf8);
  assert(nbytes > 0);
  Str temp = s_cria("");
  s_aloca(temp, nbytes);
  temp->espaco_usado = nbytes;
  temp->caracteres = 1;
  memcpy(temp->string, utf8, nbytes);
  s_substitui(s, pos, 0, temp);
  s_destroi(temp);
}

void s_anexa(Str s, Str_c sb)
{
  s_substitui(s, -1, 0, sb);
}

void s_anexa_c(Str s, unichar c)
{
  s_insere_c(s, -1, c);
}

void s_remove(Str s, int pos, int tam)
{
  s_substitui(s, pos, tam, NULL);
}


void s_apara(Str s, Str_c sobras)
{
  s_ok(s);
  s_ok(sobras);
  int inicio = 0;
  while(inicio < s->caracteres && s_char_em(s_ch(s, inicio), sobras)) inicio++;
  if(inicio == s->caracteres){
    free(s->string);
    s_inicia_vazia(s);
    return;
  }
  int fim = s->caracteres - 1;
  while(fim >= inicio && s_char_em(s_ch(s, fim), sobras)) fim--;
  int tam = fim - inicio + 1;
  Str temp = s_cria_substring(s, inicio, tam);
  free(s->string);
  s->string = temp->string;
  s->espaco = temp->espaco;
  s->espaco_usado = temp->espaco_usado;
  s->caracteres = temp->caracteres;
  free(temp);
}

// operações de E/S {{{1

void s_imprime(Str_c s)
{
  s_ok(s);
  for(int i = 0; i < s->espaco_usado; i++){
    putchar(s->string[i]);
  }
}

void s_grava_arquivo(Str_c s, char *nome)
{
  s_ok(s);
  FILE *arq = fopen(nome, "wb");
  if(arq == NULL) return;
  if(s->espaco_usado > 0) fwrite(s->string, 1, s->espaco_usado, arq);
  fclose(arq);
}

Str s_cria_número(double num){
  char temp[20];
  snprintf(temp, sizeof(temp), "%g", num);
  return s_cria(temp);
}

double s_número(Str_c s){
  s_ok(s);
  char *c = s_strc(s);
  double num = strtod(c, NULL);
  free(c);
  return num;
}

Str s_cria_unindo(Lista l, Str sep){
  Str s = s_cria("");
  int pos_l = 0;
  while(pos_l < l_tam(l)){
    if(pos_l > 0) s_anexa(s, sep);
    s_anexa(s, l_dado_pos(l, pos_l));
    pos_l++;
  }
  return s;
}

// vim: foldmethod=marker shiftwidth=2