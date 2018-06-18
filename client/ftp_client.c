#include "../libs/functions.h"

#define VERBOSE 1             //whether output the verbose infomation

void print_reply(int code);
void client_login();
int fclient_list(int control_sock, int data_sock);
int open_client_data_sock(int sockfd);
int fclient_get(int control_sock, int data_sock, char *filename);


int main(int argc, char **argv)
{
	int control_sock;
	char cmd[MAXLINE], *arguments[MAXARGS];
	if (3 != argc) {
		fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
		exit(0);
	}

	char *host = argv[1];
	char *port = argv[2];
	
	/* connect to server */
	control_sock = open_clientfd(host, port);
	printf("connect to %s.\n", host);
	VERBOSE? print_reply(recv_code(control_sock)) : recv_code(control_sock);

	/* get and send username,password */
	client_login(control_sock);

	while (1)
	{
		printf("fclient> ");
		fflush(stdout);

		if((fgets(cmd, MAXLINE, stdin) == NULL) && ferror(stdin)) {
			app_error("fgets error");
		}

		if (parseline(cmd, arguments) != 0) {
			continue;
		}

		Send(control_sock, cmd, MAXLINE, 0);

		int answer_code = recv_code(control_sock);

		if (answer_code == 221) {
			print_reply(221);
			break;
		}

		if (answer_code == 502) {
			print_reply(502);
		}
		else {
			int data_sock;
			if ((data_sock = open_client_data_sock(control_sock)) < 0) {
				fprintf(stderr, "establish data_sock failed\n");
				exit(0);
			}

			if (strcmp(arguments[0], "list") == 0) {
				fclient_list(control_sock, data_sock);
			}
			else if(strcmp(arguments[0], "get") == 0) {
				fclient_get(control_sock, data_sock, arguments[1]);
			}

			Close(data_sock);
		}
	}


	return 0;
}

void print_reply(int code)
{
	switch (code)
	{
		case 220:
			printf("220 Welcome, server ready.\n");
			break;
		case 221:
			printf("221 Goodbye\n");
			break;
		case 226:
			printf("226 Closing data connection. Request file action successful\n");
			break;
		case 550:
			printf("550 Requested action not token. File unavailable\n");
			break;
		case 430:
			printf("Invalid username/password\n");
			break;
		case 230:
			printf("Login successfully\n");
			break;
		case 502:
			printf("Invalid command\n");
			break;
	}
}

void client_login(int sockfd)
{
	char username[MAXLINE];

	printf("username: ");
	fflush(stdout);
	if((fgets(username, MAXLINE, stdin) == NULL) && ferror(stdin)) {
		app_error("fgets error");
	}

	Send(sockfd, username, MAXLINE, 0);

	int wait;
	Recv(sockfd, &wait, sizeof(wait), 0);   //wait answer back code

	char *password = getpass("password:");
	if (strlen(password) == 0) {
		password = malloc(sizeof(char) * 2);
		password[0] = ' ';
		password[1] = '\0';
	}
	Send(sockfd, password, (int)strlen(password), 0);

	print_reply(recv_code(sockfd));
}

int fclient_list(int control_sock, int data_sock)
{
	size_t num_recvd;
	char buf[MAXLINE];

	recv_code(control_sock);

	memset(buf, 0, MAXLINE);

	while ((num_recvd = recv(data_sock, buf, MAXLINE, 0)) > 0) {
		printf("%s", buf);
		memset(buf, 0, MAXLINE);
	}

	VERBOSE? recv_code(control_sock) : print_reply(recv_code(control_sock));

	return 0;
}

int open_client_data_sock(int sockfd)
{
	int iport = CPORT;
	char *port = malloc(sizeof(char) * 33);
	sprintf(port, "%d", iport);

	int data_listen = open_listenfd(port);   //sock_type not supported

	int assure = 1;
	Send(sockfd, &assure, sizeof(assure), 0);  //inform server ready

	struct sockaddr_in sai;
	socklen_t len = sizeof(sai);
	int data_sock = Accept(data_listen, (SA *) &sai, &len);

	Close(data_listen);

	return data_sock;
}

int fclient_get(int control_sock, int data_sock, char *filename)
{
	char data[MAXLINE];
	size_t num_recvd;
	FILE *fp = fopen(filename, "w");

	VERBOSE? print_reply(recv_code(control_sock)) : recv_code(control_sock);

	while ((num_recvd = Recv(data_sock, data, MAXLINE, 0)) > 0) {
		fwrite(data, 1, num_recvd, fp);
	}

	VERBOSE? print_reply(recv_code(control_sock)) : recv_code(control_sock);

	fclose(fp);

	return 0;
}
