#include "vdp.h"

unsigned char gchar(unsigned char r, unsigned char c) {
	VDP_SET_ADDRESS(gImage+(r<<5)+c);
	VDP_SAFE_DELAY();
	return VDPRD;
}
