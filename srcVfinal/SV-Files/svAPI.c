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
#include "../reply.h"
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
    struct reply *r = malloc(sizeof(struct reply));

    lseek(fpstock, idArtigo * sizeof(struct stock) + sizeof(int), SEEK_CUR);

    if(read(fpstock, &st, sizeof(int)) > 0)
    {
        st += qnt;
        if (st >= 0) {
            lseek(fpstock, -sizeof(int), SEEK_CUR);
            write(fpstock, &st, sizeof(int));

            if (qnt < 0)
                createVenda(idArtigo, qnt);
            r->error = 0;
            r->stock = st;
        }
        else {
            r->error = -1;
        }
    }

    else {
        r->error = -2;
    }
    
    sprintf(fifo, "./Fifos/%dR", client_pid);
    int fp_fifo = open(fifo, O_WRONLY);
    //write(1, response, strlen(response));
    write(fp_fifo, r, sizeof(struct reply));
    close(fp_fifo);
    close(fpstock);
    free(r);
}

/*---------------------------------------------------------------------*/

// * Mostra stock
void getStock(int id, int fdWrite)
{
    int fpstock = open(STOCKS, O_RDONLY, 0777);
    int fpArtigo = open(ARTIGOS, O_RDONLY, 0777);
    float price = 0;
    int quantidade = 0;
    struct reply *r = malloc(sizeof(struct reply));


    lseek(fpArtigo, (id * sizeof(struct artigo)) + 2 * sizeof(int), SEEK_CUR);

    if(read(fpArtigo, &price, sizeof(float)) > 0){
        lseek(fpstock, (id * sizeof(struct stock)) + sizeof(int), SEEK_CUR);
        read(fpstock, &quantidade, sizeof(int));
        r->error = 0;
        r->stock = quantidade;
        r->preco = price;
    }
    else {
        r->error = -2;
    }

    //write(1, response, strlen(response));
    write(fdWrite, r, sizeof(struct reply));


    close(fpstock);
    close(fpArtigo);
    free(r);
}

/*---------------------------------------------------------------------*/
void singleAg(int lastAg, int lastVenda){
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
        write(fd_logs, &lastAg, sizeof(int));
        close(fd_out[READ_END]);                                                      //! closes o fd de leitura do pipe de leitura
        // printf("This ag: %d\n", lastAg);
        close(fd_logs);
        close(fd_venda);
        close(fd_ag);                                                   
        wait(&status);
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
        _exit(-1);
    }
}

