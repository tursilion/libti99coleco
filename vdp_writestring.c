#include "vdp.h"

void writestring(unsigned char row, unsigned char col, char *pStr) {
	VDP_SET_ADDRESS_WRITE(gImage+(row<<5)+col);
	while (*pStr) {
		VDPWD = *(pStr++);
	}
}
