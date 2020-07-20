#ifndef LR_H_   /* Include guard */
#define LR_H_

struct lRequest{
   int type; //! 1->client 2->ma 3->ag
   int pid;
   int idArtigo;
   int quantidade;
   float preco;
};

#endif // LR_H_