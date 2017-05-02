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
#include <poll.h>

#define LISTEN_QUEUE 1024
#define MAX_LINE 4096
#define SERVER_PORT 9877
#define OPEN_MAX 1024

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
	int					i, maxi, listenfd, connfd, sockfd;
	int					nready;
	ssize_t				n;
	char				buf[MAX_LINE];
	socklen_t			clilen;
	struct pollfd		client[OPEN_MAX];
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

	client[0].fd = listenfd;
	client[0].events = POLLRDNORM;
	for (i = 1; i < OPEN_MAX; ++i)
	{
		client[i].fd = -1;
	}
	maxi = 0;

	for ( ; ; )
	{
		nready = poll(client, maxi + 1, -1);
		if (nready < 0)
		{
			err_quit(1, "poll error for nready: %d", nready);
		}

		if (client[0].revents & POLLRDNORM)	// new client connection
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

			for (i = 1; i < OPEN_MAX; ++i)
			{
				if (client[i].fd < 0)
				{
					client[i].fd = connfd;	// save descriptor
					break;
				}
			}
			if (i == OPEN_MAX)
			{
				err_quit(0, "too many clients");
			}

			client[i].events = POLLRDNORM;
			if (i > maxi)
			{
				maxi = i;
			}

			if (--nready <= 0)
			{
				continue;
			}
		}

		for (i = 1; i <= maxi; ++i)
		{
			if ((sockfd = client[i].fd) < 0)
			{
				continue;
			}

			if (client[i].revents & (POLLRDNORM | POLLERR))
			{
				if ((n = read(sockfd, buf, MAX_LINE)) < 0)
				{
					if (errno == ECONNRESET)
					{
						// connection reset by client
						printf("client[%d] aborted connection\n", i);
						if (close(sockfd) < 0)
						{
							err_quit(1, "socket close error");
						}
						client[i].fd = -1;
					}
					else
					{
						err_quit(1, "socket read error");
					}
				}
				if (n == 0)
				{
					// connection closed by client
					printf("client[%d] close connection\n", i);
					if (close(sockfd) < 0)
					{
						err_quit(1, "socket close error");
					}
					client[i].fd = -1;
				}
				else
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

