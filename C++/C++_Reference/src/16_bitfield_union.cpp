#include <stdio.h>
#include <stdint.h>

union suppression_mask_u{
	struct suppression_mask_s{
		uint16_t configurable:     10;// Starts here
		uint16_t non_configurable: 6; // Ends here
  }part;
  uint16_t full;
}mask;


uint16_t cfg_get_suppression_mask(void){
	printf("mask=%x\n",mask.full);
	return mask.full;
}

void cfg_set_suppression_mask(uint16_t new_mask){
	mask.part.configurable     = new_mask;
	mask.part.non_configurable = 0;
	printf("mask=%x\n",mask.part.configurable);
}

int main(void){
	cfg_set_suppression_mask(0x03ff);
	cfg_get_suppression_mask();
	return 0;
}
