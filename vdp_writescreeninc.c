#include "vdp.h"

void vdpwritescreeninc(int pAddr, unsigned char nStart, int cnt) {
	VDP_SET_ADDRESS_WRITE(pAddr+gImage);
	while (cnt--) {
		VDPWD=nStart++;
	}
}
