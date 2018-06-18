#include "functions.h"

/*
 * open_listenfd - open and bind port
 */
int open_listenfd(char *port)
{
	struct addrinfo hints, *listp, *p;
	int listenfd, optval = 1;

	/* get a list of potential server address */
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG | AI_NUMERICSERV;
	Getaddrinfo(NULL, port, &hints, &listp);

	for (p = listp; p; p = p->ai_next) {
		if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
		  continue;

		/* eliminates "address already in use" error from bind */
		Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
		
		if(bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
		  break; //success

		Close(listenfd);
	}

	freeaddrinfo(listp);
	if (!p)
	  return -1;

	if (listen(listenfd, LISTENQ) < 0) {
		Close(listenfd);
		return -1;
	}

	return listenfd;
}

/*
 * open_clientfd - connect to server
 */
int open_clientfd(char *host, char *port)
{
	int clientfd;
	struct addrinfo hints, *listp, *p;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICSERV | AI_ADDRCONFIG;
	Getaddrinfo(host, port, &hints, &listp);

	for (p = listp; p; p = p->ai_next) {
		if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
		  continue;

		if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
		  break;

		Close(clientfd);
	}

	freeaddrinfo(listp);
	if (!p)
	  return -1;
	else
	  return clientfd;
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int ret;
	if ((ret = accept(sockfd, addr , addrlen)) < 0) {
		unix_error("accept error");
	}

	return ret;
}

int Send(int sockfd, const void *buf, size_t len, int flags)
{
	int ret;
	if ((ret = send(sockfd, buf, len, flags)) < 0) {
		unix_error("send error");
	}
	return ret;
}

int Recv(int sockfd, void *buf, size_t len, int flags)
{
	int ret;
	if ((ret = recv(sockfd, buf, len, flags)) < 0) {
		unix_error("recv error");
	}

	return ret;
}

/* Getaddrinfo - wrapper for the getaddrinfo function */
void Getaddrinfo(const char *host, const char *service, const struct addrinfo *hints, struct addrinfo **result)
{
	int rc;

	if ((rc = getaddrinfo(host, service, hints, result)) != 0) {
		gai_error(rc, "getaddrinfo error");
	}
}

void Getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, size_t hostlen, char *service, size_t servlen, int flags)
{
	int ret;
	if ((ret = getnameinfo(sa, salen, host, hostlen, service, servlen, flags)) != 0) {
		gai_error(ret, "getnameinfo error");
	}
}

void Getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	if (getpeername(sockfd, addr, addrlen) < 0) {
		unix_error("getpeername error");
	}
}

/* Setsockopt - wrapper for the setsockopt function */
void Setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
	if (setsockopt(sockfd, level, optname, optval, optlen) < 0) {
		unix_error("setsockopt error");
	}
}

/* Close - wrapper fot the close function */
void Close(int fd)
{
	if (close(fd) != 0) {
		unix_error("close error");
	}
}

/* Pthread_create - wrapper for the pthread_create */
void Pthread_create(pthread_t *tid, pthread_attr_t *attr, void *(*f) (void *), void *arg)
{
	int ret;

	if ((ret = pthread_create(tid, attr, f, arg)) != 0) {
		posix_error(ret, "pthread_create error");
	}
}

/* Pthread_detach - wrapper for pthread_detach */
void Pthread_detach(pthread_t tid)
{
	int ret;
	if ((ret = pthread_detach(tid)) != 0) {
		posix_error(ret, "pthread_detach error");
	}
}

void Pthread_cancel(pthread_t pid)
{
	int ret;
	if ((ret = pthread_cancel(pid)) != 0) {
		posix_error(ret, "pthread_cancel error");
	}
}

/* gai_error - gai style error wrapper function */
void gai_error(int code, char *msg)
{
	fprintf(stderr, "%s: %s\n", msg, gai_strerror(code));
	exit(0);
}

/* unix_error - unix style error wrapper function */
void unix_error(char *msg)
{
	fprintf(stderr, "%s: %s\n", msg, strerror(errno));
	exit(0);
}

void posix_error(int code, char *msg)
{
	fprintf(stderr, "%s %s\n", msg, strerror(code));
	exit(0);
}

void app_error(char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(0);
}


/*
 * send_code - send the answer back code
 */
void send_code(int sockfd, int code)
{
	Send(sockfd, &code, sizeof(code), 0);
}

/*
 * recv_code - receive the code
 */
int recv_code(int sockfd)
{
	int code;
	recv(sockfd, &code, sizeof(code), 0);
	
	return code;
}




int parseline(const char *cmd, char **argv)
{
	static char array[MAXLINE];
	char *buf = array;
	char *delim;
	int argc = 0;

	strcpy(buf, cmd);
	buf[strlen(buf) - 1] = ' ';

	while (*buf && (*buf == ' ')) {
		buf++;
	}
	delim = strchr(buf, ' ');

	while (delim) {
		argv[argc++] = buf;
		*delim = '\0';
		buf = delim + 1;

		while (*buf && (*buf == ' ')) {
			buf++;
		}

		delim = strchr(buf, ' ');
	}

	argv[argc] = NULL;

	if (argc == 0) return -1;

	return 0;
}

