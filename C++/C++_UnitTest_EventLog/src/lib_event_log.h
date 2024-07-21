/*
 * lib_event_log.h
 *
 *  Created on: May 19, 2023
 *      Author: asharma
 */

#ifndef LIB_EVENT_LOG_H_
#define LIB_EVENT_LOG_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* C++ Header */
#include <vector>
#include <fstream>
#include <sstream>
#include <array>

using namespace std;

#define MAX_LENGTH          80

extern const uint8_t EVT_ALM;
extern const uint8_t EVT_CLR;
extern const uint8_t EVT_INF;
extern const uint8_t EVT_DATA;

// event defines
// These also serve as indexes to the cfg string table.
// Do not change the order.
//
#define EVT_A_RX_INPUT_LVL_LO       1
#define EVT_B_RX_INPUT_LVL_LO       2
#define EVT_A_RX_SIMULATED          3
#define EVT_B_RX_SIMULATED          4
#define EVT_REF_10MHZ               5
#define EVT_A_VDC_LO_TH             6
#define EVT_A_VDC_HI_TH             7
#define EVT_B_VDC_LO_TH             8
#define EVT_B_VDC_HI_TH             9
#define EVT_WGUIDE_SW_FAULT         10
#define EVT_A_IDC_LO_TH             11
#define EVT_A_IDC_HI_TH             12
#define EVT_B_IDC_LO_TH             13
#define EVT_B_IDC_HI_TH             14
#define EVT_EMERG_OVERRIDE_ON       15

#define EVT_AHZ                     16
#define EVT_BAM                     17
#define EVT_BSM                     18
#define EVT_BSW                     19
#define EVT_CIA                     20
#define EVT_CIC                     21
#define EVT_CIG                     22
#define EVT_CIHA                    23
#define EVT_CIHD                    24
#define EVT_CIM                     25
#define EVT_CIS                     26
#define EVT_CM1                     27
#define EVT_CM2                     28
#define EVT_CMS                     29
#define EVT_CPS                     30
#define EVT_CTD                     31
#define EVT_CTM                     32
#define EVT_IAL                     33
#define EVT_IAH                     34
#define EVT_IBL                     35
#define EVT_IBH                     36
#define EVT_NTP                     37
#define EVT_RAS                     38
#define EVT_RBS                     39
#define EVT_RAZ                     40
#define EVT_RBZ                     41
#define EVT_RAL                     42
#define EVT_RBL                     43
#define EVT_SHP                     44
#define EVT_SHZ                     45
#define EVT_TZO                     46
#define EVT_VAL                     47
#define EVT_VAH                     48
#define EVT_VBL                     49
#define EVT_VBH                     50
#define EVT_CRT1                    51
#define EVT_CRT2                    52
#define EVT_C13A                    53
#define EVT_C13B                    54
#define EVT_C22A                    55
#define EVT_C22B                    56
#define EVT_C4M                     57
#define EVT_TME                     58

#define EVT_CFG_REST_FAULT          253
#define EVT_REBOOT                  254

#define LOG_ALARM_MS    10000
#define NUM_ALM_ENTRIES 1000

// date_time is the binary coded decimal month, day, year, and hour
// time_alarm is the binary coded decimal minute, and second.
typedef struct alarm_entry_s {
  uint32_t date_time;
  int32_t  data;
  uint8_t  event;
  uint8_t  status;
} alarm_entry_t;

#define ENQUEUE_NEEDED 0

struct event_log_entry_s{
	uint16_t event;
	uint8_t  status;
	uint32_t times_stamp;
	double 	 data;
};

class EventLog{
public:
	EventLog(uint16_t event, uint8_t status, double data);
	EventLog();

	void enqueue();
	int next_entry(char str[MAX_LENGTH]);
	void erase(void);
	size_t get_size(void);
	void stringify(event_log_entry_s entry,char buff[MAX_LENGTH],char seperator);

private:
  struct event_log_entry_s event_log_entry;
	vector<string> event_log_v;

	int write(void);
	int read(void);

};

bool event_log_is_empty(void);
void event_log_enqueue(uint16_t event, uint8_t status, void *data);
int  event_log_rm(void);
int  event_log_next_entry(char **str);
int  event_log_reset_read_ptr(void);
int  event_log_init(void);
int  event_log_memread(void);

bool is_clear_alarm_req(void);
void clear_alarm_req(void);

#endif /* LIB_EVENT_LOG_H_ */
