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
// #include "bst.h"
#define MAX 1024
static const char *VENDAS = "./BaseDados/vendas";

/*-----------------------------------------------------------------------*/
/*-------------------------------- MAIN ---------------------------------*/
/*-----------------------------------------------------------------------*/

int main(int argc, char *argv[])
{

    char *buf = (char *)calloc(MAX, sizeof(char *));
    char *token;
    char **args = (char **)calloc(MAX, sizeof(char **));
    int i = 0;
    int status;

    pid_t pid;

    //* Criação dos fifos para a escrita e leitura
    int client_server;
    char *read_fifo = "/tmp/client_server";

    mkfifo(read_fifo, 0666);


    if((pid = fork()) == 0) { //! fork para tratamento dos pedidos de cliente e ma

        //* open dos fifos para a escrita e leitura
        client_server = open(read_fifo, O_RDONLY);

        //* inicialização da cache
        
        while(1) {
            
            while (read(client_server, buf, 1024))
            {
                trim(buf);
                write(1,buf,strlen(buf));

                token = strtok(buf, " ");

                while (token != NULL)
                {
                    args[i] = strdup(token);
                    token = strtok(NULL, " ");
                    i++;
                }

                args[i] = NULL;

                if (i < 2 || i > 4)
                {
                    perror("Número de argumentos inválidos");
                }
                else
                {
                    

                    default:
                        write(1, "Argumentos inválidos!\n", 24);
                        break;
                    }
                }
                //a cada "refresh" reseta o conteúdo da memória alocada, evitando que imprima lixo
                buf = (char *)calloc(MAX, sizeof(char *));
                //reseta a posição a cada comando
                i = 0;
            }
        }
        _exit(0);
    }    

    else {
        wait(&status);
    }

    return 0;
}

