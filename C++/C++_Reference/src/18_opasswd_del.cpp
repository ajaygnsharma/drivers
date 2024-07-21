/*
 * 18_opasswd_del.cpp
 *
 *  Created on: Mar 29, 2023
 *      Author: asharma
 */


/* C++ headers */
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

/* Linux OS specific headers */
#include <sys/stat.h>
#include <pwd.h>
#include <shadow.h>


/* C headers */
#include <string.h>

using namespace std;

class User{
public:
	User();
	int  mirror_passwd(void);
	void reset_lock(string name);
	int  remove_opasswd_entry(string name);
	void remove_opasswd_file(void);
	void unlock_all_users(void);
	int  get_count(void);
	int  print_list(void);
	int  run_cmd_adduser(char *name, char *type, struct sys_user_s &usr);
	int  lib_user_delete_chpasswd_file(void);
	int  run_cmd_chpasswd(char *name, char *passwd);
	int  lib_user_create_chpasswd_file(char *name, char *passwd);
	int  set_shadow_db(char *name);

private:
	int read_opasswd_entry(string name);
	int write_opasswd_entry(void);

	vector<string> opasswd_v;

	const char *tmp_file = "/tmp/users";
#if(1)
	const char *opasswd_file     = "./src/opasswd";
	const char *opasswd_bak_file = "./src/opasswd.bak";
#else
	char *opasswd_file     = "/etc/security/opasswd";
	char *opasswd_bak_file = "/etc/security/opasswd.bak";
#endif

};

void User::reset_lock(string name){
	char cmd[100] = {0};
	snprintf(cmd, 100, "/sbin/pam_tally2 --reset=0 --user=%s",name.c_str());

	int  status = 0;
	status = system(cmd); //Run command
	if(status != 0){
		perror("Cannot run pam_tally2:");
	}
}


int User::read_opasswd_entry(string name){
	ifstream opasswd_stream(opasswd_file);
	if((opasswd_stream.rdstate() & ifstream::failbit) != 0){
		return 0;
	}

	stringstream buffer;
	buffer << opasswd_stream.rdbuf();
	opasswd_stream.close();

  cerr << buffer.str() << endl;

	string token;
	while(getline(buffer, token, '\n') && (token.length() > 0)){
		size_t found = token.find(name);
		if(found != string::npos){
			/* Found the name. Ignore that line */
		}else{
			/* Save the other password entries */
			opasswd_v.push_back(token);
		}
	}
}

int User::write_opasswd_entry(void){
	fprintf(stderr,"%d\n",__LINE__);
	/* Please only create the opasswd file if it exists */
	if(opasswd_v.size() != 0){
		int status = 0;
		ofstream outFile(opasswd_bak_file);

		for(const string &s: opasswd_v){
			outFile << s << endl;
		}
		outFile.close();

		return status;
	}
}

User::User(){
}

int User::remove_opasswd_entry(string name){
	read_opasswd_entry(name);
	fprintf(stderr,"%d\n",__LINE__);
	write_opasswd_entry();

	fprintf(stderr,"%d\n",__LINE__);
	struct stat sb;
	if( (stat(opasswd_bak_file, &sb) != -1) && (sb.st_size != 0) ){
		rename(opasswd_bak_file, opasswd_file);
	}
}

int main(void){
	User u = User();
  string username = "factory";
  u.reset_lock(username);
  u.remove_opasswd_entry(username);

}
