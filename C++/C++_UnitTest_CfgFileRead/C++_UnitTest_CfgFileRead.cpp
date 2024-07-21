#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

#define OPTION_SNMP_V1_V2C 1
#define IBUC_CFG                     "./ibuc_cfg.txt"

/* 51 is 50(valid chars) + 1'\0' */
enum IBUC_e{
	MAX_LEN    					 = 51,
  IBUC_SNMP_MAX_USERS  = 10,
  IBUC_SNMP_MAX_VIEWS  = 10,
  IBUC_SNMP_MAX_ACCESS = 10,
  IBUC_SNMP_MAX_TRAPS  = 10,
  IBUC_SYS_MAX_USERS   = 10,
	IBUC_SNMP_MAX_SESSIONS=10,
	IBUC_MAX_NTP_SERVERS  = 2
};


struct redundancy_s{
	int16_t switch_position;
	/* Refer to IBUC.h for all possible values for each */
	uint16_t switching_mode; /* Reverting/Non Reverting */
	uint16_t switching_type; /* Manual/Automatic */
};

struct LNB_s{
  	int16_t  low_threshold_mB;
  	uint16_t vdc_low_threshold;
 	  uint16_t vdc_high_threshold;
 	  uint16_t idc_low_threshold;
 	  uint16_t idc_high_threshold;
 	  bool     alarm_simulate;
 	  bool     alarm_suppress;
};

struct ten_MHz_s{
  	bool alm_enable;
  	uint16_t trim[2];
  	uint16_t internal_enable;
  	uint16_t calibration;
};

/*
 * SNMP Info
 */
enum SNMP_TYPE{
  SNMP_TRAP,
  SNMP_INFORM
};

enum snmp_community_type_e{
  SNMP_COMMUNITY_PUBLIC,
  SNMP_COMMUNITY_PRIVATE,
  SNMP_COMMUNITY_TRAP,
};

struct trap_s{
  uint32_t enable[2];
  uint32_t version;
  uint32_t type;
  char     string[33];
  uint32_t addr[2];
};

enum snmp_version_e{
	SNMP_DISABLED,
	SNMP_V1,
	SNMP_V2,
	SNMP_V3,
};

#if(OPTION_SNMP_COM2SEC)
/* Com2sec is not used since only V3(USM) is supported by this IBUC */
struct com2sec_s{
  char name[33];
  char ip[33];
  char netmask[33];
};
#endif

/* This is the Access Directive of the VACM model.
 * We do not specify the model and its assumed to be v3. */
struct snmp_access_s{
  char name[MAX_LEN];      // Group Name
  char level[MAX_LEN];      // auth|priv
  char readView[MAX_LEN];   // valid view name
  char writeView[MAX_LEN];
  char notifyView[MAX_LEN];//!!Warning!! This may not be supported.
};

/* View access table: Only specify the view data. Access table maps the view
 * to group.
 */
struct snmp_view_s{
  char name[MAX_LEN];      // Name of the view
  char flag[MAX_LEN];      // Included/Excluded
  char subtree[MAX_LEN];   // OID Tree
  char mask[MAX_LEN];      // Mask of 0xffa0
};

/* USM Table or entry for the individual user. We add an extra mapping to group
 * thereby avoiding the sec2group. Sec2Group really maps group to a security
 * where the security is community string in case of v1/v2c and a USM user in
 * case of v3 USM model. Since we only supported USM model, we can omit the need
 * for a mapping and directly assume a group for a user. */
struct snmp_user_s{
  char name[MAX_LEN];
  char level[MAX_LEN];     // auth(ro), priv(rw)
  char auth[MAX_LEN];      // MD5|SHA|SHA-512|SHA-256|SHA-224
  char authpass[MAX_LEN];  // Authentication Password
  char encryption[MAX_LEN];// DES or AES
  char privpass[MAX_LEN];  // privpass(Password for encryption)
  char group[MAX_LEN];     // Group that this user belongs to.
};

struct trap_sess_s{
	char profile[MAX_LEN];
	char username[MAX_LEN];
	char privilege[MAX_LEN];
	char authentication[MAX_LEN];
	char authentication_password[MAX_LEN];
	char encryption[MAX_LEN];
	char encryption_password[MAX_LEN];
	uint32_t trap_ip;
};

struct notification_filter_s{
	char name[MAX_LEN]; // Filter/Profile name
	char subtree[MAX_LEN]; // OID Tree
	char flag[MAX_LEN]; // Included/Excluded
	char mask[MAX_LEN]; // Mask of 0xff00
};

enum sys_user_type_e{
  SYS_USER_USER = 1,
  SYS_ADMIN_USER
};

struct sys_user_s{
  char name[MAX_LEN];
  char passwd[MAX_LEN];
  enum sys_user_type_e type;
};

