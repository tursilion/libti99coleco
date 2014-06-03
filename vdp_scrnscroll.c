#include "vdp.h"

// tested

void scrn_scroll() {
	int adr;
	unsigned char x[4];		// 4 byte buffer to speed it up

	// hacky, slow, but functional scroll that takes minimal memory
	unsigned char nLine = nTextEnd-nTextRow+1;
	for (adr=gImage+nLine; adr<nTextEnd; adr+=4) {
		vdpmemread(adr, x, 4);
		vdpmemcpy(adr-nLine, x, 4);
	}
	vdpmemset(nTextRow, ' ', nLine);	// clear the last line
}
