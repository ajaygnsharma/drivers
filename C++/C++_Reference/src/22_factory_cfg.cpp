/*
 * 22_factory_cfg.cpp
 *
 *  Created on: Aug 1, 2023
 *      Author: asharma
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define VAR_PASS        0
#define VAR_PWD         1
#define VAR_RANGE       2
#define VAR_INVALID     3

/* 51 is 50(valid chars) + 1'\0' */
enum IBUC_e{
	MAX_LEN    					 = 51,
};

enum mfg_e{
	MODEL_NAME_FIX_LENGTH    = 25,
	MODEL_STRING_LENGTH      = 6,
	SERIAL_NUMBER_MIN_LENGTH = 4,
	SERIAL_NUMBER_MAX_LENGTH = 32,
	PRIVATE_LABEL_MAX_LENGTH = 32,
	HW_REVISION_MIN_LENGTH   = 4,
	HW_REVISION_MAX_LENGTH   = 32,
	TRIM_10MHZ_MAX           = 4095,
	RATED_POWER_FUDGE_HIGH   = 100,
	RATED_POWER_FUDGE_LOW    = 2000,
};

enum model_number_offset_e{
	PAD															= 0,
	FAMILY													= 2,
	FREQ_BAND												= 3,
	DASH1														= 9,
	SPECTRAL_INVERSION_AND_10MHZ_REF= 10,
	OPERATING_VOLTAGE								= 12,
	POWER_CLASS											= 13, /* Or Power level */
	TECHNOLOGY_AND_LINEARIZER				= 17,
	DASH2														= 20,
	SPECIAL_OPTION									= 21,
};

#define MFG_CYBER_HARDENED_PRFX        "IBB"
#define MFG_CYBER_HARDENED_IBUC3_PRFX  "IBF"

enum power_supply_type_e{
	AC = 0,
	DC = 1
};

enum service_status_e{
	DISABLED= 0,
	ENABLED = 1
};

typedef struct freq_table_s {
   char      lo_id;
   uint16_t  rfFreqMin ;     // RF Frequency minimum
   uint16_t  rfFreqMax ;     // RF Frequency Maximum
   uint16_t  lo_f_inv ;      // LO frequency when spectrum is inverted.
   uint16_t  lo_f_non_inv ;  // LO Frequency when spectrum is NOT inverted.
   char      bandRange[7];
   char      bandAbbrevation[4];
   char      bandDescription[17];
} freq_table_t;

freq_table_t tx_freq_table[] = {
	// lof_id,min,   max,   lo_inv,lo_ninv, freqRange,Abbr,  Description
		{ 3,    5850,  6425,  7375,  4900,    "058064", "CS" , "C-Band Standard" }, // "3 Standard C",
		{ 4,    6725,  7025,  8175,  5760,    "067070", "CI" , "C-Band Insat"    }, // "4 Insat",
		{ 5,    13750, 14250, 15200, 12800,   "137142", "KuE", "Ku-Band Extended"}, // "5 Extended Ku",
		{ 6,    14000, 14500, 15450, 13050,   "140145", "KuS", "Ku-Band Standard"}, // "6 Standard Ku",
		{ 7,    7900,  8400,  9350,  6950,    "079084", "X"  , "X-Band"          }, // "7 X-band",
		{ 8,    13750, 14500, 15450, 12800,   "137145", "KuF", "Ku-Band Full"    }, // "8 Full Ku",
		{ 9,    5850,  6650,  7600,  4900,    "058066", "CE",  "C-Band Extended" }, // "9 Extended C",
		{10,    5850,  6725,  7700,  4900,    "058067", "CF" , "C-Band Full"     }, // "10 Full C",
		{11,    12750, 13250, 14200, 11800,   "127132", "KuL", "Ku-Band Low"     }, // "11 Ku 13 GHz",
		{12,    6425,  6725,  7700,  5300,    "064067", "CP" , "C-Band  Palapa"  }, // "12 Palapa",
		{13,    14850, 15150, 16200, 13800,   "148151", "KuH", "Ku-band High"    }, // "13 High Ku",
		{14,    17300, 18100, 19050, 16350,   "173181", "DBS", "DBS-Band 1"      }, // "14 DBS Low band",
		{15,    18100, 18400, 19550, 16950,   "181184", "DBS", "DBS-Band 2"      }, // "15 DBS High band",
		{16,    29500, 30000, 31000, 28500,   "295300", "Ka",  "Ka-Band 1"       }, // "16 Ka Band 1",
		{17,    30000, 31000, 32000, 29000,   "300310", "Ka",  "Ka-Band 2"       }, // "17 Ka Band 2",
		{18,    12800, 13300, 14300, 11800,   "128133", "Ku",  "Ku-Band 4"       }, // "18 Ku Band 4",
		{19,    5700,  6425,  7375,  4775,    "057064", "C6",  "C-Band 6"        }, // "19 C Band 6",
		{20,    29000, 30000, 31000, 28000,   "290300", "Ka",  "Ka-Band 3"       }, // "20 Ka Band 3",
		{21,    30500, 31000, 32000, 29500,   "305310", "Ka",  "Ka-Band 4"       }, // "21 Ka Band 4",
		{22,    5090,  5250,     0,  4250,    "050052", "GS",  "C-Band GS"       }, // "22 C Band GlobalStar",
		{ 0,    0,     0,     0,     0,       "000000", ""   , ""                }  // "default, Forces fault
};