struct snmp_config_s{/* Restore default */
    uint32_t version;
    char sysContact[MAX_LEN];
    char sysLocation[MAX_LEN];
    char sysName[MAX_LEN];
    struct trap_s trap;
#if(OPTION_SNMP_COM2SEC)
    struct com2sec_s com2sec;
#endif
    struct snmp_access_s access[IBUC_SNMP_MAX_ACCESS];
    uint32_t access_count;
    struct snmp_view_s view[IBUC_SNMP_MAX_VIEWS];
    uint32_t view_count;
    struct snmp_user_s user[IBUC_SNMP_MAX_USERS];
    uint32_t user_count;
    struct trap_sess_s trap_sess[IBUC_SNMP_MAX_TRAPS];
    uint32_t trap_sess_count;
    struct notification_filter_s notify_filter[IBUC_SNMP_MAX_SESSIONS];
    uint32_t notify_filter_count;
#if(OPTION_SNMP_V1_V2C)
    char publicStr[MAX_LEN];
    char privateStr[MAX_LEN];
#endif
};

struct ntp_server_s{
	char ip[IBUC_MAX_NTP_SERVERS][MAX_LEN];
  uint32_t count;
  int32_t  time_zone_offset;        /* Restore default */
};


struct alarm_s{
	uint16_t major_mask;
	uint16_t minor_mask;
	uint16_t suppression_mask;
	uint16_t state;
	bool     suppressedFlag[2];
};

/*
 * Top level config
 */
struct cfg_s{
  struct LNB_s LNB[2];
  struct alarm_s alarm;

  struct redundancy_s redundancy;
  struct ten_MHz_s    ten_MHz;
  struct snmp_config_s snmp;

  char user_title[MAX_LEN];  /* Restore default */

  uint32_t ip_addr;                 /* Restore default */
  uint32_t gateway;                 /* Restore default */
  uint32_t netmask;                 /* Restore default */

  struct ntp_server_s ntp_server;

  uint16_t stat_log_time;
  uint16_t control_value_13V;
  uint16_t enable_22K;

  uint32_t ssh_enabled;
  uint32_t web_enabled;
};

const char* const misc_alarm_str[] = {
   "Not Used",
   "Err Rdng Cfg Dat",          // #define  CFG_REST_FAULT_ALM     0x0002
   "Fw Download",               // #define  FW_DOWNLOAD_ALM        0x0004
   "Not Used",
   "Reboot",                    // #define  REBOOT_ALM             0x0010
   "Clone Comm Fault",          // #define  CLONE_COMM_FLT_ALM     0x0020
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL};

/* All strings in "" are NULL terminated. */
const struct snmp_config_s snmp_cfg_default = {
    3, //SNMP enable
    "contact",
    "location",
    "IBUC", //Sysname
    { {1, 0}, 3, SNMP_TRAP, "public", {0, 0} },// Trap:En0,1,type, string, ip1,2
#if(OPTION_SNMP_COM2SEC)
    {"mysecurity", "192.168.1.0", "24"}, // com2sec
#endif
    //Access: grp name, ver, level, readView, WriteView, NotifyView
    {{"iBUCGroup", "auth", "systemview", "systemview", "none"},
     {"none", "none", "none", "none", "none"},
     {"none", "none", "none", "none", "none"},
     {"none", "none", "none", "none", "none"},
     {"none", "none", "none", "none", "none"},
     {"none", "none", "none", "none", "none"},
     {"none", "none", "none", "none", "none"},
     {"none", "none", "none", "none", "none"},
     {"none", "none", "none", "none", "none"},
     {"none", "none", "none", "none", "none"}},
    1,
    {{"all", "excluded",".1", "none"},
     {"systemview", "excluded", ".1", "none"},
     {"systemview", "included", ".1.3.6.1.2.1.1", "none"},     // System MIB(only). Others are masked out. Although can be included if needed.
     {"systemview", "included", ".1.3.6.1.6.3", "none"},       // SNMP MIB
     {"systemview", "included", ".1.3.6.1.4.1.21369", "none"}, // IBUC MIB
     {"none", "none", "none", "none"},
     {"none", "none", "none", "none"},
     {"none", "none", "none", "none"},
     {"none", "none", "none", "none"},
     {"none", "none", "none", "none"}},
    5,
    // Assume no users first
    {{"none", "none", "none", "none", "none", "none", "none"},
     {"none", "none", "none", "none", "none", "none", "none"},
     {"none", "none", "none", "none", "none", "none", "none"},
     {"none", "none", "none", "none", "none", "none", "none"},
     {"none", "none", "none", "none", "none", "none", "none"},
     {"none", "none", "none", "none", "none", "none", "none"},
     {"none", "none", "none", "none", "none", "none", "none"},
     {"none", "none", "none", "none", "none", "none", "none"},
     {"none", "none", "none", "none", "none", "none", "none"},
     {"none", "none", "none", "none", "none", "none", "none"}},
    0,
	 {{"none", "none", "none", "none", "none", "none", "none", 0},
		{"none", "none", "none", "none", "none", "none", "none", 0},
		{"none", "none", "none", "none", "none", "none", "none", 0},
		{"none", "none", "none", "none", "none", "none", "none", 0},
		{"none", "none", "none", "none", "none", "none", "none", 0},
		{"none", "none", "none", "none", "none", "none", "none", 0},
		{"none", "none", "none", "none", "none", "none", "none", 0},
		{"none", "none", "none", "none", "none", "none", "none", 0},
		{"none", "none", "none", "none", "none", "none", "none", 0},
		{"none", "none", "none", "none", "none", "none", "none", 0}},
	  0,
		{{"none", "none", "excluded", "0x0000"},
		 {"none", "none", "excluded", "0x0000"},
		 {"none", "none", "excluded", "0x0000"},
		 {"none", "none", "excluded", "0x0000"},
		 {"none", "none", "excluded", "0x0000"},
		 {"none", "none", "excluded", "0x0000"},
		 {"none", "none", "excluded", "0x0000"},
		 {"none", "none", "excluded", "0x0000"},
		 {"none", "none", "excluded", "0x0000"},
		 {"none", "none", "excluded", "0x0000"}},
		0,
		"public",
		"private"
};

