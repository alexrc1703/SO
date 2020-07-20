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
#include <time.h>
#include "../SV-Files/sv.h"
static const char *TMP = "./BaseDados/tmpAG";

void printAg(int fpTmp, int totalS){
    lseek(fpTmp, 0, SEEK_SET);
    struct venda *vnd = malloc(sizeof(struct venda));
    int n = 0;

    while (totalS > 0)
    {
        n = read(fpTmp, vnd, sizeof(struct venda));
        //write(1, vnd, sizeof(struct venda));

        if ( n > 0 && vnd->quantidade != 0){
            write(1, vnd, sizeof(struct venda));
            totalS--;
        }
        else {
            lseek(fpTmp, sizeof(struct venda), SEEK_CUR);
        }
    }

    free(vnd);
    
}

int insereVenda(struct venda *vnd, int fpTmp){
    struct venda *vnd2 = malloc(sizeof(struct venda));
    int n = 0;

    //TODO: lseek para o id da vendar
    lseek(fpTmp, vnd->idArtigo * sizeof(struct venda), SEEK_SET);

    //TODO: confirma se já existe algo escrito
    n = read(fpTmp, vnd2, sizeof(struct venda));
    //write(1, vnd2, sizeof(struct venda));

    //TODO: caso não exista escreve
    if (n <= 0 || vnd2->quantidade <= 0){
        lseek(fpTmp, vnd->idArtigo * (sizeof(struct venda)), SEEK_SET);
        write(fpTmp, vnd, sizeof(struct venda));
        //write(1, vnd, sizeof(struct venda));
        free(vnd2);
        return 1;
    }
    //TODO: caso exista soma
    else {
        vnd2->quantidade += vnd->quantidade;
        vnd2->total += vnd->total;
        lseek(fpTmp, vnd->idArtigo * (sizeof(struct venda)), SEEK_SET);
        write(fpTmp, vnd2, sizeof(struct venda));
        //write(1, vnd2, sizeof(struct venda));
        free(vnd2);
        return 0;
    }

    return 0;
}

int main()
{
    //TODO: Cria um ficheiro temporário para guardar as agregações
    int fpTmp = open(TMP, O_CREAT | O_RDWR, 0777);

    struct venda *vnd = malloc(sizeof(struct venda));
    int totalS = 0;

    while( read(0, vnd, sizeof(struct venda)) > 0 ){

        //write(1, vnd, sizeof(struct venda));
        totalS += insereVenda(vnd, fpTmp);
        free(vnd);
        vnd = malloc(sizeof(struct venda));

    }
    printAg(fpTmp, totalS);
    free(vnd);
    close(fpTmp);
    unlink(TMP);

    return 0;
}