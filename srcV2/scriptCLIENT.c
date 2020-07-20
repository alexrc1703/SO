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
#define MAX 1024
#define READ_END    0 
#define WRITE_END   1


/*-----------------------------------------------------------------------*/
/*-------------------------------- MAIN ---------------------------------*/
/*-----------------------------------------------------------------------*/

int main(int argc, char *argv[])
{

    
    int pid;

    for(int i = 0; i < 10; i++)
    {
        pid = fork();
        if(pid == 0){
            int fd = open("script", O_RDONLY);
            dup2(fd,0);
            close(fd);
            execlp("./cv","./cv",NULL);
            _exit(-1);
        }    
    }
    
    return 0;

}