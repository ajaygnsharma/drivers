/*
 ============================================================================
 Name        : 101_cfg_read_getline.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

char *find_key(char *buf, char *key){
  char *ptr = NULL;

  ptr = strstr(buf,key);

  if(ptr != NULL){
    char *p = strpbrk(ptr,"=");
    p++;
    return p;
  }else{
    return NULL;
  }
}

struct cfg_s {
  int val1;
  char str[50];
  int val2;
}cfg;

class Cfg_err{
	vector <string> msgs;

public:
	Cfg_err(){}
	void add(string msg);
	bool any(void);
	void log(void);
};



Cfg_err cfg_err = Cfg_err();


void Cfg_err::add(string msg){
	msgs.push_back(msg);
}

bool Cfg_err::any(void){
	return msgs.size();
}

void Cfg_err::log(void){
	for(vector<string>::iterator it = msgs.begin(); it != msgs.end(); ++it){
		cerr << *it;
		//ibuc_log()
	}
}


bool find_str(vector <string> tokens, string to_find_str, string &dest){
	string var;
	size_t found = -1;
	for(vector<string>::iterator it = tokens.begin(); it != tokens.end(); ++it){
		string s = *it;
		found = s.find(to_find_str);
		if(found != string::npos){ /* Found it! */
			stringstream ss(s);
			string token1;
			vector <string> v1;
			while(getline(ss, token1, '=')){
				v1.push_back(token1);
			}
			dest = v1[1];
			break; //Found, so exit.
		}
	}

	if(found != -1){ //Found can be 0 if found at the first location of string
		return true;
	}else{
		string s = to_find_str + " not found\n";
		cfg_err.add(s);
		dest.clear(); //Make it clear for error proofing.
		return false;
	}
}

void create_data(void){
  FILE *pFile;
  char *buf = NULL;
  char *f_buf = NULL;
  char *to = NULL;
  ssize_t n = 0;
  struct cfg_s cfg = {10, "my_security", 20};

  char *ptr = "mysecurity";
  pFile = fopen("test.txt", "w");

  n = asprintf(&buf, "TEMPERATURE_MAX=%d\n"
                     "SNMP_C2GROUP_SECURITY=%s\n"
                     "BURST_THOLD=%d\n",
                     cfg.val1, cfg.str, cfg.val2 );
  //cfg.val2 = 50;

  fwrite(buf , n, 1, pFile);
  //fprintf(pFile, "SNMP_SEC2GROUP_SECURITY=mysecurity\n");
  //fputs(buffer,pFile);
  fclose(pFile);
  return;
}

int
main(int argc, char *argv[])
{
  /* First load defaults */
  create_data();

  ifstream t("test.txt");
  stringstream buffer;
  buffer << t.rdbuf();

  //cout << buffer.str() << endl;
  vector <string> tokens;
  string token;
  while(getline(buffer, token, '\n')){
  	tokens.push_back(token);
  }

  size_t s;
  string str;
  if(find_str(tokens, "TEMPERATURE_MAX=", str)){
  	cfg.val1 = stoi(str, &s);
  }

  if(find_str(tokens, "SNMP_C2GROUP_SECURITY=", str))
  	strcpy(cfg.str, str.c_str());

  if(find_str(tokens, "BURST_THOLD=", str))
  	cfg.val2 = stoi(str, &s);

  if(find_str(tokens, "TRUST_THOLD=", str))
  	cfg.val2 = stoi(str, &s);

  cout << cfg.val1 << endl;
  cout << cfg.str << endl;
  cout << cfg.val2 << endl;

  if(cfg_err.any()){
  	cfg_err.log();
  }

  exit(EXIT_SUCCESS);
}
