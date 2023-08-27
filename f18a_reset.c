// F18A reset - by Tursi - public domain

#include <vdp.h>
#include <f18a.h>

// default F18 palette values
const unsigned int DEFAULT_PALETTE[] = {
	0x0000,0x0000,0x02C3,0x05D6,0x054F,0x076F,0x0D54,0x04EF,
	0x0F54,0x0F76,0x0DC3,0x0ED6,0x02B2,0x0C5C,0x0CCC,0x0FFF
};

// reset F18A if unlocked (1.6 or later)
void reset_f18a() {
	// reset the F18A to defaults (except palette, requires 1.6)
	VDP_SET_REGISTER(F18A_REG_PRIORITY, F18A_PRI_RESET); 	
	// VR2/50, value 10000000
	// reset and lock F18A, or corrupt R2

    // make sure the display is off and dark
    VDP_SET_REGISTER(VDP_REG_COL, 0);         // do this first to minimize the time for a flash
    VDP_SET_REGISTER(VDP_REG_MODE1, VDP_MODE1_16K);
    
    loadpal_f18a(DEFAULT_PALETTE, 16);
}
