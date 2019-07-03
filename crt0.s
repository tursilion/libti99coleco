; crt0.s for Colecovision cart
; todo: need to implement spinnerInt stub or wrapper in the C code...

	.module crt0
	.globl _main
	.globl _vdpinit
	.globl _my_nmi
	.globl _vdpLimi
	.globl _spinnerInt

	.area _HEADER(ABS)
	.org 0x8000

	.db 0x55, 0xaa			; Title screen and 12 second delay - swap 0x55 and 0xaa not to skip it.
	.dw 0								; sprite table stuff? - rarely used by the BIOS as a pointer
	.dw 0								; unknown - rarely used as a pointer to a single byte of RAM by the BIOS.
	.dw 0								; unknown - frequently used in BIOS routines as a pointer to a memory location (data is both written to and read from there, at least 124 bytes are used - maybe this is where the bios routines store most of their data, though with the common value of 0 this would be in the BIOS ROM itself - strange).
	.dw 0								; unknown - rarely used as a pointer to a single byte of RAM by the BIOS.
	.dw start						; where to start execution of program.
	ei		; RST 0x08
	reti					
	ei		; RST 0x10
	reti
	ei		; RST 0x18
	reti
	ei		; RST 0x20
	reti
	ei		; RST 0x28
	reti
	ei		; RST 0x30
	reti

	ei		; RST 0x38 - spinner interrupt
	reti
;	jp spinner	; RST 0x38 - spinner interrupt (one or the other - padding is important)
;	nop
	
	jp nmi		; NMI
	nop
	.ascii " / / NOT"

start:
	; clear interrupt flag right off
	ld hl,#_vdpLimi
	ld (hl),#0
	; clear RAM before starting
	ld hl,#0x7000			; set copy source
	ld de,#0x7001			; set copy dest
	ld bc,#0x03ff			; set bytes to copy (1 less than size)
	ld (hl),#0				; set initial value (this gets copied through)
	ldir					; do it
	
	ld  sp, #0x7400			; Set stack pointer directly above top of memory.
	ld	bc,#0xFFFE			; switch in code bank
   	ld	a,(bc)				; note that this does NOT set the local pBank variable, user code still must do that!
	call gsinit				; Initialize global variables. (always at end of code bank, so need above bank switch)
	call _vdpinit			; Initialize something or other ;)
	call _main
	rst 0x0					; Restart when main() returns.

	;; Ordering of segments for the linker - copied from sdcc crt0.s
	.area _HOME
	.area _CODE
	.area _INITIALIZER
	.area   _GSINIT
	.area   _GSFINAL
        
	.area _DATA
	.area _BSS
	.area _HEAP

	.area _BSS
_vdpLimi:		; 0x80 - interrupt set, other bits used by library
	.ds 1

    .area _CODE
nmi:
; all we do is set the MSB of _vdpLimi, and then check
; if the LSB is set. If so, we call user code now. if
; not, the library will deal with it when enabled.
	push af					; save flags (none affected, but play safe!)
	push hl
	
	ld hl,#_vdpLimi
	bit 0,(hl)				; check LSb (enable)
	jp z,notokay
	
; okay, full on call, save off the (other) regs
	push bc
	push de
	;push ix ; saved by callee
	push iy
	call _my_nmi			; call the lib version
	pop iy
	;pop ix
	pop de
	pop bc	
	jp clrup				

notokay:
	set 7,(hl)				; set MSb (flag)

clrup:
	pop hl					
	pop af
	retn
	
spinner:
; Spinner handler - call out to C code
	push af					; save flags
	push hl
	push bc
	push de
	;push ix ; saved by callee
	push iy
	
; to prevent the NMI from blowing us up, we'll run two paths
; this is still slightly racey if the user is enabling/disabling
; the NMI, so don't do that if you are using the spinner.
	ld hl,#_vdpLimi
	bit 0,(hl)				; small race here...?
	jp z,spin_nmi_blocked	; if it's blocked, just do the easy path

; NMI was enabled, so to reduce conflict, disable them now
	res 0,(hl)
	
; now call the C code - it should be safe
	call _spinnerInt		; NMI is already off, so just call the code version
	
; now before we re-enable NMIs, we need to check if they already happened
	ld hl,#_vdpLimi
	set 0,(hl)				; turn them back on first
	bit 7,(hl)				; did we miss one?
	jp z,spin_cleanup		; there was no NMI missed, then we are done
	
; else, just call the NMI handler before we go
	call _my_nmi
	jp spin_cleanup

; we come this way if the NMI wasn't enabled to begin with	
spin_nmi_blocked:	
	call _spinnerInt		; NMI is already off, so just call the code version
	
; and we come this way in both cases	
spin_cleanup:
	pop iy
	;pop ix
	pop de
	pop bc	
	pop hl					
	pop af
	ei
	reti

; soft function to be overridden if that's possible
_spinnerInt:
	ret

	.area _GSINIT
gsinit::
	.area _GSFINAL
	ret

