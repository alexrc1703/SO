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
static const char *LOGS = "./BaseDados/logsSV";


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

    int fpvenda = open(VENDAS, O_WRONLY | O_APPEND, 0777);

    struct venda *vnd = malloc(sizeof(struct venda));
    vnd->idArtigo = idArtigo;
    vnd->quantidade = (-qnt);
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
    char fifo[20];
    char response[64];

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

    sprintf(fifo, "./Fifos/%dR", client_pid);
    int fp_fifo = open(fifo, O_WRONLY);
    //write(1, response, strlen(response));
    write(fp_fifo, response, strlen(response));

    close(fpstock);
}

/*---------------------------------------------------------------------*/

// * Mostra stock
void getStock(int id, int fdWrite)
{
    int fpstock = open(STOCKS, O_RDONLY, 0777);
    int fpArtigo = open(ARTIGOS, O_RDONLY, 0777);
    float price = 0;
    int quantidade = 0;
    char response[1024];


    lseek(fpArtigo, (id * sizeof(struct artigo)) + 2 * sizeof(int), SEEK_CUR);

    if(read(fpArtigo, &price, sizeof(float)) > 0){
        lseek(fpstock, (id * sizeof(struct stock)) + sizeof(int), SEEK_CUR);
        read(fpstock, &quantidade, sizeof(int));
        sprintf(response, "Stock: %d Preço: %f\n", quantidade, price);
    }
    else {
        sprintf(response,"id inválido!\n");
    }

    //write(1, response, strlen(response));
    write(fdWrite, response, strlen(response));


    close(fpstock);
    close(fpArtigo);
}

/*---------------------------------------------------------------------*/


// Corre o agregador
void runAgregador( int lastVenda , int lastAg) {
    
    int fd_venda = open(VENDAS, O_RDONLY, 0777);
    int fd_logs = open(LOGS, O_RDWR, 0777);

    // get timestamp
    time_t rawtime;
    struct tm *timeinfo;
    time( &rawtime );
    timeinfo = localtime( &rawtime );

    char filename[1024];
    sprintf(filename, "./Agregacao/%d-%d-%dT%d:%d:%d", timeinfo->tm_mday, timeinfo->tm_mon+1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    //cria o ficheiro para guardar a agregação
    int fd_ag = open(filename, O_CREAT | O_WRONLY | O_APPEND, 0777);

    char *buf_out = (char *)malloc(MAX);
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
            lastAg += sizeof(struct venda);                                         //! atualiza a posição 
            //sprintf(buf_out, ">>idArtigo: %d \n Quantidade: %d \n Total: %f LastAg: %d\n\n", vnd->idArtigo, vnd->quantidade, vnd->total, lastAg); //? debug
            //write(1, buf_out, strlen(buf_out));   //? debug
            write(fd[WRITE_END], vnd, sizeof(struct venda));                        //! escreve no fd de escrita do pipe de escrita o buf
            //memset(vnd, -1, sizeof(struct venda));
        }
        /* Closes the write end of the pipe */
        close(fd[WRITE_END]);                                                       //! closes o fd de escrita do pipe de escrita


        while( read(fd_out[READ_END], vnd, sizeof(struct venda)) > 0 )                //! lê do fd de leitura do pipe de leitura
        {   
            sprintf(buf_out, "<<idArtigo: %d \n Quantidade: %d \n Total: %f \n\n", vnd->idArtigo, vnd->quantidade, vnd->total);
            //write(1, buf_out, strlen(buf_out));   //? debug                                                    
            write(fd_ag, buf_out, strlen(buf_out));                                   //! escreve para o ficheiro de agregação o que leu do pipe de leitura
            free(buf_out);
            buf_out = (char *)malloc(MAX);                         
        }
        
        // printf("This ag: %d\n", lastAg);
        write(fd_logs, &lastAg, sizeof(int));
        close(fd_logs);
        close(fd_venda);
        close(fd_ag);
        close(fd_out[READ_END]);                                                    //! closes o fd de leitura do pipe de leitura
        waitpid(pid, &status, WUNTRACED);
        printf("Fim agregação\n"); //? debug
        
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