void concAg( int inits[], int limits[], int nsa) {

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

    //? debug
    /* for (int j = 0; j < nsa; j++){                         
        printf("init%d: %d\n",j,inits[j]);
        printf("limit%d: %d\n",j,limits[j]);
    } */

    //! Concorrência
    
    int status;

    int fd_public[2];
    int fd_privateW[nsa+1][2];
    int fd_privateR[nsa+1][2];
    pipe(fd_public);
    /* pid_t pid;
    pid_t pid2; */
    pid_t pids[nsa];

    for (int i = 0; i < nsa+1; i++) {

        pids[i] = fork();

        if (i != nsa){

            if( pids[i] == 0 ){
                pipe(fd_privateW[i]);
                pipe(fd_privateR[i]);

                if(fork() > 0) {
                    //printf("AG %d INIT\n", i);
                    close(fd_privateR[i][READ_END]); //! close do fd privado de leitura do ag
                    close(fd_privateW[i][WRITE_END]); //! close do fd privado de escrita do ag
                    close(fd_public[READ_END]); //! close do fd publico de leitura do ag final
                    lseek(fd_venda, inits[i], SEEK_SET);

                    while(inits[i] < limits[i]){
                        read(fd_venda, vnd, sizeof(struct venda));
                        inits[i] += sizeof(struct venda);
                        write(fd_privateR[i][WRITE_END], vnd, sizeof(struct venda)); //! escreve para o fd privado de leitura do ag
                    }

                    close(fd_privateR[i][WRITE_END]); //! close do fd privado de leitura do ag

                    while( read(fd_privateW[i][READ_END], vnd, sizeof(struct venda)) > 0 ) //! lê do fd privado de escrita do ag
                    {   
                        write(fd_public[WRITE_END], vnd, sizeof(struct venda)); //! escreve para o pipe publico de onde o ag final lê
                        /* sprintf(buf_out, "<<BY AG % idArtigo: %d \n Quantidade: %d \n Total: %f \n\n", i, vnd->idArtigo, vnd->quantidade, vnd->total);
                        write(1, buf_out, strlen(buf_out));   //? debug                                                    
                        free(buf_out);
                        buf_out = (char *)malloc(MAX); */
                    }
                    close(fd_privateW[i][READ_END]); //! closes o fd de escrita do ag
                    close(fd_public[WRITE_END]); //! closes do fd publico
                    wait(&status);
                    //printf("FINAL DO AG %d\n",i);
                }
                else {
                    /* Close the write end of the pipe */
                    close(fd_privateR[i][WRITE_END]); //! closes a escrita do fd privado de leitura
                    close(fd_privateW[i][READ_END]);  //! closes a leitura do fd privado de escrita
                    /* Reads from the read end of the pipe */
                    dup2(fd_privateW[i][WRITE_END], 1); //! copia para o fd de escrita o stdout
                    dup2(fd_privateR[i][READ_END], 0);  //! copia para o fd de leitura o stdin
                    /* Execute ag */
                    execlp("./ag", "./ag", NULL);
                    _exit(-1);
                }
                _exit(0);
		    }
        }
        else {
            
            if( pids[i] == 0 ){
            pipe(fd_privateW[i]);
            pipe(fd_privateR[i]);

                if(fork() > 0) {
                    //printf("AG FINAL INIT (%d)\n",i);
                    close(fd_privateR[i][READ_END]); //! close do fd privado de leitura do ag
                    close(fd_privateW[i][WRITE_END]); //! close do fd privado de escrita do ag
                    close(fd_public[WRITE_END]); //! close do fd publico de leitura do ag final

                    while( read(fd_public[READ_END], vnd, sizeof(struct venda)) > 0 ) //! lê do fd de escrita do ag final
                    {   
                        //printf("A ESCREVER PARA O FINAL!\n");
                        write(fd_privateR[i][WRITE_END], vnd, sizeof(struct venda));
                    }
                    //printf("ANTES DO CLOSE\n"); //? debug
                    close(fd_privateR[i][WRITE_END]); //! close do fd privado de leitura do ag
                    //printf("DEPOIS DO CLOSE\n"); //? debug
                    while( read(fd_privateW[i][READ_END], vnd, sizeof(struct venda)) > 0 ) //! lê do fd privado de escrita do ag
                    {   
                        printf("A LER DO FINAL!\n");
                        write(fd_public[WRITE_END], vnd, sizeof(struct venda)); //! escreve para o pipe publico de onde o ag final lê
                        sprintf(buf_out, "<<BY AG % idArtigo: %d \n Quantidade: %d \n Total: %f \n\n", i, vnd->idArtigo, vnd->quantidade, vnd->total);
                        write(1, buf_out, strlen(buf_out));   //? debug                                                    
                        free(buf_out);
                        buf_out = (char *)malloc(MAX);
                    }

                    write(fd_logs, &limits[nsa-1], sizeof(int));
                    close(fd_logs);
                    close(fd_venda);
                    close(fd_ag);
                    close(fd_public[READ_END]);  //! closes o fd de escrita do ag final
                    wait(&status);
                    printf("Fim agregação final\n"); //? debug
                }
                else {
                    /* Close the write end of the pipe */
                    close(fd_privateR[i][WRITE_END]); //! closes a escrita do fd privado de leitura
                    close(fd_privateW[i][READ_END]);  //! closes a leitura do fd privado de escrita
                    /* Reads from the read end of the pipe */
                    dup2(fd_privateW[i][WRITE_END], 1); //! copia para o fd de escrita o stdout
                    dup2(fd_privateR[i][READ_END], 0);  //! copia para o fd de leitura o stdin
                    /* Execute ag */
                    execlp("./ag", "./ag", NULL);
                    _exit(-1);
                }
                _exit(0);
            }
        }
    }
    for (int i = 0; i < nsa+1; i++) {
        wait(&status);
    }
}

// Corre o agregador
void runAgregador( int lastVenda , int lastAg) {

    //! Logistica

    int nsa = 4;
    int nVendas = (lastVenda - lastAg) / sizeof(struct venda); //* número de vendas neste intervalo
    //printf("Número vendas: %d\n", nVendas);

    if (nVendas <= 4) {
        nsa = 1; //! Não compensa criar vários agregadores se tiver menos que 4 vendas para agregar
    }
    else if (nVendas > 4 && nVendas <= 8) {
        nsa = 2;
    }
    else if (nVendas > 8 && nVendas <= 13 ) {
        nsa = 3;
    }
    //printf("Número de agregadores: %d\n", nsa);
    int vendasAg = nVendas / nsa;
    //printf("Vendas a ler por cada agregador: %d \n", vendasAg);

    int limits[nsa];
    int inits[nsa];

    inits[0] = lastAg;
    limits[nsa-1] = lastVenda;

    for(int i = 0; i < nsa-1; i++) {
        limits[i] = inits[i+1] = inits[i] + (vendasAg * sizeof(struct venda));
    }

    if (1) { //! nsa == 1
        singleAg(lastAg, lastVenda);
    }
    else {
        concAg(inits, limits, nsa);
    }
}
