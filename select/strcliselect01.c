#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <stdarg.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>

#define MAX_LINE 4096
#define SERVER_PORT 9877
#define max(a,b)	((a) > (b) ? (a) : (b))

void err_quit(int errnoflag, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	printf(fmt, ap);
	va_end(ap);

	if(errnoflag)
	{
		printf("\nerrno: %d, %s", errno, strerror(errno));
	}
	printf("\n");
	exit(1);
}

void str_cli(FILE *fp, int sockfd)
{
	char sendline[MAX_LINE];
	char recvline[MAX_LINE];
	int nfds;
	fd_set readfds;
	int ready, n;

	FD_ZERO(&readfds);

	for ( ; ; )
	{
		FD_SET(fileno(fp), &readfds);
		FD_SET(sockfd, &readfds);
		nfds = max(fileno(fp), sockfd) + 1;
		ready = select(nfds, &readfds, NULL, NULL, NULL);
		if (ready < 0)
		{
			printf("select error for ready: %d", ready);
		}

		if (FD_ISSET(sockfd, &readfds))
		{
			if ((n = read(sockfd, recvline, MAX_LINE)) == 0)
			{
				err_quit(0, "str_cli: server terminated prematurely");
			}
			else if (n < 0 && errno != EINTR)
			{
				err_quit(1, "str_cli: read error");
			}
			fputs(recvline, stdout);
		}

		if (FD_ISSET(fileno(fp), &readfds))
		{
			if (fgets(sendline, MAX_LINE, fp) == NULL)
			{
				return;
			}
			
			if (write(sockfd, sendline, strlen(sendline)) != strlen(sendline))
			{
				err_quit(1, "sock write error");
			}
		}
	}
}

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		err_quit(0, "usage: a.out <IPaddress>");
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		err_quit(1, "socket error");
	}

	struct sockaddr_in  servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERVER_PORT);
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
	{
		err_quit(1, "inet_pton error for %s", argv[1]);
	}

	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		err_quit(1, "connect error");
	}

	str_cli(stdin, sockfd);

	if (close(sockfd) < 0)
	{
		err_quit(1, "close error");
	}

	exit(0);
}



