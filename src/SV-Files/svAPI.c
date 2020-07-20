#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include "../MA-Files/ma.h"
#include "sv.h"
// #include "bst.h"
#define MAX 1024
#define READ_END    0 
#define WRITE_END   1

static const char *ARTIGOS = "./BaseDados/artigos";
static const char *VENDAS = "./BaseDados/vendas";
static const char *STOCKS = "./BaseDados/stocks";


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

// imprime n vendas
void printVenda(int n) {
    int fp_venda = open(VENDAS, O_RDONLY, 0777);
    struct venda *vnd = malloc(sizeof(struct venda));
    lseek(fp_venda, sizeof(int), SEEK_SET);
    int r = 0;
    int i = 1;

    while( n > 0 ) {
        r = read(fp_venda, vnd, sizeof(struct venda));
        if (r == 0)
            break;

        printf("######## Venda %d #########\n", i);
        printf("idArtigo: %d\n", vnd->idArtigo);
        printf("Quantidade: %d\n", vnd->quantidade);
        printf("Total: %f\n", vnd->total);
        printf("##########################\n");

        n--;
        i++;
    }
}

/*-----------------------------------------------------------------------*/
/*--------------------------Funções Solicitadas--------------------------*/
/*-----------------------------------------------------------------------*/

//* Criar Vendas
void createVenda(int idArtigo, int qnt)
{
    float preco = 0;
    int fp = open(ARTIGOS, O_RDONLY, 0777);
    struct artigo *art = malloc(sizeof(struct artigo));

    lseek(fp, idArtigo * sizeof(struct artigo), SEEK_CUR);
    read(fp, art, sizeof(struct artigo));
    preco = art->preco;
    free(art);
    close(fp);

    //* Cria a venda
    //* Cria o ficheiro vendas se não existir, e escreve na primeira posição do tamanho int a pos da ultima agragação aka sizeof(int)
    int size = sizeof(int);
    int fpvenda = open(VENDAS, O_WRONLY | O_APPEND, 0777);
    if (fpvenda == -1) {
        fpvenda = open(VENDAS, O_CREAT | O_WRONLY | O_APPEND, 0777);
        write(fpvenda, &size, sizeof(int));
    }
    else {
        lseek(fpvenda, sizeof(int), SEEK_SET);
    }

    struct venda *vnd = malloc(sizeof(struct venda));
    vnd->idArtigo = idArtigo;
    vnd->quantidade = qnt;
    vnd->total = (-qnt) * preco;

    write(fpvenda, vnd, sizeof(struct venda));

    free(vnd);
    close(fpvenda);
}

/*---------------------------------------------------------------------*/

// * Atualizar Stock
void updateStock(int idArtigo, int qnt, int client_pid)
{
    int st = 0;
    int fpstock = open(STOCKS, O_RDWR, 0777);
    char response[1024];
    char fifo[1024];

    lseek(fpstock, idArtigo * sizeof(struct stock) + sizeof(int), SEEK_CUR);

    if(read(fpstock, &st, sizeof(int)) > 0)
    {
        st += qnt;
        if (st >= 0) {
            lseek(fpstock, -sizeof(int), SEEK_CUR);
            write(fpstock, &st, sizeof(int));

            if (qnt < 0)
                createVenda(idArtigo, qnt); //! Fork para libertar o pai

            sprintf(response, "Stock atual: %d\n", st);
        }
        else {
            sprintf(response, "Stock insuficiente!\n");
        }
    }

    else {
        sprintf(response,"Artigo inválido!\n");
    }

    sprintf(fifo, "%d", client_pid);
    int fp_fifo = open(fifo, O_WRONLY);
    //write(1, response, strlen(response));
    write(fp_fifo, response, strlen(response));

    close(fpstock);
}

/*---------------------------------------------------------------------*/

// * Mostra stock
void getStock(int id, int client_pid)
{
    int fpstock = open(STOCKS, O_RDONLY, 0777);
    int fpArtigo = open(ARTIGOS, O_RDONLY, 0777);
    float price = 0;
    int quantidade = 0;
    char response[1024];
    char fifo[1024];


    lseek(fpArtigo, (id * sizeof(struct artigo)) + 2 * sizeof(int), SEEK_CUR);

    if(read(fpArtigo, &price, sizeof(float)) > 0){
        lseek(fpstock, (id * sizeof(struct stock)) + sizeof(int), SEEK_CUR);
        read(fpstock, &quantidade, sizeof(int));
        sprintf(response, "Stock: %d Preço: %f\n", quantidade, price);
    }
    else {
        sprintf(response,"id inválido!\n");
    }

    sprintf(fifo, "%d", client_pid);
    int fp_fifo = open(fifo, O_WRONLY);
    //write(1, response, strlen(response));
    write(fp_fifo, response, strlen(response));


    close(fpstock);
    close(fpArtigo);
}

