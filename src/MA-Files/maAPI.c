#include "ma.h"
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
#define MAX 1024
#define PROMPT ">>>"

static const char *ARTIGOS = "./BaseDados/artigos";
static const char *STRINGS = "./BaseDados/strings";
static const char *STOCKS = "./BaseDados/stocks";
static const char *TMP = "./BaseDados/tmp";
static const char *LOGS = "./BaseDados/logs";

/*---------------------------------------------------------------------*/
/*--------------------------Funções Auxiliares-------------------------*/
/*---------------------------------------------------------------------*/

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

/*---------------------------------------------------------------------*/

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

/*---------------------------------------------------------------------*/

// Imprime o manual de manutenção de artigos
void printMan()
{
    write(1, "###############################################################\n", 65);
    write(1, "################# MANUAL MANUTENÇAO DE ARTIGOS ################\n", 66);
    write(1, "###############################################################\n", 65);
    write(1, "# Para inserir um artigo: i <nome> <preço>                    #\n", 66);
    write(1, "# Para mudar o nome de um artigo: n <id artigo> <novo nome>   #\n", 65);
    write(1, "# Para mudar o preço de um artigo: p <id artigo> <novo preço> #\n", 67);
    write(1, "# Para ler um artigo pelo id: r <id artigo>                   #\n", 65);
    write(1, "# Para correr o agregador: ag                                 #\n", 65);
    write(1, "# Para eliminar todos os ficheiros: d                         #\n", 65);
    write(1, "###############################################################\n\n", 65);
}

/*---------------------------------------------------------------------*/

// Função de debug auxiliar que imprime os artigos
void printArtigos(struct artigo *art, char *name)
{
    printf("idArtigo: %d \n", art->idArtigo);
    printf("PosNome: %d \n", art->posNome);
    printf("Name: %s\n", name);
    printf("Preço: %f \n", art->preco);
}

/*---------------------------------------------------------------------*/

// Função de debug para testar o ficheiro artigos
void readArtigoById(int id)
{
    int fp = open(ARTIGOS, O_RDONLY, 0777);
    int fps = open(STRINGS, O_RDONLY, 0777);
    char nome[200];
    int n;
    struct artigo *art = malloc(sizeof(struct artigo));

    lseek(fp, id * sizeof(struct artigo), SEEK_CUR);
    if ( (n = read(fp, art, sizeof(struct artigo))) > 0 ) {
        //* ir buscar ao ficheiro strings o nome na posição x
        lseek(fps, art->posNome, SEEK_CUR);
        readln(fps, nome, 200);
        printArtigos(art, nome);
    }

    free(art);
    close(fp);
    close(fps);
}

/*-----------------------------------------------------------------------*/
/*--------------------------Funções Solicitadas--------------------------*/
/*-----------------------------------------------------------------------*/

//Comprimir o ficheiro strings conforme o tamanho total
void compress() {
    int fp_artigos = open (ARTIGOS, O_RDWR, 0777);
    int fp_strings = open (STRINGS, O_RDONLY, 0777);
    int fp_tmp = open (TMP, O_CREAT | O_WRONLY | O_APPEND, 0777);
    char nome[200];
    int newPos = 0;
    int n;

    struct artigo *art = malloc(sizeof(struct artigo));

    while( (n = read(fp_artigos, art,sizeof(struct artigo))) > 0 ){ //! lê todos os artigos  (fp_artigos -> inicio do proximo artigo)

        //* ir ao ficheiro artigos buscar a posição do nome
        //* ir ao ficheiro strings buscar o nome
        printf("############################\n");
        printf("Artigo: %d\n", art->idArtigo);
        printf("Pos nome: %d\n", art->posNome);
        printf("Preço: %f\n", art->preco);

        lseek(fp_strings, art->posNome, SEEK_SET); //! avança n bytes até à posição do nome no ficheiro strings
        readln(fp_strings, nome, 200); //! lê a linha e escreve em nome
        printf("Nome: %s\n\n", nome);
        
        //* escrever no ficheiro tmp o nome
        //* sacar a posição do nome no ficheiro tmp
        newPos = lseek(fp_tmp, 0, SEEK_END); //! diz a posição do nome no novo ficheiro
        char *snome = strcat(nome, "\n"); //! concatena \n no nome e escreve-o em snome
        write(fp_tmp, snome, strlen(snome)); //! escreve o nome no final do ficheiro temporário (O_APPEND)
        printf("newPos: %d\n", newPos);
        printf("SNOME: %s\n", snome);
        printf("############################\n\n\n");

        //* atualizar a posição do nome no ficheiro artigos
        lseek(fp_artigos, (- sizeof(int) - sizeof(float)) , SEEK_CUR); //! recoloca o fp_artigos no posNome do artigo a alterar
        write(fp_artigos, &newPos, sizeof(int)); //! altera apenas a posição (fp_artigos -> artigo.preco)
        lseek(fp_artigos, sizeof(float), SEEK_CUR); //! recoloca o fp_artigos no proximo
    }
    close(fp_artigos);
    close(fp_strings);
    close(fp_tmp);
    free(art);
    
    //* apagar o ficheiro strings
    remove(STRINGS);
    //* mudar o nome do tmp para strings 
    rename(TMP,STRINGS);
}