enum service_status_e{
	DISABLED= 0,
	ENABLED = 1
};

enum LNB_position_e{
	LNB_POS_A,
	LNB_POS_B,
	LNB_ALL
};

enum redundancy_e{
	SW_POS_BOTH = 2,
	SW_POS_NONE = 3,

	FIF_NONE                     = 0,
  FIF_NOTREDUND                = 1,
  FIF_RED_AND_SLAVE            = 2,
  FIF_SLAVE_POS                = 3,
  FIF_NOT_RED_OR_REDANDSLAVE   = 4,

	SWITCHING_MODE_REVERTING     = 0,
	SWITCHING_MODE_NON_REVERTING = 1,

	SWITCHING_TYPE_MANUAL        = 0,
	SWITCHING_TYPE_AUTOMATIC     = 1,
	STANDBY_DISABLED             = 0,
	STANDBY_WARM                 = 1,
};


const static struct redundancy_s redundancy_default = {
		.switch_position = LNB_POS_A,
		.switching_mode  = SWITCHING_MODE_REVERTING,
		.switching_type  = SWITCHING_TYPE_AUTOMATIC,
};

const struct LNB_s LNB_default = {
  	.low_threshold_mB   = -4000,
  	.vdc_low_threshold  =  100,
 	  .vdc_high_threshold =  250,
 	  .idc_low_threshold  =  250,
 	  .idc_high_threshold =  500,
 	  .alarm_simulate     =  false,
 	  .alarm_suppress     =  false
};

enum alarm_e{
	//A_RX_INPUT_LVL_LO_ALM	= (1 << Alarm_pos_e::A_RX_INPUT_LVL_LO),
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


#define RX_ALARM_MASK_MAJOR_DEFAULT (A_RX_SIMULATED_ALM   | \
                                     B_RX_SIMULATED_ALM | \
                                     WGUIDE_SW_FAULT_ALM | \
                                     A_IDC_LO_TH_ALM       | \
                                     A_IDC_HI_TH_ALM | \
                                     B_IDC_LO_TH_ALM | \
                                     B_IDC_HI_TH_ALM | \
                                     A_RX_INPUT_LVL_LO_ALM | \
                                     B_RX_INPUT_LVL_LO_ALM | \
                                     EMERG_OVERRIDE_ON)

#define RX_ALARM_MASK_MINOR_DEFAULT (A_VDC_LO_TH_ALM | \
                                     A_VDC_HI_TH_ALM | \
                                     B_VDC_LO_TH_ALM | \
                                     B_VDC_HI_TH_ALM | \
                                     INTERNAL_10MHZ_1 | \
                                     REF_10MHZ_ALM)

#define RX_ALARM_MASK_SUPRS_DEFAULT (A_RX_INPUT_LVL_LO_ALM | \
                                     B_RX_INPUT_LVL_LO_ALM | \
                                     A_VDC_LO_TH_ALM | \
                                     A_VDC_HI_TH_ALM | \
                                     A_IDC_LO_TH_ALM | \
                                     A_IDC_HI_TH_ALM | \
                                     B_VDC_LO_TH_ALM | \
                                     B_VDC_HI_TH_ALM | \
                                     B_IDC_LO_TH_ALM | \
                                     B_IDC_HI_TH_ALM)

const struct alarm_s alarm_default = {
    .major_mask = RX_ALARM_MASK_MAJOR_DEFAULT,
		.minor_mask = RX_ALARM_MASK_MINOR_DEFAULT,
		.suppression_mask = RX_ALARM_MASK_SUPRS_DEFAULT
};

const struct ten_MHz_s ten_MHz_default = {
		.alm_enable = ENABLED,
  	.trim = { 2048, 2048 },
		.internal_enable = 0,
  	.calibration = 0
};

const struct ntp_server_s ntp_server_default = {
		.ip = {"0.pool.ntp.org", "1.pool.ntp.org"},
		.count = 2,
		.time_zone_offset = 0
};

const struct cfg_s cfg_default = {
    .LNB = { LNB_default, LNB_default },
		.alarm  = alarm_default,
		.redundancy = redundancy_default,
		.ten_MHz = ten_MHz_default,
		.snmp = snmp_cfg_default,
		"Undefined",//user_title
		.ip_addr = 0xC0A80115, //ip_addr 192.168.1.21
		.gateway = 0xC0A80101, //gateway 192.168.1.1
		.netmask = 0xFFFFFF00, //netmask 255.255.255.0
		.ntp_server = ntp_server_default,
		.stat_log_time = 1440,
		.control_value_13V  = ENABLED,
		.enable_22K  = ENABLED,
		.ssh_enabled = ENABLED,
		.web_enabled = ENABLED,
};


