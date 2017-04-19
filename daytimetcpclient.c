#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>  // sockaddr_in{} and other Internet defines
#include <string.h> // bzero
#include <stdlib.h> // exit(0)
#include <stdarg.h> // 可变参数
#include <errno.h>

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
	exit(1);
}

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		err_quit(0, "usage: a.out <IPaddress>");
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		err_quit(1, "socket error");
	}

	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(13); // daytime server
	if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
	{
		err_quit(1, "inet_pton error for %s", argv[1]);
	}

	if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		err_quit(1, "connect error");
	}

	int n = 0;
	char recvline[MAX_LINE + 1];
	while((n = read(sockfd, recvline, MAX_LINE)) > 0)
	{
		recvline[n] = 0;
		if(fputs(recvline, stdout) == EOF)
		{
			err_quit(1, "fputs error");
		}
	}

	if(n < 0)
	{
		err_quit(0, "read error");
	}

	exit(0);
}