/*---------------------------------------------------------------------*/

//Insere um novo artigo
int insere(char *nome, float preco)
{

    int fds = open(STRINGS, O_CREAT | O_RDWR, 0777);
    int fda = open(ARTIGOS, O_CREAT | O_RDWR | O_APPEND, 0777);
    int fdstock = open(STOCKS, O_CREAT | O_WRONLY | O_APPEND, 0777);
    int fd = open(LOGS, O_RDWR, 0777);
    int stringNum;
    int artNum;

    read(fd, &stringNum, sizeof(int));
    read(fd, &artNum, sizeof(int));
    lseek(fd, 0, SEEK_SET);
    stringNum++;
    artNum++;
    write(fd, &stringNum, sizeof(int));
    write(fd, &artNum, sizeof(int));

    //Insere artigo no ficheiro strings a posição

    int posNome = lseek(fds, 0, SEEK_END);
    char *snome = strcat(nome, "\n");
    write(fds, snome, strlen(nome));
    close(fds);

    //Insere artigo no ficheiro artigos
    artNum--;
    int idArtigo = artNum;

    struct artigo *art = malloc(sizeof(struct artigo));
    art->idArtigo = idArtigo;
    art->posNome = posNome;
    art->preco = preco;

    write(fda, art, sizeof(struct artigo));
    close(fda);

    //* Insere o artigo ao stock

    struct stock *stock = malloc(sizeof(struct stock));
    stock->idArtigo = idArtigo;
    stock->quantidade = 100;

    write(fdstock, stock, sizeof(struct stock));
    close(fdstock);

    free(stock);
    free(art);

    return 0;
}

/*---------------------------------------------------------------------*/

//Altera o nome de determinado artigo
int setNome(int idArtigo, char *nome)
{
    //* ir ao ficheiro Strings e adicionar o novo nome (sacar a posição)
    int fds = open(STRINGS, O_RDWR | O_APPEND, 0777);
    int fd = open(LOGS, O_RDWR, 0777);
    int stringNum;
    int artNum;

    read(fd, &stringNum, sizeof(int));
    read(fd, &artNum, sizeof(int));
    lseek(fd, 0, SEEK_SET);
    stringNum++;
    write(fd, &stringNum, sizeof(int));


    int newPos = lseek(fds, 0, SEEK_END);
    char *snome = realloc(nome, strlen(nome) + sizeof(char));
    sprintf(snome, "%s\n", snome);
    write(fds, snome, strlen(snome));
    printf("Nome do artigo com a nova posição: %d\n", newPos);
    close(fds);

    //* ir ao ficheiro Artigos e mudar a posição do nome
    int fda = open(ARTIGOS, O_RDWR, 0777);
    struct artigo *art = malloc(sizeof(struct artigo));

    lseek(fda, idArtigo * sizeof(struct artigo) + sizeof(int), SEEK_SET);
    write(fda, &newPos, sizeof(int));
    close(fda);
    

    free(art);

    double dif = ( (double)stringNum / (double)artNum ) - 1;
    if ( dif > 0.20 ) {
        compress();
        lseek(fd, 0, SEEK_SET);
        write(fd, &artNum, sizeof(int));
    }

    close(fd);
    return 0;
}

/*---------------------------------------------------------------------*/

//Altera o preço de determinado artigo
int setPreco(int idArtigo, float preco)
{
    struct artigo *art = malloc(sizeof(struct artigo));

    int fda = open(ARTIGOS, O_RDWR, 0777);
    art->preco = preco;
    lseek(fda, idArtigo * sizeof(struct artigo) + 2 * (sizeof(int)), SEEK_SET);
    write(fda, &preco, sizeof(float));

    close(fda);
    free(art);

    return 0;
}