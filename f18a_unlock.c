// F18A by Tursi - public domain

#include <vdp.h>
#include <f18a.h>

// unlocks the f18a - does not test if successful
void unlock_f18a() {
	VDP_SET_REGISTER(F18A_REG_UNLOCK, F18A_UNLOCK_VALUE);	
	// VR1/57, value 00011100, write once (corrupts VDPR1)
	VDP_SET_REGISTER(F18A_REG_UNLOCK, F18A_UNLOCK_VALUE);	
	// write again (unlocks enhanced mode)
}