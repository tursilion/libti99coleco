#include "vdp.h"

unsigned char gSaveIntCnt;	// console interrupt count byte

// returns non-zero if interrupt already fired (ie: we are late)
unsigned char vdpwaitvint() {
	unsigned char ret = 0;

	// wait for a vertical interrupt to occur (enables interrupts - first call may not wait)
	// to avoid a race on the return condition, we reproduce VDP_INT_ENABLE here

	// set the enable flag
	__asm__("\tpush hl\n\tld hl,#_vdpLimi\n\tset 0,(hl)\n\tpop hl"); 
	
	// check if we missed one despite that
	if (vdpLimi&0x80) {
		my_nmi(); 
		ret = 1;
	}
	
	// wait for the interrupt to run (if we missed it, it ran already and this won't wait)
	{
		// this countdown should be unnecessary. But if the user has
		// messed with the interrupt flags or status register, or
		// just a simple emulator bug like Classic99 seems to have,
		// then continue anyway rather than hanging. My math is weird,
		// so I'm just testing against emulation and then adding 100% fudge
		unsigned int cnt = 2048;	// 1024 seemed to be enough for SSA, so double that
		while (VDP_INT_COUNTER == gSaveIntCnt) { 
			if (--cnt == 0) {
				// we're stuck - clear the VDP and exit
				//VDP_SET_REGISTER(7,3);
				VDP_STATUS_MIRROR = VDPST;
				break;
			}
		} 
	}

	// turn the interrupt flag back off
	VDP_INT_DISABLE; 

	// remember the new value
	gSaveIntCnt=VDP_INT_COUNTER; 

	// back to caller
	return ret;
}