bool streq(const char *src, const char *dest){
	if(strcmp(src, dest) == 0){
		return true;
	}else{
		return false;
	}
}

bool streq_n(const char *src, const char *dest, size_t n){
	if(strncmp(src, dest, n) == 0){
		return true;
	}else{
		return false;
	}
}

/* This data once set is not to be deleted. */
struct fcfg_s{
  char serial_number[MAX_LEN]; // Factory <- CC2
  char model_number[MAX_LEN];  // Factory <- model number OK- CC1
  char private_label[MAX_LEN]; // Factory <- CPL
  char hardware_revision[MAX_LEN];// Factory <- CC3
  uint16_t rated_power_mB;// Factory <- CC4
  uint16_t power_supply_type;// Factory <- model number OK- CC1
  float   volt_multiplier;   // Factory <- model number OK- CC1
  float   volt_slope;        // Factory <- model number OK- CC1
  float   volt_intercept; // Factory <- model number OK- CC1
  float   amp_multiplier;// Factory <- model number OK- CC1
  float   amp_slope;// Factory <- model number OK - CC1
  float   amp_intercept;// Factory <- model number OK- CC1
  uint16_t ibr_power_class;// Factory <- model number OK- CC1
  uint16_t current_low;// Factory FUTURE
  uint16_t current_high;// Factory FUTURE
  int16_t voltage_low; // Factory <- model number OK- CC1
  int16_t voltage_high;// Factory <- model number OK- CC1
  uint8_t internal_10MHz;// Factory <- model number- CC1
  int16_t base_overdrive_mB;// Factory <- model number OK- CC1
  uint16_t freq_index;   // Factory <- model number OK- CC1
  uint16_t lo_invert;    // Factory <- model number OK- CC1
  uint16_t trim_10MHz; // ?
  int16_t  input_min_mB;
  int16_t  input_max_mB;
};

static struct fcfg_s *fcfg_p;


static struct fcfg_s factory_cfg_default = {
    "Undefined", //serial_number
    "Undefined", //model_number       "
    "Undefined", //private_label
    "Undefined", //hardware_revision
    0, //rated_power_mB
    AC, //power_supply_type
    0, //volt_multiplier
    0, //volt_slope
    0, //volt_intercept
    0, //amp_multiplier
    0, //amp_slope
    0, //amp_intercept
    0, //ibr_power_class
    0, //current_low
    0, //current_high
    0, //voltage_low
    0, //voltage_high
    0, //internal_10MHz
    0, //base_overdrive_mB
    0, //freq_index
    0, //lo_invert
    2048, //trim_10MHz
	-6000,
	-1500,
};

