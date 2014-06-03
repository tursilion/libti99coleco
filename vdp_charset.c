// VDP code for the TI-99/4A by Tursi
// You can copy this file and use it at will if it's useful

#include "vdp.h"

#define COLECO_FONT (unsigned char*)0x15A3

void charset() {
	vdpmemcpy(gPattern+0x100, COLECO_FONT, 96*8);	// the character set (not shifted)
}

