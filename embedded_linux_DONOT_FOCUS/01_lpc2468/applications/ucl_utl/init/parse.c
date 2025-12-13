
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/termios.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <linux/version.h>
#include <utmp.h>
#include <errno.h>
#include <time.h>
#include <termios.h>
#ifdef SHADOW_PWD
#include <shadow.h>
#endif

#include "pathnames.h"

#define BUF_SIZ 100	/* max size of a line in inittab */
#define NUMCMD 30	/* step size when realloc more space for the commands */
#define NUMTOK 20	/* max number of tokens in inittab command */


struct initline {
	char	**toks;
	char	*line;
	char	*fullline;
	unsigned char xcnt;
};

struct initline *inittab;

/* How many struct initline's will fit in the memory pointed to by inittab */
int inittab_size = 0; 
int numcmd;

static int  read_initfile(const char *);

static void init_itab(struct initline *p) {
	bzero(p, sizeof(struct initline));
}

static void clear_itab(struct initline *p) {
	if (p->line)
		free(p->line);
	if (p->fullline)
		free(p->fullline);
	if (p->toks)
		free(p->toks);
	init_itab(p);
}

static int
read_initfile(const char *initfile)
{
	struct initline *p;
	FILE *f;
	char *buf = NULL;
	size_t buf_len = 0;
	int i,j,k;
	char *ptr, *getty;
#ifdef SPECIAL_CONSOLE_TERM
	char tty[50];
	struct stat stb;
#endif
			
	i = numcmd;

	if (!(f = fopen(initfile, "r")))
		return 1;

	while(!feof(f)) {
		if (i+2 == inittab_size) {
			/* need to realloc inittab */
			inittab_size += NUMCMD;
			inittab = realloc(inittab, inittab_size * sizeof(struct initline));
			if (!inittab) {
				/* failure case - what do you do if init fails? */
				err("malloc failed");
				_exit(1);
			}
		}
		if (getline(&buf, &buf_len, f) == -1) break;

		for(k = 0; k < buf_len && buf[k]; k++) {
			if(buf[k] == '#') { 
				buf[k] = '\0'; break; 
			}
		}

		if(buf[0] == '\0' || buf[0] == '\n') continue;

		p = inittab + i;
		init_itab(p);
		p->line = strdup(buf);
		p->fullline = strdup(buf);
		if (!p->line || !p->fullline) {
			err("Not memory to allocate inittab entry");
			clear_itab(p);
			continue;
		}
		ptr = strtok(p->line, ":");
		if (!ptr) {
			err("Missing TTY/ID field in inittab");
			clear_itab(p);
			continue;
		}
		strncpy(p->tty, ptr, 9);
		//p->tty[9] = '\0';
		ptr = strtok(NULL, ":");
		if (!ptr) {
			err("Missing TERMTYPE field in inittab");
			clear_itab(p);
			continue;
		}
		strncpy(p->termcap, ptr, 29);
		//p->termcap[29] = '\0';

		getty = strtok(NULL, " \t\n");
		if (!getty) {
			err("Missing PROCESS field in inittab");
			clear_itab(p);
			continue;
		}
		add_tok(p, getty);
		j = 1;
		while((ptr = strtok(NULL, " \t\n")))
			add_tok(p, ptr);

#ifdef SPECIAL_CONSOLE_TERM
		/* special-case termcap for the console ttys */
		strcpy(tty, "/dev/");
		strcat(tty, p->tty);
		if(!termenv || stat(tty, &stb) < 0) {
			err("no TERM or cannot stat tty\n");
		} else {
			/* is it a console tty? */
			if(major(stb.st_rdev) == 4 && minor(stb.st_rdev) < 64) {
				strncpy(p->termcap, termenv, 30);
				p->termcap[29] = 0;
			}
		}
#endif

		i++;
	}

	if (buf)
		free(buf);
	
	fclose(f);

	numcmd = i;
	return 0;
}


int main(int argc, char *argv[]){
	int 	i;
	
	/* initialize the array of commands */
	inittab = (struct initline *)malloc(NUMCMD * sizeof(struct initline));
	inittab_size = NUMCMD;

	if (!inittab) {
		/* failure case - what do you do if init fails? */
		err("malloc failed");
		_exit(1);
	}

	read_inittab();
	
	if (read_initfile(_PATH_INITTAB) == 0)
		i++;
}
