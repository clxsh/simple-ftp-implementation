#include "../libs/functions.h"

int fserver_list(int control_sock, int data_sock);
void *client_serve(void *vargp);
int open_server_data_sock(int sockfd);
int fserver_get(int control_sock, int data_sock, char *filename);
int process_login(int sockfd);
int check(char *username, char *password);

int main(int argc, char **argv)
{
	int listenfd, *connfdp;
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;
	char client_hostname[MAXLINE], client_port[MAXLINE];
	pthread_t tid;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(0);
	}

	listenfd = open_listenfd(argv[1]);
	while (1) {
		clientlen = sizeof(struct sockaddr_storage);

		connfdp = malloc(sizeof(int));                   // avoid racing
		*connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);

		Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
		printf("Connected to (%s %s)\n", client_hostname, client_port);

		Pthread_create(&tid, NULL, client_serve, connfdp);

	}
	Close(listenfd);       //

	return 0;
}

//问题：客户端ctl+c，服务端也会退出
/*
 * client_serve - client process routine
 */
void *client_serve(void *vargp)
{
	int control_sock = *((int *)vargp);
	char cmd[MAXLINE];
	char *argv[MAXARGS];
	int rc;

	Pthread_detach(pthread_self());
	free(vargp);

	send_code(control_sock, 220);     //send welcome code

	if (process_login(control_sock) == 0) {  //verify success
		send_code(control_sock, 230);        //buggy
	}
	else {
		send_code(control_sock, 430);
		Pthread_cancel(pthread_self());
	}

	//list后出现段错误
	while (1) {
		Recv(control_sock, cmd, MAXLINE, 0);
		parseline(cmd, argv);

		if (strcmp(argv[0], "quit") == 0)
			rc = 221;
		else if ((strcmp(argv[0], "list") == 0) || (strcmp(argv[0], "get") == 0))
			rc = 200;
		else
			rc = 502;

		send_code(control_sock, rc);
		if (rc == 221) break;

		if (rc == 200) {
			int data_sock;
			if ((data_sock = open_server_data_sock(control_sock)) < 0) {
				fprintf(stderr, "establish data_sock failed\n");
				continue;
			}

			if (strcmp(argv[0], "list") == 0) {
				fserver_list(control_sock, data_sock);
			}
			else if(strcmp(argv[0], "get") == 0) {
				fserver_get(control_sock, data_sock, argv[1]);
			}

			Close(data_sock);
		}
	}

	Close(control_sock);

}

int fserver_list(int control_sock, int data_sock)
{
	char *data = malloc(sizeof(char) * MAXLINE );
	size_t num_read;
	FILE *fd;

	int rs = system("ls -l | tail -n+4 > tmp.txt");

	if (rs < 0) {
		Pthread_cancel(pthread_self());
	}

	fd = fopen("tmp.txt", "r");
	if (!fd) {
		Pthread_cancel(pthread_self());
	}

	fseek(fd, SEEK_SET, 0);

	send_code(control_sock, 1);

	memset(data, 0, MAXLINE);

	while ((num_read = fread(data, 1, MAXLINE, fd)) > 0) {
		Send(data_sock, data, num_read, 0);
		memset(data, 0, MAXLINE);
	}

	fclose(fd);
	
	send_code(control_sock, 226);
	
	return 0;
}

int open_server_data_sock(int sockfd)
{
	int tmp;
	Recv(sockfd, &tmp, sizeof(tmp), 0);

	struct sockaddr_in sai;
	socklen_t len = sizeof(sai);

	Getpeername(sockfd, (SA *) &sai, &len);
	
	char buf[MAXLINE];
	inet_ntop(AF_INET, &sai.sin_addr, buf, MAXLINE);

	int iport = CPORT;
	char *port = malloc(sizeof(char) * 33);
	sprintf(port, "%d", iport);

	return open_clientfd(buf, port);
}

int fserver_get(int control_sock, int data_sock, char *filename)
{
	FILE *fp = fopen(filename, "r");
	char data[MAXLINE];
	size_t num_read;

	if (!fp) {
		send_code(control_sock, 550);
	}
	else {
		send_code(control_sock, 150);

		memset(data, 0, MAXLINE);
		while ((num_read = fread(data, 1, MAXLINE, fp)) > 0) {
			Send(data_sock, data, MAXLINE, 0);
			memset(data, 0, MAXLINE);
		}

		send_code(control_sock, 226);

		fclose(fp);
	}

	return 0;
}


int process_login(int sockfd)
{
	char username[MAXLINE], password[MAXLINE];

	if (Recv(sockfd, username, MAXLINE, 0) < 0) {
		app_error("Recv username error");
		Pthread_cancel(pthread_self());
	}

	send_code(sockfd, 331);

	if (Recv(sockfd, password, MAXLINE, 0) < 0) {
		app_error("Recv password error");
		Pthread_cancel(pthread_self());
	}

	return check(username, password);
}

/*
 * check - check username and password
 * return 0 when success, -1 when fail
 */
int check(char *username, char *password)
{
	return 0;
}
