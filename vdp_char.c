#include "vdp.h"

void vdpchar(int pAddr, unsigned char ch) {
	VDP_SET_ADDRESS_WRITE(pAddr);
	// it's already so close!
	__asm
		NOP
	__endasm;
	VDPWD=ch;
}
