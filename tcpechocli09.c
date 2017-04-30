#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "sum.h"

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

void str_client(FILE *fp, int sockfd)
{
	char sendline[MAX_LINE];
	struct Args args;
	struct Result result;

	while (fgets(sendline, MAX_LINE, fp) != NULL)
	{
		if (sscanf(sendline, "%ld%ld", &args.arg1, &args.arg2) != 2)
		{
			printf("invalid input: %s", sendline);
		}

		if (write(sockfd, &args, sizeof(args)) != sizeof(args))
		{
			err_quit(1, "wirte error");
		}

		if (Readn(sockfd, &result, sizeof(result)) == 0)
		{
			err_quit(0, "str_client: server terminate prematurely");
		}

		printf("%ld\n", result.sum);
	}
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		err_quit(0, "usage: a.out <IPaddress>");
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		err_quit(1, "socket error");
	}

	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERVER_PORT);
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
	{
		err_quit(1, "inet_pton error for %s", argv[1]);
	}

	if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		err_quit(1, "connect error");
	}

	str_client(stdin, sockfd);

	exit(0);
}
