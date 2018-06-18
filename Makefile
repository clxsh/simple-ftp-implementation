VPATH = server:libs:client

all : fserver fclient
.PHONY : all

fserver : ftp_server.o functions.o
	gcc server/ftp_server.o libs/functions.o -lpthread -o server/fserver

fclient : ftp_client.o functions.o
	gcc client/ftp_client.o libs/functions.o -lpthread -o client/fclient 

ftp_client.o : ftp_client.c functions.h
	gcc -c -o client/ftp_client.o client/ftp_client.c

ftp_server.o : ftp_server.c functions.h
	gcc -c -o server/ftp_server.o server/ftp_server.c

functions.o : functions.h functions.c
	gcc -c -o libs/functions.o libs/functions.c

.PHONY : clean
clean :
	-rm server/ftp_server.o client/ftp_client.o libs/functions.o server/fserver client/fclient
