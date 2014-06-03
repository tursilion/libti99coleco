#include "vdp.h"

void vchar(unsigned char r, unsigned char c, unsigned char ch, int cnt) {
	int pAddr = gImage+(r<<5)+c;
	while (cnt--) {
		vdpchar(pAddr, ch);
		pAddr+=32;
	}
}