#define CFG_MAX_SIZE 10000
static char cfg_buf[CFG_MAX_SIZE];

static struct cfg_s *cfg_p;

int cfg_map(void){
	int fd;
	/* Make sure that the process mask is 006. That way resulting file
	 * permission is true in the user and group bits */
	mode_t mask = (S_IROTH | S_IWOTH);
	umask(mask);

	fd = shm_open("/ibuc-cfg", O_RDWR | O_CREAT, \
	    (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) );
	if(fd < 0) {
	  fprintf(stderr, "shm_open in ibuc-cfg: %s\n", strerror(errno));
	  return -1;
	}
	if(ftruncate(fd, sizeof(struct cfg_s)) == 0){
		cfg_p = static_cast<struct cfg_s *>(mmap(NULL, sizeof(struct cfg_s),
				PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));

		if(cfg_p == MAP_FAILED){
			fprintf(stderr,  "mmap in ibuc-cfg: %s\n", strerror(errno));
		}
	}
  if (close(fd) == -1)
  	fprintf(stderr, "close: ibuc-cfg %s\n", strerror(errno));
  return 0;
}


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
			if(v1.size() > 1){
				dest = v1[1];
			}else{
				dest = "0";
			}

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

/*
 * cfg_read() - configuration read
 *
 * Parses, loads configuration file into shared memory.If there are errors
 * parsing any element, then an error message for the entry is added to
 * a list. The user can try to read the errors messages to see which entry is
 * not read and notify user to fix configuration file. This applies to ECFG_READ
 * situation only. To read error use: cfg_get_error(char **msg);
 *
 * @param cfg_file
 *
 * return -  SUCCESS if all good.
 *          -ECFG_READ if any of the items cannot be read.
 *          -EFILE_INEXISTENT = File is not present.
 *          -EFILE_OPEN       = File open failed.
 *          -EFILE_READ       = File cannot be read.
 */
