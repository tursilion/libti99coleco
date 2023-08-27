// F18A - by Tursi - public domain

#include <vdp.h>
#include <f18a.h>

// load an F18A palette from ptr (16-bit words, little endian)
// data format is 12-bit 0RGB color.
void loadpal_f18a(const unsigned int *ptr, unsigned char cnt) {
	VDP_SET_REGISTER(F18A_REG_DPM, F18A_DPM_ENABLE|F18A_DPM_INC);
	// Reg 47, value: 1100 0000, DPM = 1, AUTO INC = 1, palreg 0.       

	while (cnt-- > 0) {
		VDPWD = ptr[0]>>8;
		VDPWD = ptr[0]&0xff;
		ptr++;
	}

	VDP_SET_REGISTER(F18A_REG_DPM, 0x00);	// Turn off the DPM mode
}
