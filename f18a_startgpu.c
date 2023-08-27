// F18A - by Tursi - public domain

#include <vdp.h>
#include <f18a.h>

void startgpu_f18a(unsigned int adr) {
    VDP_SET_REGISTER(F18A_REG_GPUCFG, F18A_GPUCFG_OP);  // make sure it's configured to start
    VDP_SET_REGISTER(F18A_REG_GPUMSB, (adr>>8));     // write MSB to register 0x36
    VDP_SET_REGISTER(F18A_REG_GPULSB, (adr&0xff));   // write LSB to register 0x37 (causes start)
}
