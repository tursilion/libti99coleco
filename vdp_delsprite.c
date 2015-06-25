#include "vdp.h"

// just pushes it off screen, but not disabled
void delsprite(unsigned char n) {
	unsigned int adr=gSprite+(n<<2);
	VDP_SET_ADDRESS_WRITE(adr);
#ifdef PARANOID_TIMING
	VDP_SAFE_DELAY();
#endif
	VDPWD=192;
#ifndef PARANOID_TIMING
	// this is where the VDP is really working
#endif
}