char     fcfg_get_lo_id(void){            return tx_freq_table[fcfg_p->freq_index].lo_id; }
uint16_t fcfg_get_ibr_power_class(void){  return fcfg_p->ibr_power_class; }
char    *fcfg_get_model_number(void){     return fcfg_p->model_number; }

uint16_t ibuc_verify_modelname(const char *name){
	uint32_t len = strlen(name);
	if(len != MODEL_NAME_FIX_LENGTH)
		return VAR_RANGE;
	else
		return VAR_PASS;
}

uint16_t ibuc_verify_modelname_dashes(const char *name){
	if(name[DASH1] != '-' || name[DASH2] != '-')
		return VAR_RANGE;
	else
		return VAR_PASS;
}

/* Cyber-hardened unit only supports this prefix, nothing else */
uint16_t ibuc_verify_model_prefix(const char *name){
	if( (!streq_n(name, MFG_CYBER_HARDENED_PRFX, 3)) &&
			(!streq_n(name, MFG_CYBER_HARDENED_IBUC3_PRFX, 3)) ){
		return VAR_RANGE;
	}else{
		return VAR_PASS;
	}
}

uint16_t ibuc_set_operating_electrical_spec(const char offset, struct fcfg_s *fcfg){
	if(offset == 'A' || offset == 'B'){
		fcfg->power_supply_type = AC;
		fcfg->amp_slope         = 0.002442;
		fcfg->amp_intercept     = 0;
		fcfg->amp_multiplier    = 1;

		fcfg->voltage_low       = 100;
		fcfg->voltage_high      = 240;

		/* Set Amp multiplier and voltage low or high */
		char     lo_id               = fcfg_get_lo_id();
		uint16_t watts               = fcfg_get_ibr_power_class();
		char    *model_number        = fcfg_get_model_number();
		char     tech_and_linearizer = model_number[TECHNOLOGY_AND_LINEARIZER];

		switch(lo_id){
		case 3:
		case 4:
		case 9:
		case 10:
		case 12: /* Higher power units result in higher current consumption */
			if(watts == 400)
				fcfg->amp_multiplier = 2;
			if(watts == 350)
				fcfg->amp_multiplier = 2;
			break;

		case 6:
		case 8:
		case 11:
			if(watts > 201)
				fcfg->amp_multiplier = 2;
			break;
		}


		if(watts < 150) {
			fcfg->voltage_low  = 100;
			fcfg->voltage_high = 240;
		} else {
			fcfg->voltage_low  = 200;
			fcfg->voltage_high = 240;
		}
	}else if(offset == 'C'){
		fcfg->power_supply_type = AC;
		fcfg->amp_slope         = 0.002442;
		fcfg->amp_intercept     = 0;
		fcfg->amp_multiplier    = 2;
		//fcfg->voltage_low       = 100; TODO?
		//fcfg->voltage_high      = 240; TODO?
	}else if(offset == 'E'){
			fcfg->power_supply_type = AC;
			fcfg->amp_slope         = 0.002442;
			fcfg->amp_intercept     = 0;
			fcfg->amp_multiplier    = 2;
			fcfg->voltage_low       = 100;
			fcfg->voltage_high      = 240;
	}else{
		/* Rest are DC units */
		fcfg->power_supply_type = DC;
		switch(offset){
		case '2':
		case '3':
		case '8': //24V
			fcfg->voltage_low  = 20;
			fcfg->voltage_high = 28;
			break;
		case '4': //48V Coax
			fcfg->voltage_low  = 37;
			fcfg->voltage_high = 60;
			break;
		case '5': //48V No Coax
		{
			fcfg->voltage_low  = 37;
			fcfg->voltage_high = 60;

			uint16_t watts = fcfg_get_ibr_power_class();
			switch(tx_freq_table[fcfg->freq_index].lo_id) {
				// C Band.
				case 3:
				case 4:
				case 9:
				case 10:
				case 12:
				case 19:
					if(watts >= 100)
						fcfg->voltage_low = 42;
					break;
				// Ku Band
				case 5:
				case 6:
				case 8:
				case 11:
				case 13:
				case 18:
					if(watts >= 60)
						fcfg->voltage_low = 42;
					break;
				// X Band
				case 7:
					if(watts >= 80)
						fcfg->voltage_low = 42;
					break;
				default:
					break;
			}
		}
			break;

		case '6':
			fcfg->voltage_low  = 18;
			fcfg->voltage_high = 75;
			break;

		case '7':
			fcfg->voltage_low  = 37;
			fcfg->voltage_high = 60;
			break;
		default:
			return VAR_RANGE;
		}

		if(offset ==  '6' || offset ==  '7'){
			fcfg->volt_slope = 2.03538E-2;
		}else{
			fcfg->volt_slope = 1.46484e-2;
		}
		fcfg->volt_intercept = 0;
		fcfg->volt_multiplier= 1;

		/* Start with this */
		char *model_number = fcfg_get_model_number();
		if(!strncmp(&model_number[SPECIAL_OPTION], "0503", 4))
			fcfg->amp_slope  = 3.2274e-3;
		else
			fcfg->amp_slope  = 3.05176e-3;

		fcfg->amp_intercept  = 0;
		fcfg->amp_multiplier = 1;

		// Configure Amperage Options
		// Set the amperage multiplier.
		uint16_t watts = fcfg_get_ibr_power_class();
		switch(offset) {
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
			switch(tx_freq_table[fcfg->freq_index].lo_id) {
			// C Band.
			case 3:
			case 4:
			case 9:
			case 10:
			case 12:
			case 19:
				if(watts >= 100)
					fcfg->amp_multiplier = 2;

				if(watts <= 15)
					fcfg->amp_multiplier = 0.30303030;
				break;
			// Ku Band
			case 5:
			case 6:
			case 8:
			case 11:
			case 13:
			case 18:
				if(watts >= 60)
					fcfg->amp_multiplier = 2;

				if(watts <= 8)
					fcfg->amp_multiplier = 0.30303030;
				break;
			// X Band
			case 7:
				if(watts >= 100)
					fcfg->amp_multiplier = 2;

				if(watts <= 11)
					fcfg->amp_multiplier = 0.30303030;
				break;
			// DBS Band
			case 14:
			case 15:
				if(watts <= 6)
					fcfg->amp_multiplier = 0.30303030;
				break;
			// Ka Band
			case 16:
			case 17:
			case 20:
			case 21:
				if(watts <= 46)
						fcfg->amp_multiplier = 0.30303030;
				break;
			default:
				break;
			}
			break;
		default:
			return VAR_RANGE;
			break;
		}

		if(!strncmp(&model_number[SPECIAL_OPTION], "0402", 4))
			fcfg->amp_multiplier = 0.30303030;
	}
	return VAR_PASS;
}

