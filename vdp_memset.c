#include "vdp.h"

void vdpmemset(int pAddr, unsigned char ch, int cnt) {
	VDP_SET_ADDRESS_WRITE(pAddr);
	while (cnt--) {
		VDPWD = ch;
	}
}
