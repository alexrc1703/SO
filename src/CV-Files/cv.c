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
#define PROMPT "Cliente > "
#define PSIZE 11

/*-----------------------------------------------------------------------*/
/*-------------------------------- MAIN ---------------------------------*/
/*-----------------------------------------------------------------------*/


int main()
{
    int n = 1;
    char *buf = (char *)calloc(1024, sizeof(char *));
    char buf1[50];
    char *response = (char *)calloc(1024, sizeof(char *));

    //* Criação dos fifos para a escrita e leitura
    int client_server;
    char *write_fifo = "/tmp/client_server";
    client_server = open(write_fifo, O_WRONLY); //* open dos fifos para a escrita e leitura

    int server_client;

    char *pid = (char *)calloc(64, sizeof(char));
    int z = getpid();
    sprintf(pid, "%d", z);
    char *read_fifo = pid;
    mkfifo(read_fifo, 0666);
    int try = 0;

    while (n > 0)
    {
        
        write(1, PROMPT, PSIZE);
        n = read(0, buf, 1024);
        trim(buf);
        if (strcmp(buf,"\n") && isdigit(buf[0]) && n > 0)
        {
            
            sprintf(buf1,"cv %s %s",pid, buf);
            
            while(write(client_server, buf1, strlen(buf1)) == -1 && try++ < 30) {
                sleep(1);
            }

            if(try < 30){
                server_client = open(read_fifo, O_RDONLY);       
                read(server_client, response, 1024);

                write(1, response, strlen(response));  
            }
            else {
                write(1,"Server error envie de novo por favor!\n",39);
            }
            
        }
        try = 0;

        memset(response,0,1024*sizeof(response[0]));
    }

    unlink(read_fifo);
    return 0;
}