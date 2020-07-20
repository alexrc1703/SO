#ifndef MA_H_   /* Include guard */
#define MA_H_

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h> 
#include <stdio.h>


struct artigo{
   int idArtigo;
   int posNome;
   float preco;
};

struct stock
{
   int idArtigo;
   int quantidade;
};


//API utilizada

//Função que remove possiveis espaços á direita de uma string
char *trim(char *s);

//Função que retorna o número de bytes de um ficheiro
ssize_t readln(int fildes, void *buf, size_t nbyte);

// Imprime o manual de manutenção de artigos
void printMan();

// Função de debug auxiliar que imprime os artigos
void printArtigos(struct artigo *art, char *name);

// Função de debug para testar o ficheiro artigos
void readArtigoById(int id);

//Comprimir o ficheiro strings conforme o tamanho total
void compress();

//Insere um novo artigo
int insere(char *nome, float preco); // retorna o id do novo artigo

//Altera o nome de determinado artigo
int setNome(int idArtigo, char *nome); // retorna 0 sucesso, 1 algo correu mal

//Altera o preço de determinado artigo
int setPreco(int idArtigo, float preco); // retorna 0 sucesso, 1 algo correu mal

#endif // MA_H_