#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <time.h>
#include "../SV-Files/sv.h"
#include "bst.h"

//! --------------------- Funções auxiliares --------------------------
//Função que remove possiveis espaços á direita de uma string

char *trim(char *s)
{
    while (isspace(*s))
        s++;
    char *back = s + strlen(s);
    while (isspace(*--back))
        ;
    *(back + 1) = '\0';
    return s;
}

//Função que retorna o número de bytes de um ficheiro

ssize_t readln(int fildes, void *buf, size_t nbyte)
{
    ssize_t nbytes = 0;
    int n;
    char c;
    char *buffer = (char *)buf;

    // So le o numero de caracteres corresponde ao tamanho do buffer
    while (nbytes < nbyte && (n = read(fildes, &c, 1)) > 0 && c != '\n')
        buffer[nbytes++] = c;

    (nbytes < nbyte) ? (buffer[nbytes] = '\0') : (buffer[nbytes - 1] = '\0');

    return nbytes;
}

//!--------------------------------------------------------------------

struct node *insereVenda(int idArtigo, int quantidade, float total, struct node *tree)
{
    struct venda *vnd = malloc(sizeof(struct venda));
    vnd->idArtigo = idArtigo;
    vnd->quantidade = quantidade;
    vnd->total = total;


       tree = insert(tree, vnd);   

   

    return tree;
}

//* Auxiliar do inorder
void print(struct venda* venda){
    char idArtigo[64];
    char quantidade[64];
    char total[64];

    sprintf(idArtigo,"###### idArtigo: %d ######\n", venda->idArtigo);
    sprintf(quantidade, "Quantidade: %d\n", venda->quantidade);
    sprintf(total, "Total: %f\n", venda->total);
    write(1, idArtigo, strlen(idArtigo));
    write(1, quantidade, strlen(quantidade));
    write(1, total, strlen(total));
//     write(1, "##########################\n", 28);
}

//* Imprime para o stdout o agregado
void inorder(struct node *root)
{
    if (root != NULL)
    {
        inorder(root->left);
        print(root->venda);
        inorder(root->right);
    }
}

int main()
{
    int i = 0;
    char *buf_stdin = (char *)calloc(1024, sizeof(char *));
    char *token_stdin;
    char **args_stdin = (char **)calloc(1024, sizeof(char **));
    struct node *tree_stdin = NULL;
    struct venda *vnd = malloc(sizeof(struct venda));

    while( readln(0, buf_stdin, 1024) > 0 ){

        trim(buf_stdin);

        token_stdin = strtok(buf_stdin, " ");

        while (token_stdin != NULL)
        {
            args_stdin[i] = strdup(token_stdin);
            token_stdin = strtok(NULL, " ");
            i++;
        }

        args_stdin[i] = NULL;

        

        
        vnd->idArtigo = atoi(args_stdin[0]);
        vnd->quantidade = atoi(args_stdin[1]);
        vnd->total = atof(args_stdin[2]);
    


         

        // if (tree_stdin == NULL)
        // {
            tree_stdin = insert(tree_stdin, vnd);
            //tree_stdin = insereVenda(atoi(args_stdin[0]), atoi(args_stdin[1]), atof(args_stdin[2]), tree_stdin);
        // }
        // else
        // {
        //     insert(tree_stdin, vnd);
        //     //insereVenda(atoi(args_stdin[0]), atoi(args_stdin[1]), atof(args_stdin[2]), tree_stdin);
        // } 
        
        vnd = malloc(sizeof(struct venda));
        i = 0;
        buf_stdin = (char *)calloc(1024, sizeof(char *));
    }

    //write(1,"done\n",6);
    inorder(tree_stdin);
    free(tree_stdin);
    
    return 0;
}