int8_t cfg_fread(const char *cfg_file, struct cfg_s *cfg_p){
  memset(cfg_buf, 0, CFG_MAX_SIZE);

  ifstream cfg_stream(cfg_file);
  if((cfg_stream.rdstate() & ifstream::failbit) != 0){
  	return -1;
  }

  stringstream buffer;
  buffer << cfg_stream.rdbuf();

  vector <string> tokens;
  string token;
  while(getline(buffer, token, '\n')){
  	tokens.push_back(token);
  }

  size_t sz;
  string str;
  if(find_str(tokens, "LOW_THRESHOLD_MB0=", str)) {  cfg_p->LNB[0].low_threshold_mB = stod(str, &sz); }
  if(find_str(tokens, "VDC_LOW_THRESHOLD0=", str)){  cfg_p->LNB[0].vdc_low_threshold = stod(str, &sz); }
  if(find_str(tokens, "VDC_HIGH_THRESHOLD0=", str)){ cfg_p->LNB[0].vdc_high_threshold = stod(str, &sz); }
  if(find_str(tokens, "IDC_LOW_THRESHOLD0=", str)){  cfg_p->LNB[0].idc_low_threshold = stod(str, &sz); }
  if(find_str(tokens, "IDC_HIGH_THRESHOLD0=", str)){ cfg_p->LNB[0].idc_high_threshold = stod(str, &sz); }
  if(find_str(tokens, "ALARM_SIMULATE0=", str)){     cfg_p->LNB[0].alarm_simulate = stod(str, &sz); }
  if(find_str(tokens, "ALARM_SUPPRESS0=", str)){     cfg_p->LNB[0].alarm_suppress = stod(str, &sz); }

  if(find_str(tokens, "LOW_THRESHOLD_MB1=", str)) {  cfg_p->LNB[1].low_threshold_mB = stod(str, &sz); }
  if(find_str(tokens, "VDC_LOW_THRESHOLD1=", str)){  cfg_p->LNB[1].vdc_low_threshold = stod(str, &sz); }
  if(find_str(tokens, "VDC_HIGH_THRESHOLD1=", str)){ cfg_p->LNB[1].vdc_high_threshold = stod(str, &sz); }
  if(find_str(tokens, "IDC_LOW_THRESHOLD1=", str)){  cfg_p->LNB[1].idc_low_threshold = stod(str, &sz); }
  if(find_str(tokens, "IDC_HIGH_THRESHOLD1=", str)){ cfg_p->LNB[1].idc_high_threshold = stod(str, &sz); }
  if(find_str(tokens, "ALARM_SIMULATE1=", str)){     cfg_p->LNB[1].alarm_simulate = stod(str, &sz); }
  if(find_str(tokens, "ALARM_SUPPRESS1=", str)){     cfg_p->LNB[1].alarm_suppress = stod(str, &sz); }

  if(find_str(tokens, "ALARM_MINOR_MASK=", str)) {       cfg_p->alarm.minor_mask = stod(str, &sz); }
  if(find_str(tokens, "ALARM_MAJOR_MASK=", str)) {       cfg_p->alarm.major_mask = stod(str, &sz); }
  if(find_str(tokens, "ALARM_SUPPRESSION_MASK=", str)) { cfg_p->alarm.suppression_mask = stod(str, &sz); }

  if(find_str(tokens, "SWITCH_POSITION=", str)){ cfg_p->redundancy.switch_position = stod(str, &sz); }
  if(find_str(tokens, "SWITCHING_MODE=", str)) { cfg_p->redundancy.switching_mode = stod(str, &sz); }
  if(find_str(tokens, "SWITCHING_TYPE=", str)) { cfg_p->redundancy.switching_type = stod(str, &sz); }

  if(find_str(tokens, "TEN_MHZ_ALARM_ENABLE=", str)) {  cfg_p->ten_MHz.alm_enable = stod(str, &sz); }
  if(find_str(tokens, "TEN_MHZ_TRIM0=", str)) {         cfg_p->ten_MHz.trim[0] = stod(str, &sz); }
  if(find_str(tokens, "TEN_MHZ_TRIM1=", str)) {         cfg_p->ten_MHz.trim[1] = stod(str, &sz); }
  if(find_str(tokens, "TEN_MHZ_INTERNAL_ENABLE=", str)){cfg_p->ten_MHz.internal_enable = stod(str, &sz); }
  if(find_str(tokens, "TEN_MHZ_CALIBRATION=", str)) {   cfg_p->ten_MHz.calibration = stod(str, &sz); }

  if(find_str(tokens, "SNMP_ENABLE=", str)) {             		cfg_p->snmp.version = stod(str, &sz); }
  if(find_str(tokens, "SNMP_CONTACT=", str)){      		 strcpy(cfg_p->snmp.sysContact,str.c_str()); }
  if(find_str(tokens, "SNMP_LOCATION=", str)){     		 strcpy(cfg_p->snmp.sysLocation,str.c_str()); }
  if(find_str(tokens, "SNMP_NAME=", str)){         		 strcpy(cfg_p->snmp.sysName,str.c_str()); }

  if(find_str(tokens, "SNMP_TRAP_ENABLE0=", str)){	cfg_p->snmp.trap.enable[0] = stod(str, &sz); }
  if(find_str(tokens, "SNMP_TRAP_ENABLE1=", str)){	cfg_p->snmp.trap.enable[1] = stod(str, &sz); }
  if(find_str(tokens, "SNMP_TRAP_VERSION=", str)){	cfg_p->snmp.trap.version   = stod(str, &sz); }
  if(find_str(tokens, "SNMP_TRAP_TYPE=",    str)){	cfg_p->snmp.trap.type = stod(str, &sz); }
  if(find_str(tokens, "SNMP_TRAP_COMMUNITY=",str)){ strcpy(cfg_p->snmp.trap.string,str.c_str()); }
  if(find_str(tokens, "SNMP_TRAP_ADDR0=",    str)){	cfg_p->snmp.trap.addr[0] = stod(str, &sz); }
  if(find_str(tokens, "SNMP_TRAP_ADDR1=",    str)){	cfg_p->snmp.trap.addr[1] = stod(str, &sz); }

  if(find_str(tokens, "SNMP_ACCESS_COUNT=", str)){        		cfg_p->snmp.access_count = stod(str, &sz); }
  if(find_str(tokens, "SNMP_VIEW_COUNT=", str)){          		cfg_p->snmp.view_count = stod(str, &sz); }
  if(find_str(tokens, "SNMP_USER_COUNT=", str)){          		cfg_p->snmp.user_count = stod(str, &sz); }
  if(find_str(tokens, "SNMP_TRAP_COUNT=", str)){          		cfg_p->snmp.trap_sess_count = stod(str, &sz); }
  if(find_str(tokens, "SNMP_NOTIFY_FILTER_COUNT=", str)){     cfg_p->snmp.notify_filter_count = stod(str, &sz); }
  if(find_str(tokens, "SNMP_COMMUNITY_PUBLIC=", str)){ strcpy(cfg_p->snmp.publicStr,str.c_str()); }
  if(find_str(tokens, "SNMP_COMMUNITY_PRIVATE=", str)){strcpy(cfg_p->snmp.privateStr,str.c_str()); }
  if(find_str(tokens, "SNMP_COMMUNITY_PRIVATE=", str)){strcpy(cfg_p->snmp.privateStr,str.c_str()); }

  for (int i=0; i<10; i++){
  	string s = "SNMP_ACCESS_NAME" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.access[i].name,str.c_str()); }
  	s = "SNMP_ACCESS_LEVEL" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.access[i].level,str.c_str()); }
  	s = "SNMP_ACCESS_READVIEW" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.access[i].readView,str.c_str()); }
  	s = "SNMP_ACCESS_WRITEVIEW" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.access[i].writeView,str.c_str()); }
  	s = "SNMP_ACCESS_NOTIFYVIEW" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.access[i].notifyView,str.c_str()); }

  	s = "SNMP_VIEW_NAME" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.view[i].name,str.c_str()); }
  	s = "SNMP_VIEW_FLAG" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.view[i].flag,str.c_str()); }
  	s = "SNMP_VIEW_SUBTREE" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.view[i].subtree,str.c_str()); }
  	s = "SNMP_VIEW_MASK" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.view[i].mask,str.c_str()); }

  	s = "SNMP_USER_NAME" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.user[i].name,str.c_str()); }
  	s = "SNMP_USER_LEVEL" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.user[i].level,str.c_str()); }
  	s = "SNMP_USER_AUTH" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.user[i].auth,str.c_str()); }
  	s = "SNMP_USER_AUTHPASS" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.user[i].authpass,str.c_str()); }
  	s = "SNMP_USER_ENCRYPTION" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.user[i].encryption,str.c_str()); }
  	s = "SNMP_USER_PRIVPASS" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.user[i].privpass,str.c_str()); }
  	s = "SNMP_USER_GROUP" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.user[i].group,str.c_str()); }

  	s = "SNMP_PROFILE_NAME" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.trap_sess[i].profile,str.c_str()); }
  	s = "SNMP_TRAP_USERNAME" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.trap_sess[i].username,str.c_str()); }
  	s = "SNMP_TRAP_PRIVILEGE" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.trap_sess[i].privilege,str.c_str()); }
  	s = "SNMP_TRAP_AUTH" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.trap_sess[i].authentication,str.c_str()); }
  	s = "SNMP_TRAP_AUTH_PASS" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.trap_sess[i].authentication_password,str.c_str()); }
  	s = "SNMP_TRAP_ENCRYPT" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.trap_sess[i].encryption,str.c_str()); }
  	s = "SNMP_TRAP_ENCRYPT_PASS" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.trap_sess[i].encryption_password,str.c_str()); }
  	s = "SNMP_TRAP_IP" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ cfg_p->snmp.trap_sess[i].trap_ip = stod(str,&sz); }

  	s = "SNMP_NOTIFY_FILTER_NAME" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.notify_filter[i].name,str.c_str()); }
  	s = "SNMP_NOTIFY_FILTER_SUBTREE" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.notify_filter[i].subtree,str.c_str()); }
  	s = "SNMP_NOTIFY_FILTER_FLAG" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.notify_filter[i].flag, str.c_str()); }
  	s = "SNMP_NOTIFY_FILTER_MASK" + to_string(i) + "=";
  	if(find_str(tokens, s, str)){ strcpy(cfg_p->snmp.notify_filter[i].mask, str.c_str()); }
  }

  if(find_str(tokens, "USER_TITLE=", str)){strcpy(cfg_p->user_title,str.c_str()); }
  if(find_str(tokens, "STAT_LOG_TIME=", str)){ cfg_p->stat_log_time = stod(str,&sz); }
  if(find_str(tokens, "IP_ADDR=", str)){ cfg_p->ip_addr = stoul(str,&sz, 16); }
  if(find_str(tokens, "GATEWAY=", str)){ cfg_p->gateway = stoul(str,&sz, 16); }
  if(find_str(tokens, "NETMASK=", str)){ cfg_p->netmask = stoul(str,&sz, 16); }
  if(find_str(tokens, "TIME_ZONE_OFFSET=", str)){ cfg_p->ntp_server.time_zone_offset = stoi(str,&sz); }
  if(find_str(tokens, "NTP_SERVER0=", str)){ strcpy(cfg_p->ntp_server.ip[0],str.c_str()); }
  if(find_str(tokens, "NTP_SERVER1=", str)){ strcpy(cfg_p->ntp_server.ip[1],str.c_str()); }
  if(find_str(tokens, "NTP_SERVER_COUNT=", str)){ cfg_p->ntp_server.count = stoi(str,&sz); }
  if(find_str(tokens, "SSH_ENABLED=", str)){ cfg_p->ssh_enabled = stoi(str,&sz); }
  if(find_str(tokens, "WEB_ENABLED=", str)){ cfg_p->web_enabled = stoi(str,&sz); }

  if(cfg_err.any()){
    	cfg_err.log();
    	return -1;
  }else{
  	return 0;
  }
}

