#include "vdp.h"

// just pushes it off screen, but not disabled
void delsprite(unsigned char n) {
	unsigned int adr=gSprite+(n<<2);
	VDP_SET_ADDRESS_WRITE(adr);
	VDP_SAFE_DELAY();
	VDPWD=192;
}
