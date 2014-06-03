#include "vdp.h"

void hchar(unsigned char r, unsigned char c, unsigned char ch, int cnt) {
	vdpmemset(gImage+(r<<5)+c, ch, cnt);
}
