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
#include "sv.h"
#include "../lr.h"
#include "../reply.h"
// #include "bst.h"
#define MAX 1024
static const char *VENDAS = "./BaseDados/vendas";
static const char *PUBLIC = "/tmp/public";
static const char *RECEPTION = "/tmp/reception";
static const char *LOGS = "./BaseDados/logsSV";

/*-----------------------------------------------------------------------*/
/*-------------------------------- MAIN ---------------------------------*/
/*-----------------------------------------------------------------------*/
int done = 0;
int pidReception = 0;
int pidPublic = 0;
 
void killServer(int signum)
{
    kill(pidReception, SIGUSR1);
    kill(pidPublic, SIGUSR1);
}

void handlerS1(int sig)
{
    done = 1;
}

void handler(int sig)
{
    wait(NULL);
}

int main()
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = killServer;
    sigaction(SIGTERM, &action, NULL); //! serve para receber o sinal sigterm e matar o servidor

    signal(SIGCHLD, handler); //! Previne os processos zombies

    int stop = 1;
    int status;

    int clientPid;
    int smallRequest;

    int fpvenda = open(VENDAS, O_RDONLY , 0777);
    int fplogSV;
    int begin = 0;
    
    if (fpvenda == -1) {
        fpvenda = open(VENDAS, O_CREAT | O_WRONLY | O_APPEND, 0777);
        fplogSV = open(LOGS, O_CREAT | O_WRONLY, 0777);
        write(fplogSV, &begin, sizeof(int));
        read(fplogSV, &begin, sizeof(int));
        close(fplogSV);
    }
    close(fpvenda);

    //TODO: Cria o fifo publico
    int mkPublic = mkfifo(PUBLIC, 0666);
    if (mkPublic == -1){
        perror("Error Public");
    }

    //TODO: Cria o fifo reception
        int mkReception = mkfifo(RECEPTION, 0666);
        if (mkReception == -1){
            perror("Error Reception");
        }

    //TODO: FORK Rececionista
    // FILHO 1
    if ((pidReception = fork()) == 0) {
        signal(SIGUSR1,handlerS1);
        //TODO: open do fifo reception
        int fdReception;
        fdReception = open(RECEPTION, O_RDONLY);

        while(!done){

            while ( read(fdReception, &clientPid, sizeof(int)) > 0 ) {

                if (fork() == 0) {

                    printf("Client %d in!\n", clientPid);

                    char *fifoW = (char *)malloc(sizeof(char));
                    char *fifoR = (char *)malloc(sizeof(char));
                    sprintf(fifoW,"./Fifos/%dR",clientPid);
                    sprintf(fifoR,"./Fifos/%dW",clientPid);

                    //TODO: liga-se ao fifoR do cliente e envia uma resposta a avisar que está connectado
                    int fdWrite = open(fifoW, O_WRONLY);


                    //TODO: liga-se ao fifoW do cliente
                    int fdRead = open(fifoR, O_RDONLY);

                    //TODO: fica a ler do fifoW do client
                    while( read(fdRead, &smallRequest, sizeof(int)) && stop > 0 && !done){

                        if (smallRequest != -10) {
                            
                            getStock(smallRequest, fdWrite);
                            
                        }
                        else {
                            stop = 0;
                        }

                    }
                    close(fdWrite);
                    close(fdRead);
                    //TODO: caso o cliente saia então o fork é terminado
                    printf("Client %d out!\n", clientPid);
                    _exit(0);
                }
            } 
        }
        close(fdReception);
        printf("A fechar Receção!\n");
        _exit(0);
    }
    else {
        //TODO: FORK fifo publico
        //FILHO 2
        if ((pidPublic = fork()) == 0) {
            signal(SIGUSR1,handlerS1);

            struct lRequest *longRequest = malloc(sizeof(struct lRequest)); //! Usado para ler os pedidos dos clientes e do ma

            //TODO: open do fifo public
            int fdPublic;
            int n = 0;

            while(!done) {
                fdPublic = open(PUBLIC, O_RDONLY);

                while( (n = read(fdPublic, longRequest, sizeof(struct lRequest))) > 0 && !done ) {

                    //TODO: verifica se é um pedido do cv (update stock)
                    if(longRequest->type == 1){
                        updateStock(longRequest->idArtigo, longRequest->quantidade, longRequest->pid);
                    }
                        
                    //TODO: verifica se é um pedido do ma (update preço ou pedido de agregação)
                    else if (longRequest->type == 2){
                        write(1,"updateStock\n",13);
                        //!updateCache(longRequest->idArtigo, longRequest->preco, tree);
                    }

                    else if (longRequest->type == 3){
                        int fd_venda = open(VENDAS, O_RDONLY, 0777);
                        int lastVenda = lseek(fd_venda, 0, SEEK_END); //! fim da ultima venda inserida
                        int fd_logs = open(LOGS, O_RDWR, 0777); //! ultima agregação
                        int lastAg;
                        read(fd_logs, &lastAg, sizeof(int));
                        //printf("Last venda: %d\n", lastVenda); //? debug
                        //printf("Last agregação: %d\n", lastAg); //? debug
                        close(fd_venda);
                        close(fd_logs);

                        if (lastVenda != -1 && lastVenda != lastAg){
                            if (fork() == 0)
                            {
                                //write(1, "run agregador\n", 15);
                                runAgregador( lastVenda, lastAg);
                                _exit(0);
                            }
                        }
                        else {
                            write(1, "NADA PARA AGREGAR!\n", 20);
                        }
                    }
                }
                close(fdPublic);
            }
            printf("A fechar Public!\n");
            _exit(0);
        }
        else {
            printf("Waiting for Public and Reception\n");
            //wait(&status);
             for(int w = 0; w < 2; w++){
                wait(&status);
            }
        } 
    }

    printf("A fechar Server!\n");
    unlink(PUBLIC);
    unlink(RECEPTION);
    return 0;
}
