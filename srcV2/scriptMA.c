#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include "./MA-Files/maAPI.c"
#define MAX 1024
#define READ_END    0 
#define WRITE_END   1


/*-----------------------------------------------------------------------*/
/*-------------------------------- MAIN ---------------------------------*/
/*-----------------------------------------------------------------------*/


int main(int argc, char *argv[])
{

    char *buf = (char *)calloc(MAX, sizeof(char *));
    char *name = (char *)calloc(MAX, sizeof(char *));
    char *token;
    int n=0;
    int j;

    //guarda em cada linha os argumentos passados
    char **args = (char **)calloc(MAX, sizeof(char **));
    int i = 0;

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

        if (i!= 2)
        {
            if(fork()==0){
            write(1, "Número de argumentos inválidos\n", 34);
            }
            
        }
        else if (!strcmp(args[0],"i"))
        {
            for(int j=0; j<atoi(args[1]); j++){
                sprintf(name,"art%d",j);
                float preco = (rand() % (5000 + 1 - 1)) + 1;
                insere(name,preco);
            }
            break;
        }

        //a cada "refresh" reseta o conteúdo da memória alocada, evitando que imprima lixo
        buf = (char *)calloc(MAX, sizeof(char *));
        //reseta a posição a cada comando
        i = 0;
    }
    return 0;
}