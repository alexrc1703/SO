#include "../SV-Files/sv.h"
#include "bst.h"
#include <stdlib.h>
#include <strings.h>

struct node* search(struct node* root, int idArtigo) {
    if (root == NULL || root->venda->idArtigo == idArtigo){
        return root;
    }

    if (root->venda->idArtigo < idArtigo) return search(root->right, idArtigo);

    return search(root->left, idArtigo);
}

struct node *newNode(struct venda *vnd) {
    
    struct node *temp = (struct node *)malloc(sizeof(struct node));
    temp->venda = vnd;
    temp->left = temp->right = NULL;

    return temp;
}

struct node *insert(struct node *node, struct venda *vnd) {
    
    if (node == NULL) return newNode(vnd);

    else if (node->venda->idArtigo == vnd->idArtigo) {
        node->venda->quantidade += vnd->quantidade;
        node->venda->total += vnd->total;

        return node;
    }

    if (vnd->idArtigo < node->venda->idArtigo) node->left = insert(node->left, vnd);

    else if(vnd->idArtigo > node->venda->idArtigo) node->right = insert(node->right, vnd);

    return node;
}