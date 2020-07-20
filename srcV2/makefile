CC = gcc
CFLAGS = -Wall

ma: ma.o maAPI.o
	$(CC) $(CFLAGS) -o ma ma.o maAPI.o

ma.o: ./MA-Files/ma.c 
	$(CC) $(CFLAGS) -c ./MA-Files/ma.c

maAPI.o: ./MA-Files/maAPI.c ./MA-Files/ma.h 
	$(CC) $(CFLAGS) -c ./MA-Files/maAPI.c

sv: sv.o svAPI.o
	$(CC) $(CFLAGS) -o sv sv.o svAPI.o

sv.o: ./SV-Files/sv.c 
	$(CC) $(CFLAGS) -c ./SV-Files/sv.c

svAPI.o: ./SV-Files/svAPI.c ./SV-Files/sv.h 
	$(CC) $(CFLAGS) -c ./SV-Files/svAPI.c

cv: cv.o
	$(CC) $(CFLAGS) -o cv cv.o 

cv.o: ./CV-Files/cv.c
	$(CC) $(CFLAGS) -c ./CV-Files/cv.c

ag: ag.o bst.o
	$(CC) $(CFLAGS) -o ag ag.o bst.o

ag.o: ./AG-Files/ag.c ./AG-Files/bst.h
	$(CC) $(CFLAGS) -c ./AG-Files/ag.c 

bst.o: ./AG-Files/bst.c ./AG-Files/bst.h
	$(CC) $(CFLAGS) -c ./AG-Files/bst.c

sca: scriptMA.o
	$(CC) $(CFLAGS) -o sca scriptMA.o

sca.o: scriptMA.c
	$(CC) $(CFLAGS) -c scriptMA.c

scc: scriptCLIENT.o
	$(CC) $(CFLAGS) -o scc scriptCLIENT.o

scc.o: scriptCLIENT.c
	$(CC) $(CFLAGS) -c scriptCLIENT.c

script: sca scc

all: sv cv ag ma sca scc

cleanAgregacao:
	rm -v ./Agregacao/*

cleanBaseDados:
	rm -rf ./BaseDados/logs ./BaseDados/logsSV ./BaseDados/artigos ./BaseDados/stocks ./BaseDados/strings ./BaseDados/vendas

cleanFiles:
	rm -rf ./BaseDados/logs ./BaseDados/logsSV ./BaseDados/artigos ./BaseDados/stocks ./BaseDados/strings ./BaseDados/vendas ./Agregacao/* ./Fifos/*

clean:
	rm -rf ma ma.o maAPI.o sv sv.o svAPI.o cv cv.o ag ag.o bst.o sca scriptMA.o scc scriptCLIENT.o