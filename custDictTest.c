#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(void) 
{
	int fd;
	ssize_t count;
	char buf[50];
;
	/* open device */
	if ((fd = open("/proc/customdictwrite", O_RDWR)) < 0) {
		perror("open()");
		return 1;
	}

	/* write to device */
	memset(buf, 0x00, sizeof(buf));   /* clear buffer */
	strcpy(buf, "EYES=BLUE");

	count = write(fd, buf, sizeof(buf));
	
	memset(buf, 0x00, sizeof(buf));   /* clear buffer */
        strcpy(buf, "EYES");
	count = read(fd, buf, sizeof(buf));
	
	/* close device */
	close(fd);
	
	return 0;
}
