NANOX_LIB=/home/user1/uClinux-dist/user/microwin/src/lib
NANOX_INC=/home/user1/uClinux-dist/user/microwin/src/include

CFLAGS= -Wall -W -L$(NANOX_LIB) -I$(NANOX_INC) -DMWINCLUDECOLORS
LDFLAGS=-Wl,-elf2flt -lnano-X 

CC=/usr/local/bin/arm-elf-gcc
RM=rm -f

PROG=ipcsrv
SRC=ipcserver.c

OBJ=$(SRC:%.c=%.o)

$(PROG): $(OBJ)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJ) $(LDFLAGS) 

.PHONY: clean all dep

clean:
	$(RM) $(PROG) $(OBJ) *~ *.gdb .depend *.elf2flt *.elf
 
