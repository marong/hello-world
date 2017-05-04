
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
	if (argc != 2)
	{
		err_quit(0, "usage: udpcli <Ipaddress>");
	}

	int					sockfd;
	struct sockaddr_in	servaddr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		err_quit(1, "socket error");
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family 		= AF_INET;
	servaddr.sin_addr.s_addr 	= htonl(INADDR_ANY);
	servaddr.sin_port 			= htons(SERVER_PORT);

	ssize_t		n;
	char		sendline[MAX_LINE], recvline[MAX_LINE + 1];
	while (fgets(sendline, MAX_LINE, stdin) != NULL)
	{
		n = strlen(sendline);
		if (sendto(sockfd, sendline, n, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) != n)
		{
			err_quit(1, "socket sendto error");
		}

		n = recvfrom(sockfd, recvline, MAX_LINE, 0, NULL, NULL);
		if (n < 0)
		{
			err_quit(1, "socket recvform error");
		}
		recvline[n] = 0;
		fputs(recvline, stdout);
	}

	return 0;
}
