#CC = "D:/work/coleco/sdcc-4.2.0/bin/sdcc"
#AS = "D:/work/coleco/sdcc-4.2.0/bin/sdasz80"
#AR = "D:/work/coleco/sdcc-4.2.0/bin/sdar"
CC = "D:/work/coleco/sdcc20230715/bin/sdcc"
AS = "D:/work/coleco/sdcc20230715/bin/sdasz80"
AR = "D:/work/coleco/sdcc20230715/bin/sdar"
CFLAGS = -mz80 -c "-I../include" "-I." --std-sdcc99 --less-pedantic --vc -DENABLEFX --opt-code-speed --fsigned-char
AFLAGS = -plosgff
RM = cmd /c del
# might need to use o for older SDCC, rel for newer
EXT=rel

# output file
NAME=libti99.a

# List of compiled objects used in executable
#  str_strlen.$(EXT)		- seems to be unneeded in latest SDCC
OBJECT_LIST=\
  f18a_detect.$(EXT)    \
  f18a_loadpal.$(EXT)    \
  f18a_lock.$(EXT)    \
  f18a_reset.$(EXT)    \
  f18a_startgpu.$(EXT)    \
  f18a_unlock.$(EXT)    \
  halt.$(EXT)			\
  kscan.$(EXT)			\
  kscanfast.$(EXT)		\
  joystfast.$(EXT)		\
  player.$(EXT)			\
  stcount.$(EXT)			\
  str_int2str.$(EXT)	\
  str_uint2str.$(EXT)	\
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
  vdp_putchar.$(EXT)	\
  vdp_printf.$(EXT)		\
  vdp_putstring.$(EXT)	\
  vdp_rawmemcpy.$(EXT)	\
  vdp_rawmemset.$(EXT)	\
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
	"d:/work/coleco/tursi/makemegacart/release/makemegacart.exe" crt0.ihx testib.rom

library: $(OBJECT_LIST)
	$(RM) testlib.$(EXT) testlib.asm
	$(AR) -rc $(NAME) $(OBJECT_LIST)
	
test: library testlib.$(EXT) crt0.$(EXT)
	$(CC) -mz80 --no-std-crt0 --code-loc 0x8100 --data-loc 0x7000 -l./libti99.a "./crt0.$(EXT)" testlib.$(EXT) 

# Recipe to clean all compiled objects
.phony clean:
	$(RM) *.rel *.map *.lst *.lnk *.sym *.asm *~ *.o *.obj *.ihx *.sprite.* *.rom *.rel *.a *.lib

# Recipe to compile all C files
%.rel: %.c
	$(CC) -c $< $(CFLAGS) -o $@

# Recipe to compile all assembly files
%.rel: %.s
	$(AS) -o $@ $<
