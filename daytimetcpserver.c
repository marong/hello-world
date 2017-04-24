#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>

#define LISTEN_QUEUE 1024	// 最大连接数
#define MAX_LINE 4096

void err_quit(int errnoflag, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	printf(fmt, ap);
	va_end(ap);
	if(errnoflag)
	{
		printf("\nerrno: %d, %s \n", errno, strerror(errno));
	}
	else
	{
		printf("\n");
	}
	printf("\n");
	exit(1);
}

int main(int argc, char **argv)
{
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd < 0)
	{
		err_quit(0, "socket error");
	}

	struct sockaddr_in servaddr, cliaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port= htons(13);

	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		err_quit(1, "socket bind error, try sudo");
	}

	if(listen(listenfd, LISTEN_QUEUE) < 0)
	{
		err_quit(1, "socket listen error");
	}

	int connfd = 0;
	time_t ticks;
	char buff[MAX_LINE];
	socklen_t len;
	for( ; ; )
	{
		// connfd = accept(listenfd, NULL, NULL);  // 不关心客户端地址
		len = sizeof(cliaddr);
		connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len);
		if(connfd < 0)
		{
			err_quit(1, "socket accept error");
		}

		printf("connection from %s, port %u\n",
			inet_ntop(AF_INET, &cliaddr.sin_addr, buff, sizeof(buff)),
			ntohs(cliaddr.sin_port));

		ticks = time(NULL);
		snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
		printf("%s", buff);

		if(write(connfd, buff, len) != strlen(buff))
		{
			err_quit(1, "write error");
		}

		if(close(connfd) == -1)
		{
			err_quit(1, "socket close error");
		}
	}
}
