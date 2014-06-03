CC = sdcc
CFLAGS = -mz80 -c "-I../../../include" --std-sdcc99 --vc -DENABLEFX --opt-code-speed
AS = "/cygdrive/c/program files (x86)/sdcc/bin/sdasz80"
AR = sdar
AFLAGS = -plosgff
# might need to use o for older SDCC, rel for newer
EXT=rel

# output file
NAME=libti99.a

# List of compiled objects used in executable
OBJECT_LIST=\
  halt.$(EXT)			\
  kscan.$(EXT)			\
  kscanfast.$(EXT)		\
  joystfast.$(EXT)		\
  player.$(EXT)			\
  stcount.$(EXT)			\
  str_strlen.$(EXT)		\
  vdp_char.$(EXT)		\
  vdp_charset.$(EXT)		\
  vdp_charsetlc.$(EXT)	\
  vdp_delsprite.$(EXT)	\
  vdp_gchar.$(EXT)		\
  vdp_hchar.$(EXT)		\
  vdp_hexprint.$(EXT)	\
  vdp_fasthexprint.$(EXT)\
  vdp_fasterhexprint.$(EXT)\
  vdp_byte2hex.$(EXT)	\
  vdp_ints.$(EXT)		\
  vdp_memcpy.$(EXT)		\
  vdp_memread.$(EXT)		\
  vdp_memset.$(EXT)		\
  vdp_putstring.$(EXT)	\
  vdp_readchar.$(EXT)	\
  vdp_screenchar.$(EXT)	\
  vdp_scrnscroll.$(EXT)	\
  vdp_setbitmap.$(EXT)	\
  vdp_setgraphics.$(EXT)	\
  vdp_setmode.$(EXT)		\
  vdp_setmulticolor.$(EXT)	\
  vdp_settext.$(EXT)		\
  vdp_sprite.$(EXT)		\
  vdp_textdefs.$(EXT)	\
  vdp_vchar.$(EXT)		\
  vdp_waitvint.$(EXT)	\
  vdp_writeinc.$(EXT)	\
  vdp_writescreeninc.$(EXT)	\
  vdp_writestring.$(EXT)

# Recipe to compile the library
all: library test
	"/cygdrive/c/work/coleco/tursi/makemegacart/debug/makemegacart.exe" crt0.ihx testib.rom

library: $(OBJECT_LIST)
	rm -f testlib.$(EXT) testlib.asm
	$(AR) -rc $(NAME) *.$(EXT)
	
test: library testlib.$(EXT) crt0.$(EXT)
	$(CC) -mz80 --no-std-crt0 --code-loc 0x8100 --data-loc 0x7000 -l./libti99.a "./crt0.$(EXT)" testlib.$(EXT) 

# Recipe to clean all compiled objects
.phony clean:
	rm -f *.rel *.map *.lst *.lnk *.sym *.asm *~ *.o *.obj *.ihx *.sprite.* *.rom *.rel

# Recipe to compile all C files
%.rel: %.c
	$(CC) -c $< $(CFLAGS) -o $@

# Recipe to compile all assembly files
%.rel: %.s
	$(AS) -o $@ $<
