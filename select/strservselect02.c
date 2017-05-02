#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <arpa/inet.h>

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

int main(int argc, char **argv)
{
	int					i, maxi, maxfd, listenfd, connfd, sockfd;
	int					nready, client[FD_SETSIZE];
	ssize_t				n;
	fd_set				rset, allset;
	char				buf[MAX_LINE];
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0)
	{
		err_quit(1, "socket error");
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family 		= AF_INET;
	servaddr.sin_addr.s_addr 	= htonl(INADDR_ANY);
	servaddr.sin_port 			= htons(SERVER_PORT);

	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		err_quit(1, "socket bind error try sudo");
	}

	if (listen(listenfd, LISTEN_QUEUE) < 0)
	{
		err_quit(1, "socket listen error");
	}

	maxfd = listenfd;
	maxi = -1;
	for (i = 0; i < FD_SETSIZE; ++i)
	{
		client[i] = -1;
	}

	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	for ( ; ; )
	{
		rset = allset;
		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
		if (nready < 0)
		{
			err_quit(1, "select error for nready: %d", nready);
		}

		if (FD_ISSET(listenfd, &rset))
		{
			clilen = sizeof(cliaddr);
			connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
			if (connfd < 0)
			{
				err_quit(1, "socket accept error");
			}
			printf("new client: %s, port %d\n",
				inet_ntop(AF_INET, &cliaddr.sin_addr, buf, sizeof(buf)),
				ntohs(cliaddr.sin_port));

			for (i = 0; i < FD_SETSIZE; ++i)
			{
				if (client[i] < 0)
				{
					client[i] = connfd;	// save descriptor
					break;
				}
			}
			if (i == FD_SETSIZE)
			{
				err_quit(1, "too many clients");
			}
			FD_SET(connfd, &allset);	// add new descriptor to set
			if (connfd > maxfd)
			{
				maxfd = connfd;	// for select
			}
			if (i > maxi)
			{
				maxi = i;
			}
			if (--nready <= 0)
			{
				continue;
			}
		}
		
		for (i = 0; i <= maxi; ++i)
		{
			if ((sockfd = client[i]) < 0)
			{
				continue;
			}

			if (FD_ISSET(sockfd, &rset))
			{
				if ((n = read(sockfd, buf, MAX_LINE)) == 0)
				{
					// connection closed by client
					if (close(sockfd) < 0)
					{
						err_quit(1, "socket close error");
					}
					FD_CLR(sockfd, &allset);
					client[i] = -1;
				}
				else if (n < 0 && errno != EINTR)
				{
					err_quit(1, "socket read error");
				}
				else if (n > 0)
				{
					if (write(sockfd, buf, n) != n)
					{
						err_quit(1, "socket write error");
					}
				}

				// no more readable descriptors
				if (--nready <= 0)
				{
					break;
				}
			}

		}
	}
	exit(0);
}

