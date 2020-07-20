#include "../SV-Files/sv.h"
#ifndef BST_H_ /* Include guard */
#define BST_H_

struct node {
    struct venda *venda;
    struct node *left, *right;
};

struct node* insert(struct node* node, struct venda* venda);

struct node* search(struct node* node, int idArtigo);

#endif // BST_H_