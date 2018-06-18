#ifndef __functions_h__
#define __functions_h__

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define MAXLINE 256
#define LISTENQ 1024
#define MAXARGS 5
#define CPORT 2333

typedef struct sockaddr SA;

int open_listenfd(char *port);
int open_clientfd(char *host, char *port);
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int Send(int sockfd, const void *buf, size_t len, int flags);
int Recv(int sockfd, void *buf, size_t len, int flags);

void Getaddrinfo(const char *host, const char *service, const struct addrinfo *hints, struct addrinfo **result);
void Getnameinfo(const struct sockaddr *sa, socklen_t salen,char *host, size_t hostlen, char *service, size_t servlen, int flags);
void Getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

void Setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
void Close(int fd);

void Pthread_create(pthread_t *tid, pthread_attr_t *attr, void *(*f)(void *), void *arg);
void Pthread_cancel(pthread_t tid);
void Pthread_detach(pthread_t tid);

void gai_error(int code, char *msg);
void unix_error(char *msg);
void posix_error(int code, char *msg);
void app_error(char *msg);

void send_code(int sockfd, int code);
int recv_code(int sockfd);
int parseline(const char *cmd, char **argv);


#endif
