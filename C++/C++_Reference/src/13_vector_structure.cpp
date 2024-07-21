
#include <string.h>
#include <sys/stat.h>

#include <stdint.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

struct LNB_stat_s{
	int16_t volt;
	int16_t amp;
	int16_t pin;
};

// This is used to write to fram to avoid extra bytes from structure padding.
struct stat_entry_s {
  uint32_t date_time;
  struct LNB_stat_s LNB_A;
  struct LNB_stat_s LNB_B;
  int16_t  ten_mhz;
};


const char *statlog_file = "ibuc_stat.log";

uint32_t rtclk_get_timestamp(void)
{
	uint32_t time_u32 = 0;
	uint8_t mon, day, yr, hr, min, sec;
	time_t t;
	struct tm *loc;

	t = time(NULL);
	t += 0;

	loc = localtime(&t);
	if (loc == NULL)
		cerr << "localtime failed\n" ;

	mon = loc->tm_mon + 1;
	day = loc->tm_mday;
	yr  = (loc->tm_year + 1900) - 2000;
	hr  = loc->tm_hour;
	min = loc->tm_min;
	sec = loc->tm_sec;

	time_u32 = yr << 25;
	time_u32 |= mon << 21;
	time_u32 |= day << 16;
	time_u32 |= hr << 11;
	time_u32 |= min << 5;
	time_u32 |= sec / 2;

	return time_u32;
}

/*******************************************************************************
Remove the Statlog File.
*******************************************************************************/
void stat_log_erase(void){
  struct stat buffer;
  int status;
  status = lstat(statlog_file, &buffer);
  if(status != 0){
    if(remove(statlog_file) == -1){
    	perror("Stat Log file rm fail:\n");
    }
  }
}

/*******************************************************************************
This function is used to write 0 to 32768 bytes.
*******************************************************************************/
static int statlog_write(vector<struct stat_entry_s> stat_log){
	ofstream outFile(statlog_file, ofstream::trunc );

	/* << operator automatically overloads and converts data to string */
	for(const struct stat_entry_s &s: stat_log){
		outFile << s.date_time << ",";
	  outFile << s.LNB_A.amp << "," << s.LNB_A.volt << "," << s.LNB_A.pin << ",";
	  outFile << s.LNB_B.amp << "," << s.LNB_B.volt << "," << s.LNB_B.pin << ",";
	  outFile << s.ten_mhz << endl;
	}
	outFile.close();
  return 0;
}

/*******************************************************************************
This function is used to read anywhere from one byte to the entire fram.
You must provide a buffer of suitable size for the returned data.
*******************************************************************************/
static int statlog_read(vector<struct stat_entry_s> &stat_log){
	ifstream statlog_stream(statlog_file);
	if((statlog_stream.rdstate() & ifstream::failbit) != 0){
		return -1;
	}

	stringstream buffer;
	buffer << statlog_stream.rdbuf();

	string token;
	while(getline(buffer, token, '\n')){
		struct stat_entry_s s;
		int n = sscanf(token.c_str(), "%u,%hd,%hd,%hd,%hd,%hd,%hd,%hd",
				&s.date_time, &s.LNB_A.amp, &s.LNB_A.volt, &s.LNB_A.pin,
				&s.LNB_B.amp, &s.LNB_B.volt, &s.LNB_B.pin,
				&s.ten_mhz);
		stat_log.push_back(s);
	}
	statlog_stream.close();

	return 0;
}


struct stat_entry_s *get_next_stat_entry(void){
	static struct stat_entry_s s;
	vector<struct stat_entry_s> stat_log;

  statlog_read(stat_log);
  s = stat_log.front();
  stat_log.erase(stat_log.begin());
  statlog_write(stat_log);

	return &s;
}

void enqueue_stat_entry(int16_t *data){
  struct stat_entry_s curr;

  curr.date_time  = rtclk_get_timestamp();
  curr.LNB_A.amp  = data[0];
  curr.LNB_A.volt = data[1];
  curr.LNB_A.pin  = data[2];
  curr.LNB_B.amp  = data[3];
  curr.LNB_B.volt = data[4];
  curr.LNB_B.pin  = data[5];
  curr.ten_mhz    = data[6];

#if(0)
  if(DEBUG_LOGGER_OFF_en()) return;
#endif

  vector<struct stat_entry_s> stat_log;

  statlog_read(stat_log);
  stat_log.push_back(curr);
  statlog_write(stat_log);
}

int main(void){
	struct stat_entry_s s[2];
	int16_t data[10][7] = {
			{10, 20, 30, 100, 200, 300, 1},
			{-10, -20, -30, -100, -200, -300, -1}
	};

	enqueue_stat_entry(data[0]);
	enqueue_stat_entry(data[1]);

	struct stat_entry_s *stat_log = get_next_stat_entry();
	cout << stat_log->LNB_A.amp << "," << stat_log->LNB_A.volt << "," << stat_log->LNB_A.pin << ","
			<< stat_log->LNB_B.amp << "," << stat_log->LNB_B.volt << "," << stat_log->LNB_B.pin << ","
			<< stat_log->ten_mhz << endl;

	return 0;
}
