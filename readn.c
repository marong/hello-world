#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>

// Read "n" bytes from a descriptor.
ssize_t readn(int fd, void *vptr, size_t n)
{
	size_t nleft;
	ssize_t nread;
	char *ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0)
	{
		if((nread = read(fd, ptr, nleft)) < 0)
		{
			if (errno == EINTR)
			{
				nread = 0;
			}
			else
			{
				return -1;
			}
		}
		else if (nread == 0)
		{
			break;	// EOF
		}

		nleft -= nread;
		ptr += nread;
	}
	return n - nleft;
}

ssize_t Readn(int fd, void *ptr, size_t nbytes)
{
	ssize_t n;
	if ((n = readn(fd, ptr, nbytes)) < 0)
	{
		printf("readn error");
	}
	return n;
}

