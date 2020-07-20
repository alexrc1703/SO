#ifndef SV_H_   /* Include guard */
#define SV_H_

#include "../MA-Files/ma.h"

#include <stdlib.h> //atof
#include <stdio.h> 

struct venda{
   int idArtigo;
   int quantidade;
   float total;
};

//API utilizada

//Função que remove possiveis espaços á direita de uma string
char *trim(char *s);

// imprime n vendas
void printVenda(int n);

// Criar Vendas
void createVenda(int idArtigo, int qnt);

// Atualizar Stock
void updateStock(int idArtigo, int qnt, int client_pid);

// Mostra stock
void getStock(int id, int client_pid);

// Corre o agregador
void runAgregador();

#endif // SV_H_