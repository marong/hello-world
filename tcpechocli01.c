#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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

static int read_cnt = 0;
static char *read_ptr = NULL;
static char read_buf[MAX_LINE];

/*****************************************************************************
	从 socket fd 读取数据，并将数据缓存下来
	当缓存中没有数据时，从 socket 中读取数据到缓存
	当缓存中有数据时不需要从 socket 读取数据
	每次调用都将一个字符写入 ptr 所指向的 char 变量中
	返回 -1 表示出错，返回 0 表示所有数据已读完， 返回 1 表示成功取得一个字符
******************************************************************************/
static ssize_t my_read(int fd, char *ptr)
{
	if (read_cnt <= 0)
	{
again:
		if ((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0)
		{
			if (errno == EINTR)
			{
				goto again;
			}
			return -1;
		}
		else if (read_cnt == 0)
		{
			return 0;
		}
		read_ptr = read_buf;
	}

	read_cnt--;
	*ptr = *read_ptr++;
	return 1;
}

ssize_t readline(int fd, void *vptr, size_t maxlen)
{
	ssize_t n, rc;
	char    c, *ptr;
	ptr = vptr;

	for (n = 1; n < maxlen; n++)
	{
		if ((rc = my_read(fd, &c)) == 1)
		{
			*ptr++ = c;
			if (c == '\n')
			{
				break;	// newline is stored, like fgets()
			}
		}
		else if (rc == 0)
		{
			*ptr = 0;
			return (n - 1);	// EOF, n - 1 bytes were read
		}
		else
		{
			return -1;	// error, errno set by read()
		}
	}

	*ptr = 0;	// null terminate like fgets()
	return n;
}

void str_client(FILE *fp, int sockfd)
{
	char sendline[MAX_LINE];
	char recvline[MAX_LINE];

	while (fgets(sendline, MAX_LINE, fp) != NULL)
	{
		if (write(sockfd, sendline, strlen(sendline)) != strlen(sendline))
		{
			err_quit(1, "wirte error");
		}

		if (readline(sockfd, recvline, MAX_LINE) == 0)
		{
			err_quit(0, "str_client: server terminate prematurely");
		}

		fputs(recvline, stdout);
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
