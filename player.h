/*
 music and sound effects player with sfx priority
 2014 by Tursi aka Mike Brent
 Released to public domain, may be used freely
*/

//*********************
// Player Data
//*********************

// pointers into the song data - all include the command nibble.
struct OUTPUT {
	unsigned char vol[4];
	unsigned int tone[4];
};
extern struct OUTPUT musicout;

// bitmask for channels to play/being played (used to be a single 16-bit packed value)
// playmask sets bits to indicate channels that the music should not play over (for SFX mostly)
extern unsigned char playmask;
// musicmask has set bits just to indicate which channels are still playing
extern unsigned char musicmask;

#define pVoice (&musicout.tone[0])
#define pVol   (&musicout.vol[0])
// pDone is the MSB, and the Z80 is little endian
#define pDone  ( (*((volatile unsigned char*)(&playmask)+1)) == 0 )

// note, the pitch bytes are reversed, and as written to the sound chip. This means:
// for a tone channel, 0x2381 is a pitch value (on channel 0) of 0x0123
// for noise, 0x??E1 is a noise type 1 (and the ?? byte is not guaranteed)

//*********************
// Player functions
//*********************

// stinit - initialize a song stream playback
// Inputs: pSong: points to the compressed song data (in CPU memory)
//         index: index of the song to play (0 if not a multi-bank song)
void stinit(unsigned char *pMod, unsigned char num);

// ststopsfx - stop playback of the song stream
void ststop();

#ifdef ENABLEFX
// sfxinix - initialize a sound effect playback
// Inputs: pSong: points to the compressed song data (in CPU memory)
//         index: index of the sfx to play (0 if not a multi-bank)
//		   priority: priority of this sfx. If less than currently playing,
//					 this one will be ignored, else the other is replaced
//					 sfx always has priority over music
void sfxinit(unsigned char *pMod, unsigned char num, unsigned char pri);

// sfxstop - stop playback of current sfx
void sfxstop();

// allstop - stops music and sfx both
void allstop();
#endif

// stplay - play one tick (song and sfx)
// You must call this function once every 60hz interrupt. It is acceptable
// to load it directly into the VDP interrupt hook.
void stplay();

// stcount - returns how many songs are in a pack
// inputs - pSong - pointer to song data
// returns - count (which is just the table pointers subtracted and divided)
unsigned int stcount(const void *pSong);
