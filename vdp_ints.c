// VDP code for the TI-99/4A by Tursi
// You can copy this file and use it at will if it's useful

#include "vdp.h"
#include <stdint.h>

// Coleco specific init code so that the nmi counts like the TI interrupt
// it also provides the user interrupt hook
// no other TI functionality is simulated (automotion, automusic, quit)
// but if needed, this is where it goes

// synchronization to protect user interrupt hook
static unsigned char intHookReady = 0;

// storage for VDP status byte
volatile unsigned char VDP_STATUS_MIRROR = 0;

// lock variable to prevent NMI from doing anything
// Z80 int is edge triggered, so it won't break us
// 0x80 = interrupt pending, 0x01 - interrupts enabled
volatile unsigned char vdpLimi = 0;		// NO ints by default!

// address of user interupt function
static void (*userint)() = 0;

// interrupt counter
volatile unsigned char VDP_INT_COUNTER = 0;

// This is a little bit racey...
void my_nmi() {
	// do not touch the VDP if interrupts are "disabled".
	if ((vdpLimi&1) == 0) {
		vdpLimi|=0x80;			// flag occurred
		return;
	}

	vdpLimi = 0;				// block further interrupts
	VDP_STATUS_MIRROR = VDPST;
	VDP_INT_COUNTER++;
	if ((intHookReady)&&(0 != userint)) userint();
	vdpLimi = 1;				// now allow them (we know it was previously set - note this loses any that occurred while we ran!)
}


void setUserIntHook(void (*hookfn)()) {
	intHookReady = 0;	// protect the vector
	userint = hookfn;
	intHookReady = 1;	// now it should be safe
}

void clearUserIntHook() {
	intHookReady = 0;
}

// the init code needs this to mute the audio channels
volatile __sfr __at 0xff SOUND;

// called automatically by crt0.S (not in TI version)
void vdpinit() {
	volatile unsigned int x;
	
	// shut off the sound generator - if the cart skips the BIOS screen, this is needed.
	SOUND = 0x9f;
	SOUND = 0xbf;
	SOUND = 0xdf;
	SOUND = 0xff;

	vdpLimi = 0;

	// before touching VDP, a brief delay. This gives time for the F18A to finish
	// initializing before we touch the VDP itself. This is needed on the Coleco if
	// you don't use the BIOS startup delay. This is roughly 200ms.
	x=60000;
	while (++x != 0) { }		// counts till we loop at 65536

	VDP_STATUS_MIRROR = VDPST;	// init and clear any pending interrupt
}