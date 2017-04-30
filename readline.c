#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

#define MAX_LINE 4096

static int read_cnt;
static char *read_ptr;
static read_buf[MAX_LINE];

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
		read_ptr = (char *)read_buf;
	}

	read_cnt--;
	*ptr = *read_ptr++;
	return 1;
}

ssize_t readline(int fd, void *vptr, size_t maxlen)
{
	ssize_t n, rc;
	char c, *ptr;

	ptr = vptr;
	for (n = 1; n < maxlen; ++n)
	{
		if((rc = my_read(fd, &c)) == 1)
		{
			*ptr++ = c;
			if (c == '\n')
			{
				break;
			}
		}
		else if (rc == 0)
		{
			*ptr = 0;
			return n - 1;
		}
		else
		{
			return -1;
		}
	}

	*ptr = 0;
	return n;
}

ssize_t readlinebuf(void **vptrptr)
{
	if (read_cnt)
	{
		*vptrptr = read_ptr;
	}
	return read_cnt;
}

ssize_t Readline(int fd, void *ptr, size_t maxlen)
{
	ssize_t n = readline(fd, ptr, maxlen);
	if (n < 0)
	{
		printf("readline error");
	}
	return n;
}

