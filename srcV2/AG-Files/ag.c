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

//* Imprime para o stdout o agregado
void inorder(struct node *root)
{
    if (root != NULL)
    {
        inorder(root->left);
        write(1, root->venda, sizeof(struct venda));
        inorder(root->right);
    }
}

int main()
{
    struct node *tree_stdin = NULL;
    struct venda *vnd = malloc(sizeof(struct venda));

    while( read(0, vnd, sizeof(struct venda)) > 0 ){

        tree_stdin = insert(tree_stdin, vnd);
        vnd = malloc(sizeof(struct venda));

    }

    inorder(tree_stdin);
    free(tree_stdin);
    free(vnd);
    
    return 0;
}