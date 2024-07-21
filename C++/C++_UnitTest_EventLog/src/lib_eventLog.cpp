/*
 * lib_statLog.cpp
 *
 *  Created on: May 19, 2023
 *      Author: asharma
 */

/*
 * lib_eventLog.cpp
 *
 *  Created on: May 19, 2023
 *      Author: asharma
 */
/* C Header */
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* Linux specific */
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* IBUC specific */
#include "lib_event_log.h"


#define EVENT_LOG_MAX_ROWS  1000

const uint8_t EVT_ALM = 1;
const uint8_t EVT_CLR = 2;
const uint8_t EVT_INF = 3;
const uint8_t EVT_DATA = 0x80;

enum alarm_e{
	A_RX_INPUT_LVL_LO_ALM	= 0x0001,
	A_VDC_LO_TH_ALM      	= 0x0002,
	A_VDC_HI_TH_ALM      	= 0x0004,
	A_IDC_LO_TH_ALM       = 0x0008,
	A_IDC_HI_TH_ALM       = 0x0010,

	B_RX_INPUT_LVL_LO_ALM = 0x0020,
	B_VDC_LO_TH_ALM       = 0x0040,
	B_VDC_HI_TH_ALM       = 0x0080,
	B_IDC_LO_TH_ALM       = 0x0100,
	B_IDC_HI_TH_ALM       = 0x0200,

	A_RX_SIMULATED_ALM    = 0x0400,
	B_RX_SIMULATED_ALM    = 0x0800,
	REF_10MHZ_ALM     		= 0x1000,
	INTERNAL_10MHZ_1      = 0x2000,
	WGUIDE_SW_FAULT_ALM   = 0x4000,
	EMERG_OVERRIDE_ON     = 0x8000,
};

#define RX_CONFIGURABLE_MASK (A_RX_INPUT_LVL_LO_ALM | \
                              B_RX_INPUT_LVL_LO_ALM | \
                              A_VDC_LO_TH_ALM | \
                              A_VDC_HI_TH_ALM | \
                              A_IDC_LO_TH_ALM | \
                              A_IDC_HI_TH_ALM | \
                              B_VDC_LO_TH_ALM | \
                              B_VDC_HI_TH_ALM | \
                              B_IDC_LO_TH_ALM | \
                              B_IDC_HI_TH_ALM)
#if(0)
enum class Max {
	length = 80
};
#endif


const char *event_log_file     = "ibuc_event.log";
const char *event_log_file_bak = "ibuc_event_bak.log";


static const char *cmd_string_table[] = {
		"EHZ","BAM","BSM","BSW","CIA","CIC", "CIG","CIHA","CIHD",
		"CIM","CIS","CM1","CM2","CMS","CPS","CTD","CTM",
		"IAL","IAH","IBL","IBH","NTP","RAS","RBS","RAZ","RBZ","RAL","RBL","SHP",
		"SHZ","TZO","VAL","VAH","VBL","VBH","CRT1","CRT2","C13A","C13B",
		"C22A","C22B","TME"
};

static const char *status_string_table[] = {
  " ALARM"," CLEAR",""
};

static const char *const rx_alarm_string_table[] = {
 // 01234567890123456789
   "A input lvl lo",
   "B input lvl lo",
   "A simulated flt",
   "B simulated flt",
   "10MHz Ref fault",
   "A VDC lvl lo",
   "A VDC lvl hi",
   "B VDC lvl lo",
   "B VDC lvl hi",
   "Switch Fault",
   "A IDC lvl lo",
   "A IDC lvl hi",
   "B IDC lvl lo",
   "B IDC lvl hi",
   "Emerg Override",
   "Internal 10MHz #1",
   NULL
};

const char *const misc_alarm_string_table[] = {
  "Err Rdng Cfg Dat",
  "Reboot",
  NULL
};

uint32_t rtclk_get_timestamp(void);

EventLog::EventLog(uint16_t event, uint8_t status, double data){
	event_log_entry.times_stamp = rtclk_get_timestamp();
	event_log_entry.event       = event;
	event_log_entry.status      = status;
	event_log_entry.data        = data;
}

void EventLog::erase(void){
  struct stat buffer;
  int status;
  status = lstat(event_log_file, &buffer);
  if(status != 0){
    if(remove(event_log_file) == -1){
    	fprintf(stderr,"Stat Log file rm fail:%s\n",strerror(errno));
    }
  }
}

EventLog::EventLog(){}

int EventLog::read(void){
	int status = 0;

	ifstream event_log_stream(event_log_file);
	if((event_log_stream.rdstate() & ifstream::failbit) != 0){
		return -1;
	}

	stringstream buffer;
	buffer << event_log_stream.rdbuf();

	string token;
	while(getline(buffer, token, '\n')){
		/* token has no \n */
		event_log_v.push_back(token);
	}
	event_log_stream.close();
	return status;
}

/*******************************************************************************
This function is used to write 0 to 32768 bytes.
*******************************************************************************/
int EventLog::write(void){
	int status = 0;
	ofstream outFile(event_log_file_bak, ofstream::trunc);

	for(const string &entry: event_log_v){
		outFile << entry << endl;
	}
	outFile.close();
	outFile.flush();

  return status;
}

