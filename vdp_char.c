#include "vdp.h"

void vdpchar(int pAddr, unsigned char ch) {
	VDP_SET_ADDRESS_WRITE(pAddr);
	// it's already so close!
#ifdef PARANOID_TIMING
	__asm
		NOP
	__endasm;
#endif
	VDPWD=ch;
}
