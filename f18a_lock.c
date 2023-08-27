// F18A - by Tursi - public domain

#include <vdp.h>
#include <f18a.h>

// just lock the F18A (preserves all settings)
void lock_f18a() {
    VDP_SET_REGISTER(F18A_REG_UNLOCK, 0x00);	
    // VR1/57, value 00000000 (corrupts VDPR1 if already locked)
}
