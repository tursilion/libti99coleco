#include "vdp.h"

void sprite(unsigned char n, unsigned char ch, unsigned char col, unsigned char r, unsigned char c) {
	unsigned int adr=gSprite+(n<<2);
	VDP_SET_ADDRESS_WRITE(adr);
	__asm NOP __endasm;
	VDPWD=r;
	__asm NOP __endasm;
	VDPWD=c;
	__asm NOP __endasm;
	VDPWD=ch;
	__asm NOP __endasm;
	VDPWD=col;
}
