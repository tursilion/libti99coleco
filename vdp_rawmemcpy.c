#include "vdp.h"

void raw_vdpmemcpy(const unsigned char *pSrc, int cnt) {
	while (cnt--) {
		VDPWD=*(pSrc++);
	}
}