/* Once frequency band(index) if set, all frequency table values are referenced
 * from it.
 */
uint16_t ibuc_set_freq_band(const char *modelname, uint16_t *freq_idx){
	uint16_t idx = 0;
	for(idx=0; tx_freq_table[idx].lo_id != 0; idx++) {
		if( streq_n(modelname, tx_freq_table[idx].bandRange, MODEL_STRING_LENGTH) ) {
			*freq_idx = idx;
			break;
		}
	}
	if(tx_freq_table[*freq_idx].lo_id == 0){ /* Can't find a valid table entry */
		return VAR_RANGE;
	}else{
		return VAR_PASS;
	}
}

uint16_t ibuc_set_power_class(const char *modename, uint16_t *power_class){
	uint16_t watts;
	char temp[3];

	temp[0] = modename[0];
	temp[1] = modename[1];
	temp[2] = modename[2];

	watts = (uint16_t)atoi(temp);

	switch(watts) {
	case 4:
		*power_class = 2;
		break;
	case 5:
	case 6:
		*power_class = 3;
		break;
	case 8:
		*power_class = 4;
		break;
	case 10:
	case 11:
		*power_class = 5;
		break;
	case 12:
		*power_class = 14;
		break;
	case 15:
		*power_class = 21;
		break;
	case 16:
	case 17:
		*power_class = 6;
		break;
	case 20:
	case 21:
		*power_class = 7;
		break;
	case 25:
	case 26:
		*power_class = 8;
		break;
	case 30:
		*power_class = 11;
		break;
	case 40:
	case 41:
		*power_class = 9;
		break;
	case 45:
	case 46:
		*power_class = 23;
		break;
	case 50:
	case 51:
	case 52:
		*power_class = 15;
		break;
	case 60:
	case 62:
		*power_class = 10;
		break;
	case 80:
	case 81:
	case 82:
		*power_class = 12;
		break;
	case 100:
	case 101:
		*power_class = 13;
		break;
	case 125:
	case 126:
		*power_class = 16;
		break;
	case 150:
	case 151:
		*power_class = 17;
		break;
	case 175:
		*power_class = 19;
		break;
	case 200:
	case 201:
		*power_class = 18;
		break;
	case 225:
	case 226:
		*power_class = 24;
		break;
	case 300:
	case 301:
		*power_class = 22;
		break;
	case 400:
	case 401:
		*power_class = 20;
		break;
	case 501:
		*power_class = 25;
		break;
	default:
		return VAR_RANGE;
		break;
	}
	return VAR_PASS;
}

