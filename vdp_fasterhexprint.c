#include "vdp.h"

void faster_hexprint(unsigned char x) {
	unsigned int dat = byte2hex[x];

	VDPWD = dat>>8;
	VDP_SAFE_DELAY();
	VDPWD = dat&0xff;
}
