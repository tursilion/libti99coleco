#include "vdp.h"

void raw_vdpmemset(unsigned char ch, int cnt) {
	while (cnt--) {
		VDPWD = ch;
	}
}
