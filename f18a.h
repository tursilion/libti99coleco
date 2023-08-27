// F18A header for the by Tursi aka Mike Brent
// This code and library released into the Public Domain
// You can copy this file and use it at will ;)

#ifndef F18A_H
#define F18A_H

#include "vdp.h"

// all methods except detect_f18a() and unlock_f18a() require the F18A to be unlocked!
// recommended to use detect_f18a() in preference to unlock_f18a() blindly.

// return 1 if f18a is installed, else 0
// corrupts VDP Register 1 if no F18A
unsigned char detect_f18a();

// reset the F18A back to stock 9918A mode
void reset_f18a();

// unlock the F18A - corrupts VDP Register 1 if no F18A
void unlock_f18a();

// re-lock the F18A (1.6 or later)
// do this after the reset, if desired
void lock_f18a();

// load a palette into the F18A
// data format is 12-bit 0RGB (4 bits each gun)
void loadpal_f18a(const unsigned int *ptr, unsigned char cnt);

// start executing TMS9900 GPU code from VDP address adr
void startgpu_f18a(unsigned int adr);

// extra F18A registers
#define F18A_REG_SIT2           (unsigned char)0x0A	 // SIT2 - this value times >0400
#define F18A_REG_CT2            (unsigned char)0x0B     // CT2 - this value times >0040
#define F18A_REG_STATUS         (unsigned char)0x0F     // Status register to read
#define F18A_REG_HINT           (unsigned char)0x13     // Horizontal interrupt line
#define F18A_REG_EXTRAPAL       (unsigned char)0x18     // extra palette select bits
#define F18A_REG_HTO2           (unsigned char)0x19     // horizontal scroll offset 2
#define F18A_REG_VTO2           (unsigned char)0x1a     // vertical scroll offset 2
#define F18A_REG_HTO1           (unsigned char)0x1b     // horizontal scroll offset 1
#define F18A_REG_VTO1           (unsigned char)0x1c     // vertical scroll offset 1
#define F18A_REG_SIZES          (unsigned char)0x1d     // page size configuration
#define F18A_REG_MAXSPR         (unsigned char)0x1e     // max sprites per line
#define F18A_REG_BMLCFG         (unsigned char)0x1f     // bitmap layer config
#define F18A_REG_BMLADR         (unsigned char)0x20     // bitmap layer base address, times >0040
#define F18A_REG_BMLX           (unsigned char)0x21     // bitmap layer x
#define F18A_REG_BMLY           (unsigned char)0x22     // bitmap layer y
#define F18A_REG_BMLW           (unsigned char)0x23     // bitmap layer width
#define F18A_REG_BMLH           (unsigned char)0x24     // bitmap layer height
#define F18A_REG_DPM            (unsigned char)0x2f     // DPM control
#define F18A_REG_INC            (unsigned char)0x30     // increment for VDP RAM access
#define F18A_REG_ECM            (unsigned char)0x31     // ECM control
#define F18A_REG_PRIORITY       (unsigned char)0x32     // priority configuration
#define F18A_REG_STOPSPR        (unsigned char)0x33     // last sprite index to process
#define F18A_REG_GPUMSB         (unsigned char)0x36     // GPU MSB
#define F18A_REG_GPULSB         (unsigned char)0x37     // GPU LSB - writing starts execution per GPUCFG
#define F18A_REG_GPUCFG         (unsigned char)0x38     // GPU configuration
#define F18A_REG_UNLOCK         (unsigned char)0x39     // to unlock, write 000111xx twice.

// F18A status register indexes for F18A_REG_STATUS
#define F18A_STATUS_SR0         (unsigned char)0        // default status register
#define F18A_STATUS_IDBLANK     (unsigned char)1        // ID and blanking status
#define F18A_STATUS_GPUSTATUS   (unsigned char)2        // GPU status register
#define F18A_STATUS_SCANLINE    (unsigned char)3        // current scanline, 0 if blanking
#define F18A_STATUS_NANOLSB     (unsigned char)4        // 0-999 nanosecond counter, LSB
#define F18A_STATUS_NANOMSB     (unsigned char)5        // 0-999 nanosecond counter, MSB
#define F18A_STATUS_MICROLSB    (unsigned char)6        // 0-999 microsecond counter, LSB
#define F18A_STATUS_MICROMSB    (unsigned char)7        // 0-999 microsecond counter, MSB
#define F18A_STATUS_MILLILSB    (unsigned char)8        // 0-999 millisecond counter, LSB
#define F18A_STATUS_MILLIMSB    (unsigned char)9        // 0-999 millisecond counter, MSB
#define F18A_STATUS_SECONDLSB   (unsigned char)10       // 16-bit second counter, LSB
#define F18A_STATUS_SECONDMSB   (unsigned char)11       // 16-bit second counter, MSB
#define F18A_STATUS_VERSION     (unsigned char)14       // version major/minor 4 bits each
#define F18A_STATUS_REGISTER    (unsigned char)15       // read back last written VDP register

// bit values for registers
#define F18A_DPM_ENABLE         (unsigned char)0x80     // enable DPM mode
#define F18A_DPM_INC            (unsigned char)0x40     // auto-increment DPM after write

#define F18A_PRI_RESET          (unsigned char)0x80     // reset all VDP registers
#define F18A_PRI_GPUHSYNC       (unsigned char)0x40     // trigger GPU on hsync
#define F18A_PRI_GPUVSYNC       (unsigned char)0x20     // trigger GPU on vsync
#define F18A_PRI_DISABLE1       (unsigned char)0x10     // disable tile 1 layers
#define F18A_PRI_REPORTSPRMAX   (unsigned char)0x08     // report VR30 in status instead of 5th sprite
#define F18A_PRI_SCANLINES      (unsigned char)0x04     // enable virtual scanlines
#define F18A_PRI_POSATTRIBUTES  (unsigned char)0x02     // use position attributes instead of name
#define F18A_PRI_TILE2PRI       (unsigned char)0x01     // when not set, TL2 is always above sprites

#define F18A_GPUCFG_OP          (unsigned char)0x01     // when set, GPU starts when LSB written

#define F18A_UNLOCK_VALUE       (unsigned char)0x1c     // value written twice to F18A_REG_UNLOCK to unlock F18A features
#endif
