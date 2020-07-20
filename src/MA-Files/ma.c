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
static const char *VENDAS = "./BaseDados/vendas";
static const char *LOGS = "./BaseDados/logs";


/*-----------------------------------------------------------------------*/
/*-------------------------------- MAIN ---------------------------------*/
/*-----------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    printMan();
    char *buf = (char *)calloc(MAX, sizeof(char *));
    char *buf_sv = (char *)calloc(MAX, sizeof(char *));
    int n = 1;
    char *token;

    //guarda em cada linha os argumentos passados
    char **args = (char **)calloc(MAX, sizeof(char **));
    int i = 0;

    if (access(LOGS, F_OK) == -1)
    {
        int logs = 0;
        int fd = open(LOGS, O_CREAT | O_WRONLY | O_APPEND, 0777);
        write(fd, &logs, sizeof(int));
        write(fd, &logs, sizeof(int));
    }


    //* Criação dos fifos para a escrita e leitura
    int ma_server;
    char *write_fifo = "/tmp/client_server";

    // write(1, PROMPT, 4);

    while ((n = read(0, buf, MAX)) > 0)
    {
        trim(buf);
        token = strtok(buf, " ");

        while (token != NULL)
        {
            args[i] = strdup(token);
            token = strtok(NULL, " ");
            i++;
        }

        args[i] = NULL;

        if (i < 1 || i > 3)
        {
            write(1, "Número de argumentos inválidos\n", 34);
        }
        else
        {
            switch (args[0][0])
            {
            case 'i':
                if (i <= 2)
                {
                    write(1, "Para inserir um artigo precisa de <nome artigo> <preço> !!\n", 61);
                }
                else if(atof(args[2]) > 0)
                {
                    insere(args[1], atof(args[2]));
                }
                else
                {
                    write(1,"Insira um preço válido, por favor!\n",38);
                }
                break;
            case 'n':
                if (i <= 2)
                {
                    write(1, "Para mudar o nome de um artigo precisa de <id artigo> <novo nome> !!\n", 70);
                }
                else
                {
                    setNome(atoi(args[1]), args[2]);
                }
                break;
            case 'p':
                if (i <= 2)
                {
                    write(1, "Para mudar o preço de um artigo precisa de <id artigo> <novo preço> !!\n", 74);
                }
                else if(atof(args[2]) > 0)
                {
                    setPreco(atoi(args[1]), atof(args[2]));
                    ma_server = open(write_fifo, O_WRONLY);
                    sprintf(buf_sv, "ma %s %s\n", args[1], args[2]);
                    write(ma_server, buf_sv, strlen(buf_sv));
                    buf_sv = (char *)calloc(MAX, sizeof(char *));

                }
                else
                {
                    write(1,"Insira um preço válido, por favor!\n",38);
                }
                break;

            case 'r':
                if (i != 2)
                {
                    write(1, "Para ler um artigo precisa apenas do <id artigo> !!\n", 53);
                }
                else
                {
                    readArtigoById(atoi(args[1]));
                }
                break;

            case 'h':
                if (i != 1)
                {
                    write(1, "Para aceder ao help precisa apenas de inserir o comando h !!\n", 45);
                }
                else
                {
                    printMan();
                }
                break;

            case 'd':
                unlink(ARTIGOS);
                unlink(STRINGS);
                unlink(TMP);
                unlink(STOCKS);   
                unlink(LOGS); 
                unlink(VENDAS);
                system("exec rm -r ../Agregacao/*");
                break;
            
            case 'a':
                ma_server = open(write_fifo, O_WRONLY);
                sprintf(buf_sv, "ma ag");
                write(ma_server, buf_sv, strlen(buf_sv));
                buf_sv = (char *)calloc(MAX, sizeof(char *));
                break;

            default:
                write(1, "Argumento inválido!\n", 22);
                break;
            }
        }

        //a cada "refresh" reseta o conteúdo da memória alocada, evitando que imprima lixo
        buf = (char *)calloc(MAX, sizeof(char *));
        //reseta a posição a cada comando
        i = 0;
        // write(1, PROMPT, 4);
    }

    // close(ma_server);

    return 0;
}

/*---------------------------------------------------------------------*/