#include "vdp.h"
#include "halt.h"

// tested

int main() {
	unsigned char x = set_text();
	VDP_SAFE_DELAY();
	VDP_SET_REGISTER(VDP_REG_COL, (COLOR_GRAY<<4)|COLOR_DKBLUE);
	VDP_SAFE_DELAY();
	VDP_SET_REGISTER(VDP_REG_MODE1, x);
	charsetlc();
	putstring("hello world!\n");
	halt();

	return 0;
}