int8_t cfg_read(const char *cfg_file){
	return cfg_fread(cfg_file, cfg_p);
}

int8_t cfg_fsave(const char *cfg_file, struct cfg_s *cfg_p){
  ssize_t n = 0;

  /* This config should be rw-,rw-,-- */
  mode_t mask = (S_IROTH | S_IWOTH);
  umask(mask);

  FILE *pFile = NULL;
  pFile = fopen(cfg_file, "wb");
  if(pFile == NULL){
    fprintf(stderr,"w err %s\n", cfg_file);
    return -(2);
  }else{
  	char *cfg_ptr = cfg_buf;
  	memset(cfg_ptr, 0, CFG_MAX_SIZE);
  	uint32_t bytes_copied = 0;

  	for(int i = 0; i < 2; i++){
    	n = snprintf(&cfg_ptr[bytes_copied], CFG_MAX_SIZE,
    			"LOW_THRESHOLD_MB%d=%d\n"
          "VDC_LOW_THRESHOLD%d=%hu\n"
          "VDC_HIGH_THRESHOLD%d=%hu\n"
          "IDC_LOW_THRESHOLD%d=%hu\n"
          "IDC_HIGH_THRESHOLD%d=%hu\n"
    			"ALARM_SIMULATE%d=%u\n"
          "ALARM_SUPPRESS%d=%u\n",
  				i,cfg_p->LNB[i].low_threshold_mB,\
  				i,cfg_p->LNB[i].vdc_low_threshold,\
					i,cfg_p->LNB[i].vdc_high_threshold,\
					i,cfg_p->LNB[i].idc_low_threshold,\
					i,cfg_p->LNB[i].idc_high_threshold,\
					i,cfg_p->LNB[i].alarm_simulate,\
					i,cfg_p->LNB[i].alarm_suppress);
    	bytes_copied += n;
  	}

  	n = snprintf(&cfg_ptr[bytes_copied], CFG_MAX_SIZE,
  		"ALARM_MINOR_MASK=%u\n"
			"ALARM_MAJOR_MASK=%u\n"
			"ALARM_SUPPRESSION_MASK=%u\n",
  		cfg_p->alarm.minor_mask,
			cfg_p->alarm.major_mask,
			cfg_p->alarm.suppression_mask);
  	bytes_copied += n;

  	n = snprintf(&cfg_ptr[bytes_copied], CFG_MAX_SIZE,
  		"SWITCH_POSITION=%u\n"
			"SWITCHING_MODE=%u\n"
			"SWITCHING_TYPE=%u\n",
  		cfg_p->redundancy.switch_position,
			cfg_p->redundancy.switching_mode,
			cfg_p->redundancy.switching_type);
  	bytes_copied += n;

  	n = snprintf(&cfg_ptr[bytes_copied], CFG_MAX_SIZE,
  		"TEN_MHZ_ALARM_ENABLE=%u\n"
			"TEN_MHZ_TRIM0=%u\n"
			"TEN_MHZ_TRIM1=%u\n"
  		"TEN_MHZ_INTERNAL_ENABLE=%u\n"
  		"TEN_MHZ_CALIBRATION=%u\n",
  		cfg_p->ten_MHz.alm_enable,
			cfg_p->ten_MHz.trim[0],
			cfg_p->ten_MHz.trim[1],
			cfg_p->ten_MHz.internal_enable,
			cfg_p->ten_MHz.calibration);
  	bytes_copied += n;

   n = snprintf(&cfg_ptr[bytes_copied], CFG_MAX_SIZE,
  			"SNMP_ENABLE=%d\n"
  			"SNMP_CONTACT=%s\n"
  			"SNMP_LOCATION=%s\n"
  			"SNMP_NAME=%s\n"
  		  "SNMP_TRAP_ENABLE0=%d\n"
  		  "SNMP_TRAP_ENABLE1=%d\n"
  		  "SNMP_TRAP_VERSION=%d\n"
  		  "SNMP_TRAP_TYPE=%d\n"
  		  "SNMP_TRAP_COMMUNITY=%s\n"
  		  "SNMP_TRAP_ADDR0=%d\n"
  		  "SNMP_TRAP_ADDR1=%d\n"
  			"SNMP_ACCESS_COUNT=%d\n"
  			"SNMP_VIEW_COUNT=%d\n"
  			"SNMP_USER_COUNT=%d\n"
  			"SNMP_TRAP_COUNT=%u\n"
  			"SNMP_NOTIFY_FILTER_COUNT=%u\n"
  			"SNMP_COMMUNITY_PUBLIC=%s\n"
  			"SNMP_COMMUNITY_PRIVATE=%s\n",
				cfg_p->snmp.version,\
				cfg_p->snmp.sysContact,\
				cfg_p->snmp.sysLocation,\
				cfg_p->snmp.sysName,\
				cfg_p->snmp.trap.enable[0],\
				cfg_p->snmp.trap.enable[1],\
				cfg_p->snmp.trap.version,\
				cfg_p->snmp.trap.type,\
				cfg_p->snmp.trap.string,\
				cfg_p->snmp.trap.addr[0],\
				cfg_p->snmp.trap.addr[1],\
				cfg_p->snmp.access_count,\
				cfg_p->snmp.view_count,\
				cfg_p->snmp.user_count,\
				cfg_p->snmp.trap_sess_count,\
				cfg_p->snmp.notify_filter_count,\
				cfg_p->snmp.publicStr,\
				cfg_p->snmp.privateStr);
  	bytes_copied += n;

  	for(int i = 0; i < 10; i++){
  		n = snprintf(&cfg_ptr[bytes_copied], CFG_MAX_SIZE,
  		"SNMP_ACCESS_NAME%d=%s\n"
			"SNMP_ACCESS_LEVEL%d=%s\n"
			"SNMP_ACCESS_READVIEW%d=%s\n"
			"SNMP_ACCESS_WRITEVIEW%d=%s\n"
			"SNMP_ACCESS_NOTIFYVIEW%d=%s\n"
			"SNMP_VIEW_NAME%d=%s\n"
			"SNMP_VIEW_FLAG%d=%s\n"
			"SNMP_VIEW_SUBTREE%d=%s\n"
			"SNMP_VIEW_MASK%d=%s\n"
			"SNMP_USER_NAME%d=%s\n"
			"SNMP_USER_LEVEL%d=%s\n"
			"SNMP_USER_AUTH%d=%s\n"
			"SNMP_USER_AUTHPASS%d=%s\n"
			"SNMP_USER_ENCRYPTION%d=%s\n"
			"SNMP_USER_PRIVPASS%d=%s\n"
			"SNMP_USER_GROUP%d=%s\n"
  		"SNMP_PROFILE_NAME%d=%s\n"
			"SNMP_TRAP_USERNAME%d=%s\n"
			"SNMP_TRAP_PRIVILEGE%d=%s\n"
			"SNMP_TRAP_AUTH%d=%s\n"
			"SNMP_TRAP_AUTH_PASS%d=%s\n"
			"SNMP_TRAP_ENCRYPT%d=%s\n"
			"SNMP_TRAP_ENCRYPT_PASS%d=%s\n"
			"SNMP_TRAP_IP%d=%X\n"
			"SNMP_NOTIFY_FILTER_NAME%d=%s\n"
			"SNMP_NOTIFY_FILTER_SUBTREE%d=%s\n"
			"SNMP_NOTIFY_FILTER_FLAG%d=%s\n"
			"SNMP_NOTIFY_FILTER_MASK%d=%s\n",
			i,cfg_p->snmp.access[i].name,\
			i,cfg_p->snmp.access[i].level,\
			i,cfg_p->snmp.access[i].readView,\
			i,cfg_p->snmp.access[i].writeView,\
			i,cfg_p->snmp.access[i].notifyView,\
			i,cfg_p->snmp.view[i].name,\
			i,cfg_p->snmp.view[i].flag,\
			i,cfg_p->snmp.view[i].subtree,\
			i,cfg_p->snmp.view[i].mask,\
			i,cfg_p->snmp.user[i].name,\
			i,cfg_p->snmp.user[i].level,\
			i,cfg_p->snmp.user[i].auth,\
			i,cfg_p->snmp.user[i].authpass,\
			i,cfg_p->snmp.user[i].encryption,\
			i,cfg_p->snmp.user[i].privpass,\
			i,cfg_p->snmp.user[i].group,\
			i,cfg_p->snmp.trap_sess[i].profile,\
			i,cfg_p->snmp.trap_sess[i].username,\
			i,cfg_p->snmp.trap_sess[i].privilege,\
			i,cfg_p->snmp.trap_sess[i].authentication,\
			i,cfg_p->snmp.trap_sess[i].authentication_password,\
			i,cfg_p->snmp.trap_sess[i].encryption,\
			i,cfg_p->snmp.trap_sess[i].encryption_password,\
			i,cfg_p->snmp.trap_sess[i].trap_ip,\
			i,cfg_p->snmp.notify_filter[i].name,\
			i,cfg_p->snmp.notify_filter[i].subtree,\
			i,cfg_p->snmp.notify_filter[i].flag,\
			i,cfg_p->snmp.notify_filter[i].mask);
  		bytes_copied += n;
  	}


  	n = snprintf(&cfg_ptr[bytes_copied], CFG_MAX_SIZE,
  	  			"USER_TITLE=%s\n"
  	        "STAT_LOG_TIME=%hu\n"
  	        "IP_ADDR=%X\n"
  	        "GATEWAY=%X\n"
  	        "NETMASK=%X\n"
  	  			"TIME_ZONE_OFFSET=%d\n"
  	        "NTP_SERVER0=%s\n"
  	    		"NTP_SERVER1=%s\n"
  					"NTP_SERVER_COUNT=%u\n"
  	    		"SSH_ENABLED=%u\n"
  	    		"WEB_ENABLED=%u\n",
  					cfg_p->user_title,\
  					cfg_p->stat_log_time,\
  					cfg_p->ip_addr,\
  					cfg_p->gateway,\
  					cfg_p->netmask,\
  					cfg_p->ntp_server.time_zone_offset,\
  					cfg_p->ntp_server.ip[0],\
  					cfg_p->ntp_server.ip[1],\
  					cfg_p->ntp_server.count,\
  					cfg_p->ssh_enabled,\
  					cfg_p->web_enabled);
  	bytes_copied += n;

    /* Please NOTE that I am not reading "redundancy.position" variable above.
     * which is #ifdef'd in other places. If I put it in the code fails to
     * compile.
     */
    if(n != -1){
      fwrite(cfg_buf , bytes_copied, 1, pFile);
      fflush(pFile);
      fsync(fileno(pFile));
    }else{
    	return -(3);
      fprintf(stderr,"Memory allocation failed, %s\n",__FUNCTION__);
    }
    fclose(pFile);
  }

  return 0;
}

int cfg_save(){
	const char *cfg_file = IBUC_CFG;
	return cfg_fsave(cfg_file, cfg_p);
}

int cfg_load_defaults(void){
  memcpy(cfg_p, (char *)&cfg_default, sizeof(cfg_default));
  return(cfg_save());
}

int main(int argc, char **argv) {
	cfg_map();
	  int r = cfg_read(IBUC_CFG);
	  if(r == -(1)){
	  	fprintf(stderr, "%file inexistent: %s\n",IBUC_CFG);
	  	fprintf(stderr, "Restoring defaults\n");
	  	cfg_load_defaults(); //TODO: What to do if error saving defaults?
	  }

	return 0;
}
