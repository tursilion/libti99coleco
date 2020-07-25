#include "vdp.h"

void sprite(unsigned char n, unsigned char ch, unsigned char col, unsigned char r, unsigned char c) {
	unsigned int adr=gSprite+(n<<2);
	VDP_SET_ADDRESS_WRITE(adr);
	// todo: why did I think 1 NOP was enough? IS it?
//	__asm NOP __endasm;
	VDPWD=r;
	VDP_SAFE_DELAY();
//__asm NOP __endasm;
	VDPWD=c;
	VDP_SAFE_DELAY();
//__asm NOP __endasm;
	VDPWD=ch;
	VDP_SAFE_DELAY();
//__asm NOP __endasm;
	VDPWD=col;
}
