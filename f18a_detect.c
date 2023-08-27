// F18A - by Tursi - public domain

#include <vdp.h>
#include <f18a.h>

const unsigned char GPUTEST[] = {
	0x02,0x00,0x12,0x34,    //		LI R0,>1234
	0xC8,0x00,0x3F,0x10,    //		MOV R0,@>3F10
	0x03,0x40				//	    IDLE
};

// Corrupts R1 if not F18A. Also corrupts a few VDP bytes at 0x3F00
unsigned char detect_f18a() {
    // This is not the way Matthew intended, but it's a tad easier
    // We just check if the GPU is present by having it run a short
    // program, instead of mucking around with status bits.
    unsigned int val;

    reset_f18a();
    unlock_f18a();
    
    // copy the GPU program to 0x3f00
    vdpmemcpy(0x3f00, GPUTEST, 10);
    
    // and zero out the test address
    vdpmemset(0x3f10, 0, 2);

    // start the GPU program
    startgpu_f18a(0x3f00);
    
    // honestly, this is overkill. It's likely done before we can execute the next instruction
    VDP_SAFE_DELAY();
    
    // note if it IS an F18A, that delays are not needed. If it's not, well, it won't be right anyway
    VDP_SET_ADDRESS(0x3f10);
    val=(VDPRD)<<8;         // get MSB
    val|=(VDPRD);           // get LSB
    
    if (val == 0x1234) {
        return 1;
    } else {
        return 0;
    }
}
