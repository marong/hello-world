#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define LISTEN_QUEUE 1024
#define MAX_LINE 4096
#define SERVER_PORT 9877

void err_quit(int errnoflag, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	printf(fmt, ap);
	va_end(ap);

	if (errnoflag)
	{
		printf("\nerrno: %d, %s", errno, strerror(errno));
	}
	printf("\n");
	exit(1);
}

void sig_child(int signo)
{
	pid_t pid;
	int stat;

	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
	{
		printf("child %d terminated\n", pid);
	}
	return;
}

void str_echo(int sockfd)
{
	ssize_t n;
	char buf[MAX_LINE];

	while((n = read(sockfd, buf, MAX_LINE)) != 0)
	{
		if (n > 0)
		{
			if (write(sockfd, buf, n) != n)
			{
				err_quit(1, "write error");
			}
		}
		else if (errno != EINTR)
		{
			printf("str_echo: read error\n");
			break;
		}
	}
}

int main(int argc, char **argv)
{
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0)
	{
		err_quit(1, "socket error");
	}

	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERVER_PORT);

	if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		err_quit(1, "socket bind error, try sudo\n");
	}

	if (listen(listenfd, LISTEN_QUEUE) < 0)
	{
		err_quit(1, "socket listen error\n");
	}

	signal(SIGCHLD, sig_child);	// must call waitpid()

	struct sockaddr_in cliaddr;
	pid_t childpid;
	socklen_t clilen;
	int connfd;
	char buff[MAX_LINE];
	for ( ; ; )
	{
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
		if (connfd < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			err_quit(1, "socket accept error\n");
		}

		printf("connection from %s, port %d\n",
			inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)),
			ntohs(cliaddr.sin_port));

		childpid = fork();
		if (childpid == 0)
		{
			if (close(listenfd) == -1)
			{
				err_quit(1, "listen socket close error");
			}
			str_echo(connfd);
			exit(0);
		}

		if (close(connfd) == -1)
		{
			err_quit(1, "connect socket close error");
		}
	}
}

