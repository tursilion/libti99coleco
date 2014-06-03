#include "vdp.h"

void vdpscreenchar(int pAddr, unsigned char ch) {
	VDP_SET_ADDRESS_WRITE(pAddr+gImage);
	// it's very close!
	__asm NOP __endasm;
	VDPWD=ch;
}