uint16_t ibuc_set_lo_invert(char val, uint16_t *lo_invert){
	switch(val){
	case '0':
		*lo_invert = DISABLED;
		break;
	case '1':
		*lo_invert = ENABLED;
		break;
	case '2':
		*lo_invert = DISABLED;
		break;
	case '3':
		*lo_invert = ENABLED;
		break;
	default:
		return VAR_RANGE;
	}
	return VAR_PASS;
}

uint16_t ibuc_set_10MHz_ref(char val, uint8_t *internal_10MHz_ref){
	switch(val){
	case '0':
		*internal_10MHz_ref = DISABLED;
		break;
	case '1':
		*internal_10MHz_ref = DISABLED;
		break;
	case '2':
		*internal_10MHz_ref = ENABLED;
		break;
	case '3':
		*internal_10MHz_ref = ENABLED;
		break;
	default:
		return VAR_RANGE;
	}
	return VAR_PASS;
}

/*
 * CC1 - setting
 * First cfg_set_factory_info() is called to set various parameters based off
 * model number.
 */

uint16_t fcfg_set_factory_info(const char *model_str){
  if(ibuc_verify_modelname(model_str) != VAR_PASS)
  	return VAR_RANGE;

  if(ibuc_verify_model_prefix(model_str) != VAR_PASS)
  	return VAR_RANGE;

  if(ibuc_verify_modelname_dashes(model_str) != VAR_PASS)
  	return VAR_RANGE;

  if((ibuc_set_freq_band(&model_str[FREQ_BAND], &fcfg_p->freq_index)) != VAR_PASS)
  	return VAR_RANGE;

  if(ibuc_set_power_class(&model_str[POWER_CLASS], &fcfg_p->ibr_power_class)) return VAR_RANGE;

  if(ibuc_set_lo_invert(model_str[SPECTRAL_INVERSION_AND_10MHZ_REF], &fcfg_p->lo_invert))
  	return VAR_RANGE;

  if(ibuc_set_10MHz_ref(model_str[SPECTRAL_INVERSION_AND_10MHZ_REF], &fcfg_p->internal_10MHz))
    return VAR_RANGE;

  if(ibuc_set_operating_electrical_spec(model_str[OPERATING_VOLTAGE], fcfg_p) != VAR_PASS){
  	return VAR_RANGE;
  }

  return VAR_PASS;
}

int main(void){
	fcfg_p = &factory_cfg_default;

	int r = fcfg_set_factory_info("IBF140145-0N4016TSWW-0000");
	printf("r=%d\n",r);

	return 0;
}


