#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KEY_PATH "/sys/bus/i2c/devices/0-0060/input0"

int main(int argc, char **argv)
{
#define BUF_SIZE 4
	FILE* fd;
	unsigned char data = 0;
	unsigned char continueLooping = 1;
	unsigned char buf[BUF_SIZE];
	int len = 0;

	printf("%s: quit by pressing key #1 and #4 simultaneously.\n", argv[0]);

	while(continueLooping == 1)
	{

		fd = fopen(KEY_PATH, "r");
		if (fd == NULL) {
			perror("Failed to open " KEY_PATH);
			return 1;
		}

		len = fread(&buf, 1, 3, fd);
		if (len < 0 || len > BUF_SIZE)
			break;
		buf[len] = '\0';

		data = (unsigned char)strtol(buf, NULL, 0);

		printf("\nPressed keys (0x%x): ", data&0x0f);
		if ((data & 0x01) == 0)
			printf("1 ");
		else
			printf("  ");
		if ((data & 0x02) == 0)
			printf("- 2 ");
		else
			printf("-   ");
		if ((data & 0x04) == 0)
			printf("- 3 ");
		else
			printf("-   ");
		if ((data & 0x08) == 0)
			printf("- 4");


		if ((data & 0x09) == 0)
			continueLooping = 0;
		else
			usleep(300000);

		fclose(fd);
	}

	return 0;
}

