#ifndef REPLY_H_   /* Include guard */
#define REPLY_H_

struct reply{
   int error; //! 0 -> tudo bem; -1 -> stock insuficiente; -2 -> id artigo inexistente
   int stock;
   float preco;
};

#endif // REPLY_H_