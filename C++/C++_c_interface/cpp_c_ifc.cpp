#include "cpp_c_ifc.h"
#include <iostream>
#include <fstream>

static const char *channel_A = "out_voltage0_raw";

using namespace std;

int8_t _write(uint16_t value){
  int8_t status = 0;
  ofstream dac_file;

  dac_file.open(channel_A, ios::out | ios::trunc | ios::binary);

  if ( (dac_file.rdstate() & ofstream::failbit ) != 0 )
  	return -1;

  dac_file << value;
  dac_file.close();

  return status;
}
