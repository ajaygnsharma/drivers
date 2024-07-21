/*
 * 08_mkpasswd.cpp
 *
 *  Created on: Mar 2, 2022
 *      Author: asharma
 */
#include <crypt.h>
#include <shadow.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <iostream>
#include <string>

using namespace std;

#define MAX_PW_SALT_LEN (3 + 16 + 1)

enum failure_codes{
	INCORRECT_ARGS    = -2,
	INSUFFICIENT_ARGS = -3,
	USER_NOT_FOUND    = -4,
	PERMISSION_DENIED = -5,

};

unsigned long long monotonic_us(void){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000ULL + tv.tv_usec;
}

static int i64c(int i){
	i &= 0x3f;
	if (i == 0)
		return '.';
	if (i == 1)
		return '/';
	if (i < 12)
		return ('0' - 2 + i);
	if (i < 38)
		return ('A' - 12 + i);
	return ('a' - 38 + i);
}

int crypt_make_salt(char *p, int cnt /*, int x */){
	/* was: x += ... */
	unsigned x = getpid() + monotonic_us();
	do {
		/* x = (x*1664525 + 1013904223) % 2^32 generator is lame
		 * (low-order bit is not "random", etc...),
		 * but for our purposes it is good enough */
		x = x*1664525 + 1013904223;
		/* BTW, Park and Miller's "minimal standard generator" is
		 * x = x*16807 % ((2^31)-1)
		 * It has no problem with visibly alternating lowest bit
		 * but is also weak in cryptographic sense + needs div,
		 * which needs more code (and slower) on many CPUs */
		*p++ = i64c(x >> 16);
		*p++ = i64c(x >> 22);
	} while (--cnt);
	*p = '\0';
	return x;
}

char *crypt_make_pw_salt(char salt[MAX_PW_SALT_LEN], const char *algo){
	int len = 2/2;
	char *salt_ptr = salt;

	/* Standard chpasswd uses uppercase algos ("MD5", not "md5").
	 * Need to be case-insensitive in the code below.
	 */
	if ((algo[0]|0x20) != 'd') { /* not des */
		len = 8/2; /* so far assuming md5 */
		*salt_ptr++ = '$';
		*salt_ptr++ = '1';
		*salt_ptr++ = '$';
	}
	crypt_make_salt(salt_ptr, len);
	return salt_ptr;
}

char *pw_encrypt(const char *clear, const char *salt, int cleanup){
	char *s;

	s = crypt(clear, salt);
	/*
	 * glibc used to return "" on malformed salts (for example, ""),
	 * but since 2.17 it returns NULL.
	 */
	return strdup(s ? s : "");
}

char *get_shadow_password(const char *username){
	struct spwd *spwd;
	spwd = getspnam(username);
	if (spwd == nullptr && errno == EACCES){
		exit(PERMISSION_DENIED);
	}else if(spwd == nullptr){
		exit(USER_NOT_FOUND);
	}else{
		return spwd->sp_pwdp;
	}
}


int main(int argc, char *argv[]){
	char salt[MAX_PW_SALT_LEN + sizeof("rounds=999999999$")];
	const char *opt_m = "md5";
	char *salt_ptr;
	//const char *shadow_pass = "mCMZttBM$.WEWUSuxyaXmqKmaJ7Di40";
	char *shadow_pass = nullptr;
	string username, password;
	int opt;

	if(argc == 3){
		while((opt = getopt(argc, argv, "u:p:")) != -1){
			switch(opt){
			case 'u':
				username = optarg;
				break;

			case 'p':
				password = optarg;
				break;

			default:
				exit(INCORRECT_ARGS);
			}
		}

		salt_ptr = crypt_make_pw_salt(salt, opt_m);

		shadow_pass = get_shadow_password(username.c_str());
		printf("shadow_pass=%s\n",shadow_pass);

		if(shadow_pass != nullptr){
			strncpy(salt_ptr, shadow_pass, sizeof(salt) - (sizeof("$N$")-1));
			puts(pw_encrypt(password.c_str(), salt, 1));
		}
	}else{
		return (INSUFFICIENT_ARGS);
	}
  return 0;
}
