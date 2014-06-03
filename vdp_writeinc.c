#include "vdp.h"

// this one writes an incrementing series of bytes
// normally you'd use this to set up bitmap or multicolor
void vdpwriteinc(int pAddr, unsigned char nStart, int cnt) {
	VDP_SET_ADDRESS_WRITE(pAddr);
	while (cnt--) {
		VDPWD=nStart++;
	}
}