/*******************************************************************************
This function converts the rtc time to a timestamp.
This function is required by the TerraFS.
*******************************************************************************/
uint32_t rtclk_get_timestamp(void){
	uint32_t time_u32 = 0;
	uint8_t mon, day, yr, hr, min, sec;
	time_t t;
	struct tm *loc;

	t = time(NULL);
	t += 0;

	loc = localtime(&t);
	if (loc == NULL){
		fprintf(stderr,"localtime failed\n");
	}

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

void rtclk_get_log_time_str(uint32_t stamp, char str[MAX_LENGTH]){
	uint8_t mon, day, hr, min, sec;
  uint16_t yr = 0;

  yr = (stamp >> 25 & 0x7F) + 2000;

  mon = stamp >> 21 & 0x0F;
  day = stamp >> 16 & 0x1F;
  hr = stamp >> 11 & 0x1F;
  min = stamp >> 5 & 0x3F;
  sec = (stamp & 0x1F) * 2;

  snprintf(str, MAX_LENGTH, "%04u-%02u-%02u %02u:%02u:%02u",
  		yr, mon, day,hr, min, sec);
}


void EventLog::stringify
(event_log_entry_s entry,char buff[MAX_LENGTH],char seperator){
  uint16_t index;
  struct in_addr addr;
  int total_copied = 0, n = 0;
  char *p = buff;

  char tmp[MAX_LENGTH];
  memset(tmp, '\0', MAX_LENGTH );

  rtclk_get_log_time_str(entry.times_stamp, tmp);
  n = sprintf(p, "%s ", tmp);
  total_copied = n;
  p += n;

  // Determine which string table to index.
  if(entry.event > EVT_EMERG_OVERRIDE_ON && entry.event <= EVT_TME) {
    // Cfg string table.
    index = entry.event - EVT_AHZ;
    n = sprintf(p, "%s", cmd_string_table[index]);
    total_copied += n;
    if(total_copied < MAX_LENGTH)
    	p += n;

    if(entry.status & EVT_DATA) {
      switch(entry.event) {
      /* IP Addresses Changes */
      case EVT_CIA:
      case EVT_CIG:
      case EVT_CIHA:
      case EVT_CIHD:
      case EVT_CIM:
      case EVT_NTP:
        // IP addresses
        addr.s_addr = htonl(static_cast<uint32_t>(entry.data));
        n = sprintf(p, "=%s", inet_ntoa(addr));
        total_copied += n;
        if(total_copied < MAX_LENGTH)
        	p += n;
        break;

      /* Thresholds changes */
      case EVT_RAL:
      case EVT_RBL:
      case EVT_VAL:
      case EVT_VAH:
      case EVT_VBL:
      case EVT_VBH:
        // Integer to floats
      	n = sprintf(p, "=%.2f", (*(float *)&entry.data) / 100.0);
        total_copied += n;
        if(total_copied < MAX_LENGTH)
        	p += n;
        break;

      /* Mask changes */
      case EVT_CM1:
      case EVT_CM2:
      case EVT_CMS:
        n = sprintf(p, "=0x%04X", *(uint16_t*)&entry.data & RX_CONFIGURABLE_MASK);
        total_copied += n;
      	if (total_copied < MAX_LENGTH)
      		p += n;
        break;

      case EVT_TZO: /* Converted Timezone value can be a negative number */
      	n = sprintf(p, "=%d", static_cast<int32_t>(entry.data) & 0xffffffff);
      	total_copied += n;
      	if (total_copied < MAX_LENGTH)
      		p += n;
        break;

      default:
        // 2 byte unsigned integers
      	n = sprintf(p, "=%hu", static_cast<uint16_t>(entry.data) & 0xffff);
      	total_copied += n;
      	if (total_copied < MAX_LENGTH)
      		p += n;
        break;
      }
    }
  } else if(entry.event < EVT_AHZ) {
    index = entry.event - EVT_A_RX_INPUT_LVL_LO;
    n = sprintf(p, "%s", rx_alarm_string_table[index]);
    total_copied += n;
    if(total_copied < MAX_LENGTH)
    	p += n;

    index = (entry.status & 0x0f) - 1;
    n = sprintf(p, "%s", status_string_table[index]);
    total_copied += n;
  	if (total_copied < MAX_LENGTH)
  		p += n;
  } else {
    if(entry.event == EVT_REBOOT)
    	index = 1;

    n = sprintf(p, "%s", misc_alarm_string_table[index]);
    total_copied += n;
    if(total_copied < MAX_LENGTH)
    	p += n;

    index = (entry.status & 0x0f) - 1;

    n = sprintf(p, "%s", status_string_table[index]);
    total_copied += n;
    if(total_copied < MAX_LENGTH)
    	p += n;
  }
#if(0)
  printf("%s\n",buff );
#endif
}

void EventLog::enqueue(void){
	read();

	/* Erase the entry from the beginning if the size is exceeded */
	if( !event_log_v.empty() && (event_log_v.size() >= EVENT_LOG_MAX_ROWS) ){
		event_log_v.erase(event_log_v.begin());
	}

	/* Queue in the new entry */
	char tmp[MAX_LENGTH];
	memset(tmp, '\0', MAX_LENGTH);

	stringify(event_log_entry, tmp, ' ');
	event_log_v.push_back(tmp);

	/* Write permanently */
	write();
	if(rename(event_log_file_bak, event_log_file) == -1){
		perror("rename failed:" );
	}
}


/*
 * Returns the oldest statistic that has not been read.
 */
int EventLog::next_entry(char str[MAX_LENGTH]){
	read();

  if(event_log_v.size() > 0){
  	string s = event_log_v.front();
  	strcpy(str,s.c_str());

  	event_log_v.erase(event_log_v.begin());
  	write();
  	if(rename(event_log_file_bak, event_log_file) == -1){
  		perror("rename failed:" );
  	}
  	//event_log_v.clear(); /* Next read will fill it again */
  	return 0;
  }else{
  	return -1;
  }
}