/*---------------------------------------------------------------------*/


// Corre o agregador
void runAgregador( int lastVenda ) {
    
    int fd_venda = open(VENDAS, O_RDWR, 0777);
    printf("Last: %d\n", lastVenda);
    int lastAg; //! ultima agregação
    lseek(fd_venda, 0, SEEK_SET); //! coloca o fd no inicio do ficheiro
    read(fd_venda, &lastAg, sizeof(int)); //! guarda a posição da ultima agregação
    printf("LastAg: %d\n", lastAg);
    lseek(fd_venda, 0, SEEK_SET);             //! coloca o fd no inicio do ficheiro
    write(fd_venda, &lastVenda, sizeof(int)); //! guarda a posição do fim da nova agregação

    if (lastVenda != -1 && lastVenda != lastAg) {

        // get timestamp
        time_t rawtime;
        struct tm *timeinfo;
        time( &rawtime );
        timeinfo = localtime( &rawtime );

        char filename[1024];
        sprintf(filename, "./Agregacao/%d-%d-%dT%d:%d:%d", timeinfo->tm_mday, timeinfo->tm_mon+1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

        //cria o ficheiro para guardar a agregação
        int fd_ag = open(filename, O_CREAT | O_WRONLY | O_APPEND, 0777);

        char *buf = (char *)calloc(MAX, sizeof(char *));
        char *buf_out = (char *)calloc(MAX, sizeof(char *));
        struct venda *vnd = malloc(sizeof(struct venda));
        int status;
        int fd[2];
        int fd_out[2];
        lseek(fd_venda, lastAg, SEEK_SET);

        pid_t pid;

        pipe(fd);
        pipe(fd_out);
        pid = fork();

        /* Parent process */
        if (pid > 0)
        {   
            close(fd[READ_END]);                                                        //! closes o fd de leitura do pipe de escrita
            close(fd_out[WRITE_END]);                                                   //! closes o fd de escrita do pipe de leitura

            while (lastAg < lastVenda)                                                  //! todas as vendas no intervalo de tempo pedido
            {
                read(fd_venda, vnd, sizeof(struct venda));                              //! lê do ficheiro vendas
                lastAg = lseek(fd_venda, 0, SEEK_CUR);                                  //! atualiza a posição 
                sprintf(buf, "%d %d %f\n", vnd->idArtigo, vnd->quantidade, vnd->total); //! guarda no buf a venda
                write(fd[WRITE_END], buf, strlen(buf));                                 //! escreve no fd de escrita do pipe de escrita o buf
                buf = (char *)calloc(MAX, sizeof(char *));                              //! limpa o buf
            }

            /* Closes the write end of the pipe */
            close(fd[WRITE_END]);                                                       //! closes o fd de escrita do pipe de escrita
            // waitpid(pid, &status, WUNTRACED); 


            while(readln(fd_out[READ_END], buf_out, 1024)>0)                               //! lê do fd de leitura do pipe de leitura
            {                                                                           
                write(fd_ag, buf_out, strlen(buf_out));
                write(fd_ag,"\n",strlen("\n"));
                buf_out = (char *)calloc(MAX, sizeof(char *));                                     //! escreve para o ficheiro de agregação o que leu do pipe de leitura
            }
            close(fd_ag);
            close(fd_out[READ_END]);                                                    //! closes o fd de leitura do pipe de leitura

            waitpid(pid, &status, WUNTRACED);
            
        }
        /* Child process */
        else
        {
            /* Close the write end of the pipe */
            close(fd[WRITE_END]);                                                        //! closes o fd de escrita do pipe de escrita
            close(fd_out[READ_END]);                                                     //! closes o fd de leitura do pipe de leitura
            /* Reads from the read end of the pipe */
            dup2(fd[READ_END], 0);                                                       //! copia para o fd de leitura do pipe de escrita o stdin
            dup2(fd_out[WRITE_END], 1);                                                  //! copia para o fd de escrita do pipe de leitura o stdout
            /* Execute ag */
            execlp("./ag", "./ag", NULL);                                                //! executa o agregador
        }
    }
}
