#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <ctype.h>
#include "../SV-Files/svAPI.c"
#include <sys/stat.h>
#include "../lr.h"
#define PROMPT "Cliente > "
#define PSIZE 11
static const char *PUBLIC = "/tmp/public";
static const char *RECEPTION = "/tmp/reception";

/*-----------------------------------------------------------------------*/
/*-------------------------------- MAIN ---------------------------------*/
/*-----------------------------------------------------------------------*/


int main()
{
    char *response = (char *)calloc(64, sizeof(char));
    char *buf = (char *)calloc(64, sizeof(char));
    struct lRequest *longRequest = malloc(sizeof(struct lRequest));

    char *token;
    char **args = (char **)calloc(MAX, sizeof(char **));
    int i = 0;
    int n = 0;
    int r = 0;
    int c = 0;
    int smallRequest = 0;
    int try = 0;

    //TODO: Create fifo pidR
    int pid = getpid();
    printf("########### Hello Client: %d\n", pid);
    char *fifoR = (char *)calloc(64, sizeof(char));
    sprintf(fifoR, "./Fifos/%dR",pid);
    int okR = mkfifo(fifoR,0666);

    //TODO: Create fifo pidW
    char *fifoW = (char *)calloc(64, sizeof(char));
    sprintf(fifoW, "./Fifos/%dW",pid);
    int okW = mkfifo(fifoW,0666);

    if (okR == -1 || okW == -1) {
        perror("Error\n");
        return 0;
    }

    //TODO: Open reception fifo
    //TODO: connect to sv fifo reception
    int receptionW = open(RECEPTION, O_WRONLY);
    while( (c = write(receptionW, &pid, sizeof(int))) == -1 && try < 30) {
        printf("Write: %d",c);
        sleep(1);
    }
    if (try >= 30) {
        write(1,"Server error envie de novo por favor!\n",39);
        unlink(fifoR);
        unlink(fifoW);
        return 0;
    }

    try = 0;

    //TODO: open read fifo pidR
    int fdRead = open(fifoR, O_RDONLY);
    if (read(fdRead, response, 80) > 0){
        //TODO: print connect message
        //write(1, response, strlen(response));
        printf("> Client %d connected\n", pid);
        response = (char *)calloc(64, sizeof(char));
    }
    //TODO: open write fifo pidW
    int fdWrite = open(fifoW, O_WRONLY);

    //TODO: open write fifo public
    int publicW = open(PUBLIC, O_WRONLY);

    //TODO: lê do stdin enquando /= ctrl+D
    while( (n = readln(0, buf, 64)) > 0) {

        if (strcmp(buf,"\n") && isdigit(buf[0]) && n > 0)
        { 
            token = strtok(buf, " ");

            while (token != NULL)
            {
                args[i] = strdup(token);
                token = strtok(NULL, " ");
                i++;
            }

            args[i] = NULL;

            if (i < 1 || i > 2)
            {
                write(1, "Número de argumentos inválidos\n", 34);
            }
            else
            {
                switch (i)
                {
                    //TODO: se for procura envia para o pidW e lê a resposta do pidW
                    case 1: smallRequest = atoi(args[0]); //prepara a mensagem
                            if (smallRequest >= 0) {
                                write(fdWrite, &smallRequest, sizeof(int)); //envia para o privado
                                r = read(fdRead, response, 64); // recebe do privado
                                write(1, response, r); // escreve a resposta
                            }
                            else {
                                write(1, "Todos os ids são maiores ou iguais a 0\n", 41);
                            }
                    break;

                    //TODO: se for update stock envia para o public e lê do pidR ????
                    case 2: longRequest = malloc(sizeof(struct lRequest)); //prepara a mensagem
                            longRequest->type = 1;
                            longRequest->pid = pid;
                            longRequest->idArtigo = atoi(args[0]);
                            longRequest->quantidade = atoi(args[1]);

                            while(write(publicW, longRequest, sizeof(struct lRequest)) == -1 && try < 30) { // envia para o publico
                                sleep(1);
                                try++;
                            }
                            if (try < 30) {
                                r = read(fdRead, response, 64); // recebe do privado
                                write(1, response, r); // escreve a resposta
                            }
                            else {
                                write(1,"Server error envie de novo por favor!\n",39);
                            }
                            try = 0;
                    break;
                }
            }
        }

        response = (char *)calloc(64, sizeof(char));
        buf = (char *)calloc(64, sizeof(char));
        args = (char **)calloc(MAX, sizeof(char **));
        i = 0;
    }
    smallRequest = -10;
    write(fdWrite, &smallRequest, sizeof(int));
    
    //TODO: envia um exit para pidW para dar exit no fork
    printf("Bye Client: %d #########\n", pid);

    close(fdRead);
    close(fdWrite);
    //TODO: unlink dos privados
    unlink(fifoR);
    unlink(fifoW);
    return 0;
}