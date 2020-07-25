#include "vdp.h"

void vdpscreenchar(int pAddr, unsigned char ch) {
	VDP_SET_ADDRESS_WRITE(pAddr+gImage);
#ifdef PARANOID_TIMING
	// it's very close!
	__asm NOP __endasm;
#endif
	VDPWD=ch;
}
