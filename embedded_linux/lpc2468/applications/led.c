#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LED_PATH "/sys/bus/i2c/devices/0-0060/ls2"

int main(int argc, char **argv)
{
#define BUF_SIZE 4
	FILE* fd;
	int led = 0;
	int on = 0;
	int len = 0;
	unsigned char buf[BUF_SIZE];
	unsigned char data = 0;

	if(argc != 3)
	{
		printf("%s: missing arguments\n", argv[0]);
		printf("  > %s <num> on|off\n", argv[0]);
		return 1;
	}

	led = (int)strtol(argv[1], NULL, 0);
	if(strcmp(argv[2], "on") == 0)
	{
		on = 1;
	}

	fd = fopen(LED_PATH, "r+b");
	if (fd == NULL) {
		perror("Failed to open " LED_PATH);
		return 1;
	}
	
	len = fread(&buf, 1, 3, fd);
	if (len > 0 && len < BUF_SIZE)
	{
		buf[len] = '\0';
		data = (unsigned char)strtol(buf, NULL, 0);

		switch(led)
		{
		case 1:
			data &= ~0x03;
			if(on == 1)
				data |= 0x01;
   
			break;
		case 2:
			data &= ~0x0c;
			if(on == 1)
				data |= 0x04;

			break;
		case 3:
			data &= ~0x30;
			if(on == 1)
				data |= 0x10;

			break;
		case 4:
			data &= ~0xc0;
			if(on == 1)
				data |= 0x40;

			break;
		}

		sprintf(buf, "%d", data);

		fwrite(buf, 1, strlen(buf), fd);
	}

	fclose(fd);

	return 0;
}

