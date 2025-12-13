/*
    This application illustrates the use of the i2c driver to read from 
    and write to the eeprom.
    Copyright (C) 2007  Embedded Artists AB (www.embeddedartists.com)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define BUFFER_SIZE 1024

#define PRINT_TYPE_HEX    0
#define PRINT_TYPE_DEC    1
#define PRINT_TYPE_STRING 2

#define PRINT_WIDTH_8  1
#define PRINT_WIDTH_16 2
#define PRINT_WIDTH_32 4

#define EEPROM_PATH "/sys/bus/i2c/devices/0-0050/data0"

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y)) 
#endif

static char chToUpper(char ch)
{
  if((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'))
  {
    return ch;
  }
  else
  {
    return ch - ('a'-'A');
  }   
}

static int strToInt(const char* pStr, int len, int base)
{
  //                      0,1,2,3,4,5,6,7,8,9,:,;,<,=,>,?,@,A ,B ,C ,D ,E ,F
  static const int v[] = {0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,0,10,11,12,13,14,15}; 
  int i   = 0;
  int val = 0;
  int dec = 1;
  int idx = 0; 

  for(i = len; i > 0; i--)
  {
    idx = chToUpper(pStr[i-1]) - '0';    

    if(idx > sizeof(v)/sizeof(int))
    {
      printf("strToInt: illegal character %c\n", pStr[i-1]);
      continue;
    }
    
    val += (v[idx]) * dec;
    dec *= base;
  }

  return val; 
}

static void printData(char* pBuf, int len, int width, int type)
{
	int i = 0;
	int newline = 0;

	for(i = 0; i < len; i++)
	{
		newline = (i > 0 && (i%8) == 0);

		switch(type)
		{
		case PRINT_TYPE_HEX:
			
			if(newline)
				printf("\n");
			else if(i > 0)
				printf(":");

			switch(width)
			{
			case PRINT_WIDTH_8:
				printf("%.2x", pBuf[i]);

				break;
			case PRINT_WIDTH_16:
				printf("%.4x", *((unsigned short*)&pBuf[i]));
				i++;
				break;
			case PRINT_WIDTH_32:
				printf("%.8x", *((unsigned int*)&pBuf[i]));
				i+=3;
				break;
			}

			break;
		case PRINT_TYPE_DEC:

			if(newline)
				printf("\n");
			else if(i > 0)
				printf(":");

			switch(width)
			{
			case PRINT_WIDTH_8:
				printf("%3d", pBuf[i]);

				break;
			case PRINT_WIDTH_16:
				printf("%5d", *((short*)&pBuf[i]));
				i++;
				break;
			case PRINT_WIDTH_32:
				printf("%10d", *((int*)&pBuf[i]));
				i+=3;
				break;
			}

			break;
		case PRINT_TYPE_STRING:
			printf("%c", pBuf[i]);
		}
	}
	printf("\n");
	
}

static void intToBuf(unsigned int value, char* pBuf, int width)
{
	switch(width)
	{
	case PRINT_WIDTH_8:
		pBuf[0] = (char)(value & 0xFF);
		break;
	case PRINT_WIDTH_16:
		pBuf[0] = (char)(value & 0xFF);
		pBuf[1] = (char)((value >> 8) & 0xFF);
		break;
	case PRINT_WIDTH_32:
		pBuf[0] = (char)(value & 0xFF);
		pBuf[1] = (char)((value >> 8) & 0xFF);
		pBuf[2] = (char)((value >> 16) & 0xFF);
		pBuf[3] = (char)((value >> 24) & 0xFF);
		break;
	}
}

static int getData(char* pInput, char* pBuf, int bufSize, int width)
{
	int len = 0;
	int base = 16;
	char format = 'h';
	char* pNext = NULL;
	unsigned int value = 0;

	// must have room for at least one value
	if(bufSize < width)
		return bufSize;

	format = *pInput;

	// multiple input
	if(format == 'h' || format == 'd' || format == 's')
	{
		pInput+=2;

		// a string is written to eeprom
		if(format == 's')
		{			
			len = strlen(pInput);	

			if(len > bufSize)
				return len;

			memcpy(pBuf, pInput, len);

			return len;
		}		

		if(format == 'd')
			base = 10;
		
		pNext = strchr(pInput, ':');
		while(pNext != NULL)
		{			
			// there is always room for at least one value
			
			value = strToInt(pInput, pNext-pInput, base);
			intToBuf(value, pBuf+len, width);
			len += width;			

			if(len+width >= bufSize)
				return len+width;

			pInput = pNext+1;
			pNext = strchr(pInput, ':');
		}

		if(strlen(pInput) > 0)
		{
			value = strToInt(pInput, strlen(pInput), base);
			
			intToBuf(value, pBuf+len, width);
			len += width;						
		}

				
	}

	// single value
	else
	{
		if(strlen(pInput) >2 && strncasecmp("0x", pInput, 2) == 0)
			value = strToInt(pInput+2, strlen(pInput)-2, 16);
		else
			value = strToInt(pInput, strlen(pInput), 10);

		len = width;
		intToBuf(value, pBuf, width);
		
	}

	return len;
}

static void help(char* prg_name)
{
	printf("Usage: %s rX addr count [format] OR\n", prg_name);
	printf("       %s wX addr data\n", prg_name);

	printf("\n\tX\t8, 16 or 32 (size of each element read or written)\n");
	printf("\taddr\teeprom address (hexadecimal or decimal value)\n");
	printf("\tcount\tnumber of elements to read\n");
	printf("\tformat\thex\t- hexadecimal format, e.g. 1a:20:ea (DEFAULT)\n");
	printf("\t\tdec\t- decimal format, e.g. 26:32:234\n");
	printf("\t\tstring\t- read data will be printed as an ASCII string\n");

	printf("\n\tdata\tdata to be written\n");
	printf("\t\thexadecimal or decimal value, e.g., 0x1234 or 4660\n");
	printf("\t\th:100:200:300\t3 hexadecimal values\n");
	printf("\t\td:256:512:768\t3 decimal values\n");
	printf("\t\t's:hello there'\ta string\n");

	printf("\nExamples:\n\n");
	printf("\tr8  1024  10\t ten bytes are read from address 1024\n");
	printf("\tr32 0x400 2 dec\t two 32-bit values are read from address 0x400\n");
	printf("\t\t\t and printed as decimal values\n");
	printf("\tw8 0x400 0x20\t the value 0x20 is written to address 0x400\n");
	printf("\tw16 0 h:1a20:34f:a120\t three 16-bit hexadecimal values are\n");
	printf("\t\t\t\t written to address 0\n");
	printf("\tw8 0x1000 's:hi there'\t a string is written to address 0x1000\n");


}

int main(int argc, char **argv)
{
	FILE* fd;
	int read  = 0;
	int len   = 8;
	int addr  = 0;
	int type  = PRINT_TYPE_HEX;
	int width = PRINT_WIDTH_8;
	int size = 0;

	char buf[BUFFER_SIZE];

	if(argc < 4)
	{
		help(argv[0]);
		return 1;
	}

	if(*argv[1] != 'r' && *argv[1] != 'w')
	{
		help(argv[0]);
		return 1;
	}

	// read or write operation
	read = (*argv[1] == 'r');


	// 8, 16, or 32 bit operations
	if(strcmp("8", argv[1]+1) == 0)
		width = PRINT_WIDTH_8;
	else if(strcmp("16", argv[1]+1) == 0)
		width = PRINT_WIDTH_16;
	else if(strcmp("32", argv[1]+1) == 0)
		width = PRINT_WIDTH_32;

	// address
	if(strncasecmp("0x", argv[2], 2) == 0)
	{
		addr = strToInt(argv[2]+2, strlen(argv[2])-2, 16);	
	}
	else
	{
		addr = strToInt(argv[2], strlen(argv[2]), 10);	
	}


	// length
	if (read)
	{
		len = strToInt(argv[3], strlen(argv[3]), 10);
		len *= width; // length should be in bytes
	}
	else
	{
		len = getData(argv[3], buf, BUFFER_SIZE, width);
	}

	// format
	if(read && argc >= 5)
	{
		if(strcmp("hex", argv[4]) == 0)
			type = PRINT_TYPE_HEX;
		else if(strcmp("dec", argv[4]) == 0)
			type = PRINT_TYPE_DEC;
		else if(strcmp("string", argv[4]) == 0)
			type = PRINT_TYPE_STRING;

	}				

	if (len > BUFFER_SIZE)
	{
		printf("ERROR: Maximum buffer size is %d\n", BUFFER_SIZE);
		return 1;
	}

	fd = fopen(EEPROM_PATH, "r+b");
	if (fd == NULL) {
		perror("Failed to open " EEPROM_PATH);
		return 1;
	}

	fseek (fd, 0, SEEK_END);
    	size=ftell (fd);
	fseek (fd, 0, SEEK_SET);

	if (addr+len >= size)
	{
		printf("ERROR: addr + len > %d bytes\n", size);
		return 1;
	}  

	if(read)
	{

		// address to read from
		fseek(fd, addr, SEEK_SET);

		// read data
		fread(buf, 1, len, fd);

		printData(buf, len, width, type);

	}
	else
	{

		// address to write to
		fseek(fd, addr, SEEK_SET);

		// write data
		fwrite(buf, 1, len, fd);
	}

 	fclose(fd);

	return 0;
}

