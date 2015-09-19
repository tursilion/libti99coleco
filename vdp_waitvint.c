#include "vdp.h"

unsigned char gSaveIntCnt;	// console interrupt count byte

// returns non-zero if interrupt already fired (ie: we are late)
unsigned char vdpwaitvint() {
	unsigned char ret = vdpLimi & 0x80;

	// wait for a vertical interrupt to occur (enables interrupts - first call may not wait)
	VDP_INT_ENABLE; 
	while (VDP_INT_COUNTER == gSaveIntCnt) { } 
	gSaveIntCnt=VDP_INT_COUNTER; 
	VDP_INT_DISABLE; 

	return ret;
}

