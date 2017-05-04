#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#define SERVER_PORT	9877
#define MAX_LINE	1024

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

int main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr, cliaddr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		err_quit(1, "socket error");
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family 		= AF_INET;
	servaddr.sin_addr.s_addr 	= htonl(INADDR_ANY);
	servaddr.sin_port 			= htons(SERVER_PORT);

	if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		err_quit(1, "socket bind error");
	}

	ssize_t		n;
	socklen_t	len;
	char		buf[MAX_LINE];
	char		tmp[MAX_LINE];

	for ( ; ; )
	{
		len = sizeof(cliaddr);
		n = recvfrom(sockfd, buf, MAX_LINE, 0, (struct sockaddr *)&cliaddr, &len); 
		if (n < 0)
		{
			err_quit(1, "socket recvfrom error");
		}
// todo: 没有循环时可以打印一次，有 from 循环时不打印？
//		printf("recvform : %s, %d",
//			inet_ntop(AF_INET, &cliaddr.sin_addr, tmp, sizeof(tmp)),
//			ntohs(cliaddr.sin_port));

		if (sendto(sockfd, buf, n, 0, (struct sockaddr *)&cliaddr, len) != n)
		{
			err_quit(1, "socket sendto error");
		}
	}

	return 0;
}
