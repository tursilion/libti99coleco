#include "vdp.h"
#include "halt.h"

// tested

int main() {
	unsigned char x = set_text();
	unsigned char col = (COLOR_GRAY<<4)|COLOR_DKBLUE;
	// TODO: test - do we need delay between register writes?
	VDP_SAFE_DELAY();
	VDP_SET_REGISTER(VDP_REG_COL, col);
	VDP_SAFE_DELAY();
	VDP_SET_REGISTER(VDP_REG_MODE1, x);
	charsetlc();
	putstring("hello world!\n");
	vdpwaitvint();				// to make sure the first wait is not late

	while (1) {
		if (vdpwaitvint()) {	// wait for an int to occur
			// it was late - change colors - should not see this happen
			++col;
			VDP_SET_REGISTER(VDP_REG_COL, col);
		}
		VDP_SET_ADDRESS_WRITE(14);	// top center
		faster_hexprint(VDP_INT_COUNTER);
	}

	return 0;
}

