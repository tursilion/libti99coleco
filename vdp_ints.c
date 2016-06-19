// VDP code for the TI-99/4A by Tursi
// You can copy this file and use it at will if it's useful

#include "vdp.h"
#include <stdint.h>

// Coleco specific init code so that the nmi counts like the TI interrupt
// it also provides the user interrupt hook
// no other TI functionality is simulated (automotion, automusic, quit)
// but if needed, this is where it goes

// storage for VDP status byte
volatile unsigned char VDP_STATUS_MIRROR = 0;

// lock variable to prevent NMI from doing anything
// Z80 int is edge triggered, so it won't break us
// 0x80 = interrupt pending, 0x01 - interrupts enabled
// (This must be defined in the crt0.s)
//volatile unsigned char vdpLimi = 0;		// NO ints by default!

// address of user interrupt function
static void (*userint)() = 0;

// interrupt counter
volatile unsigned char VDP_INT_COUNTER = 0;

// May be called from true NMI or from VDP_INTERRUPT_ENABLE, depending on
// the flag setting when the true NMI fires.
void my_nmi() {
	// I think we're okay from races. There are only two conditions this is called:
	//
	// VDP_INTERRUPT_ENABLE - detects that vdpLimi&0x80 was set by the interrupt code.
	//						  Calls this code. But the interrupt line is still active,
	//						  so we can't retrigger until it's cleared by the VDPST read
	//						  below. At that time the vdpLimi is zeroed, and so we can't loop.
	//
	// nmi -				  detects that vdpLimi&0x01 is valid, and calls directly.
	//						  Again, the interrupt line is still active.
	//
	// I think the edge cases are covered. Except if the user is manually reading VDPST,
	// then the state of vdpLimi could be out of sync with the real interrupt line, and cause
	// a double call. User apps should only read the mirror variable. Should I enforce that?

	vdpLimi = 0;				// clear the interrupt flags - do this before clearing the VDP
	VDP_STATUS_MIRROR = VDPST;	// release the VDP - we could instantly trigger again, but the vdpLimi is zeroed, so no loop
	VDP_INT_COUNTER++;			// count up the frames

	// the TI is running with ints off, so it won't retrigger in the
	// user code, even if it's slow. Our current process won't either because
	// the vdpLimi is set to 0.
	if (0 != userint) userint();

	// the TI interrupt would normally exit with the ints disabled
	// if it fired, so we will do the same here and not reset it.
}

// NOT atomic! Do NOT call with interrupts enabled!
void setUserIntHook(void (*hookfn)()) {
	userint = hookfn;
}

// NOT atomic! Do NOT call with interrupts enabled!
void clearUserIntHook() {
	userint = 0;
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

	// interrupts off
	vdpLimi = 0;

	// before touching VDP, a brief delay. This gives time for the F18A to finish
	// initializing before we touch the VDP itself. This is needed on the Coleco if
	// you don't use the BIOS startup delay. This is roughly 200ms.
	x=60000;
	while (++x != 0) { }		// counts till we loop at 65536

	VDP_STATUS_MIRROR = VDPST;	// init and clear any pending interrupt
}
