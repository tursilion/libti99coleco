/*
 music and sound effects player with sfx priority
 2014 by Tursi aka Mike Brent
 Released to public domain, may be used freely
*/

// enable this for 30hz mode, disable for 60hz
//#define RUN30HZ
// enable this to play sound effects, disable for music only
//#define ENABLEFX
// NOTE: for the sake of the headers, these need to be defined in your makefile! (ex: -DRUN30HZ)

#include <sdcc_string.h>
#include "player.h"

// music and sound effect (138 bytes each)
// if RAM is tight, we could store these in VDP and just
// copy the one we are working on back and forth
struct SPF;		// forward reference
struct stream {
	unsigned char (*getbyte)(struct stream *);
	unsigned char *streampos, *streamref, *streambase;
	unsigned char streamcnt;
};
struct voice {
	unsigned char tmcnt, tmocnt, tmovr;
};

struct SPF {
	struct stream voicestr[4];
	struct stream volstr[4];
	struct stream timestr[4];
	struct voice voc[4];
	unsigned char *songad;
	unsigned char *freqad;
} music
#ifdef ENABLEFX
	,sfx
#endif
	;

// this lock ensures that stplay never tries to execute
// while a setup function is being called. Since the
// CPU can only do one thing at a time, we don't
// need true atomic locking, the flag is enough
volatile unsigned char lock=0;

// This provides a global reference to the current
// song address - set only when playOne is called.
unsigned char *g_songad;

// remembers the audio settings on the music track
// can be read for feedback. Do not modify, used to
// restore audio when a sound effect ends.
struct OUTPUT musicout;

// audio port
volatile __sfr __at 0xff SOUND;

// bitmask for channels to play/being played (used to be a single 16-bit packed value)
// playmask sets bits to indicate channels that the music should not play over (for SFX mostly)
unsigned char playmask;
// musicmask has set bits just to indicate which channels are still playing
unsigned char musicmask;

#ifdef RUN30HZ
// frame flag for 30hz
unsigned char frflag=0;
#endif

#ifdef ENABLEFX
// sfx priority
unsigned char sfxflag=0;
void restorechans();
#endif

void playone(struct SPF* pMus);

unsigned char getbyte_newcmd(struct stream *str);
unsigned char getbyte_count_run(struct stream *str);
unsigned char getbyte_count_inc(struct stream *str);
unsigned char getbyte_count_fixed(struct stream *str);

// debug code
//#define COUNT_BYTE_FUNCTIONS
#ifdef COUNT_BYTE_FUNCTIONS
unsigned int cnt_newcmd = 0;
unsigned int cnt_run = 0;
unsigned int cnt_fixed = 0;
unsigned int cnt_inc = 0;

void getcnts(unsigned int *i1, unsigned int *i2, unsigned int *i3, unsigned int *i4) {
	*i1 = cnt_newcmd;
	*i2 = cnt_run;
	*i3 = cnt_fixed;
	*i4 = cnt_inc;
}

#endif

// get a compressed byte from a stream (pointer to struct, stream index 0-11)
// all the following functions form a mini-state machine to reduce real time testing
// This function expects that g_songad is set correctly!
unsigned char getbyte_newcmd(struct stream *str) {
	unsigned char cnt;
	unsigned char *p;

#ifdef COUNT_BYTE_FUNCTIONS
	++cnt_newcmd;
#endif

	// count is zero, need to get a new command (also returns current byte)
	cnt = *(str->streampos++);
	if (cnt == 0) {
		// song over
		return 0;
	}

	// else, check the top two bits to see what to set up
	switch (cnt & 0xc0) {
		case 0xc0:
			// two byte reference - song offset
			cnt &= 0x3f;		// mask count
			if (--cnt) {		// account for the one we are taking
				str->getbyte = getbyte_count_run;
				str->streamcnt=cnt;	// save it
			}
			p = str->streampos;
			str->streamref = (((*p)<<8) | (*(p+1))) + g_songad;
			str->streampos += 2;
			return *(str->streamref++);			// get back-referenced byte

		case 0x80:
			// single byte reference - stream offset
			cnt &= 0x3f;		// mask count
			if (--cnt) {		// account for the one we are taking
				str->getbyte = getbyte_count_run;
				str->streamcnt=cnt;	// save it
			}
			str->streamref=*(str->streampos++) + str->streambase;	// LSB + stream base
			return *(str->streamref++);						// get back referenced byte

		case 0x40:
			// this is going to be a single character repeated
			cnt &= 0x3f;			// mask it
			if (--cnt) {			// account for the one we are taking
				str->getbyte = getbyte_count_fixed;
				str->streamcnt=cnt;		// save it
			}
			return *(str->streampos);	// get byte - note no increment

		case 0x00:
			// neither bit set, then it's just a plain run
			if (--cnt) {	// account for the one we are taking
				str->getbyte = getbyte_count_inc;
				str->streamcnt=cnt;	// save it
			}
			return *(str->streampos++);		// get byte
	}

	// shouldn't get here
	return 0;
}

// several getbyte functions here - depending on the mode
// reduces the number of decisions per read
unsigned char getbyte_count_run(struct stream *str) {
	// if (pMus->str[idx].streamcnt > 0) && (pMus->str[idx].streamref == <default case>)
	unsigned char ret;

#ifdef COUNT_BYTE_FUNCTIONS
	++cnt_run;
#endif

	// in the middle of a run, just get the next byte
	ret = *(str->streamref++);
	if (--str->streamcnt == 0) {
		// run finished, back to command mode
		str->getbyte = getbyte_newcmd;
	}
	return ret;
}
unsigned char getbyte_count_inc(struct stream *str) {
	// if (pMus->str[idx].streamcnt > 0) && (pMus->str[idx].streamref == 0x0000)
	unsigned char ret;

#ifdef COUNT_BYTE_FUNCTIONS
	++cnt_inc;
#endif

	// get a byte with increment from the main stream
	ret = *(str->streampos++);
	if (--str->streamcnt == 0) {
		str->getbyte = getbyte_newcmd;
	}
	return ret;
}
unsigned char getbyte_count_fixed(struct stream *str) {
	// if (pMus->str[idx].streamcnt > 0) && (pMus->str[idx].streamref == 0xffff)
	unsigned char ret;

#ifdef COUNT_BYTE_FUNCTIONS
	++cnt_fixed;
#endif

	// get a byte with no increment from the main stream
	ret = *(str->streampos);
	if (--str->streamcnt == 0) {
		++str->streampos;	// on the last byte, increment the stream
		str->getbyte = getbyte_newcmd;
	}
	return ret;
}

void initstream(struct stream *str, unsigned char *pos, unsigned char *songad) {
	unsigned char *realPtr = (((*pos)<<8) | (*(pos+1))) + songad;
	str->getbyte = getbyte_newcmd;
	str->streampos = realPtr;
	str->streambase = realPtr;
	str->streamref = 0;
	str->streamcnt = 0;
}

#ifdef ENABLEFX
// start a new sound effect
void sfxinit(unsigned char *pMod, unsigned char num, unsigned char pri) {
	unsigned char idx;
	unsigned char *pWork;	// stream offset pointer

	lock = 1;

	if (pri < sfxflag) {
		// higher priority sfx already playing
		lock = 0;
		return;
	}

	if (sfxflag) {
		// SFX was playing, so restore the channels before we proceed
		restorechans();
	}

	// save off the new priority
	sfxflag = pri;

	// prepare to run
	sfx.songad = pMod;

	// address of the frequency table (so we don't need to calculate it every frame, repeatedly)
	sfx.freqad = pMod + (*(pMod+2)<<8) + (*(pMod+3));

	// get the address of the stream table for this song
	pWork = pMod + (((*pMod)<<8)|(*(pMod+1)));
	pWork += num*24;		// 24 bytes per pointer table, add to offset

	// for SFX, leave playmask alone

	// init the playback registers - just piggyback the voc init on the first one
	for (idx = 0; idx < 4; idx++) {
		initstream(&sfx.voicestr[idx], pWork, pMod);	// stream address
		pWork+=2;										// next stream
		
		sfx.voc[idx].tmocnt = 0;
		sfx.voc[idx].tmcnt = 1;							// so it triggers zero on first frame
	}
	for (idx = 0; idx < 4; idx++) {
		initstream(&sfx.volstr[idx], pWork, pMod);		// stream address
		pWork+=2;										// next stream
	}
	for (idx = 0; idx < 4; idx++) {
		initstream(&sfx.timestr[idx], pWork, pMod);		// stream address
		pWork+=2;										// next stream
	}
	
	lock = 0;
}
#endif

void stinit(unsigned char *pMod, unsigned char num) {
	unsigned char *pWork;
	unsigned char idx;

	lock = 1;

#ifdef RUN30HZ
	frflag = 0;			// clear frame flag (important! not done for sfx)
#endif

	// prepare to run
	music.songad = pMod; 

	// address of the frequency table (so we don't need to calculate it every frame, repeatedly)
	music.freqad = pMod + (*(pMod+2)<<8) + (*(pMod+3));
	
	// get the address of the stream table for this song
	pWork = pMod + (((*pMod)<<8)|(*(pMod+1)));
	pWork += num*24;		// 24 bytes per pointer table, add to offset

	// init the playback registers
	for (idx = 0; idx < 4; idx++) {
		initstream(&music.voicestr[idx], pWork, pMod);	// stream address
		pWork+=2;										// next stream
		
		music.voc[idx].tmocnt = 0;
		music.voc[idx].tmcnt = 1;						// so it triggers zero on first frame
	}
	for (idx = 0; idx < 4; idx++) {
		initstream(&music.volstr[idx], pWork, pMod);	// stream address
		pWork+=2;										// next stream
	}
	for (idx = 0; idx < 4; idx++) {
		initstream(&music.timestr[idx], pWork, pMod);	// stream address
		pWork+=2;										// next stream
	}

	// put sane values in the user feedback registers (not done for sfx)
	// note that setting playmask here may cause a glitch on sound effects already playing
	// also, if you need to externally control playmask, you probably want to remove this line ;)
	playmask = 0x00;		// no muted channels
	musicmask = 0xff;		// set music playing (will be overwritten with real value on first loop)

	for (idx=0; idx<4; idx++) {
		musicout.vol[idx]=0x9f + (idx*0x20);
		musicout.tone[idx] = 1;		// highest frequency
	}

	lock = 0;
}

// call to stop the tune or initialize to silence
void ststop() {
	unsigned char idx;

	lock = 1;

	memset(&music, 0, sizeof(music));

	for (idx=0; idx<4; idx++) {
		musicout.vol[idx]=0x9f + (idx*0x20);
		musicout.tone[idx] = 1;		// highest frequency
	}

	musicmask = 0x00;		// music not playing

	lock = 0;
}

#ifdef ENABLEFX
// call to stop the sfx or initialize to silence
void sfxstop() {
	lock = 1;

	if (sfxflag) {
		restorechans();
		sfxflag=0;
	}

	memset(&sfx, 0, sizeof(sfx));
	playmask = 0x00;		// no SFX muted channels

	lock = 0;
}
	
// tiny helper for sfx restore
inline void loadtone(unsigned int tone) {
	SOUND = (tone&0xff);
	SOUND = (tone >> 8);
}

// restore music data into the audio channels (at end of SFX)
void restorechans() {
	loadtone(musicout.tone[0]);
	loadtone(musicout.tone[1]);
	loadtone(musicout.tone[2]);
	SOUND = musicout.tone[3]&0xff;
	SOUND = musicout.vol[0];
	SOUND = musicout.vol[1];
	SOUND = musicout.vol[2];
	SOUND = musicout.vol[3];
}
#endif

// call to stop both SFX and music
void allstop() {
#ifdef ENABLEFX
	sfxstop();
#endif
	ststop();
}

// call every vblank to update the music
// intended to be called from nmi
void stplay() {
	// don't execute if a control function is in mid-execution
	if (lock) return;

#ifdef RUN30HZ
	frflag=!frflag;		// boolean invert, so any init state is okay
	if (!frflag) {
#else
	{
#endif
#ifdef ENABLEFX
		// process sound effects

		if (sfxflag) {
			// we'll overwrite musicmask as well, so remember that
#ifdef RUN30HZ
			// we only really need this for 30hz mode...
			unsigned char oldmask = musicmask;
#endif

			// clear the playmask, we're about to reset it
			playmask = 0;
			musicmask = 0;

			// play sfx - will store its bits in the musicmask
			playone(&sfx);

			// copy it over and restore the musicmask
			playmask = musicmask;

#ifdef RUN30HZ
			// don't need to restore in 60hz mode cause we're about to update it anyway
			musicmask = oldmask;
#endif

			// check if sfx was finished
			if (playmask == 0) {
				// SFX over
				sfxflag = 0;
				restorechans();
			}
		}

#endif
#ifdef RUN30HZ
		// if not 30hz, also process the music
		return;
#endif
	}

	// if not SFX, do music
	musicmask = 0;			// clear the mask so we can update it (playmask has our channel mutes)
	playone(&music);		// play music
}

// process music (struct to use is passed in)
// handles muting channels using playmask,
// and stores the channels being used in musicmask
void playone(struct SPF* pMus) {
	char voice;
	struct voice *voc;
	unsigned char isSfx = 0;

	if (pMus==&sfx) {
		isSfx = 1;
	}

	// this is used fairly often. As a bonus making it global
	// means we no longer need to pass it to the getbyte functions,
	// only newcmd of which needs it (and in only one case)
	g_songad = pMus->songad;

	for (voice = 3; voice >= 0; --voice) {
		voc = &pMus->voc[voice];
		
		if (pMus->timestr[voice].streampos) {		// test time stream pointer
			musicmask |= 0x01 << voice;				// set the active bit
			--voc->tmcnt;							// count down the voice
			if (voc->tmcnt == 0) {
				unsigned char work = 0;
				if (voc->tmocnt) {
					// override count active
					voc->tmocnt--;
					work = voc->tmovr;			// byte is a special run
				} else {
					work = pMus->timestr[voice].getbyte(&pMus->timestr[voice]);		// get compressed byte from the timestream
					if (work == 0) {
						// stream ended, no more work
						pMus->timestr[voice].streampos=0;
						continue;
					}
					// magic override mapping - counts are minus 1 to account for the byte we use
					switch (work) {
					case 0x7a:
						voc->tmovr=0x43;
						voc->tmocnt=1;
						work=0x43;
						break; 

					case 0x7b:
					case 0x7c:
						voc->tmovr=0x42;
						voc->tmocnt=work-0x7a;
						work=0x42;
						break;

					case 0x7d:
					case 0x7e:
					case 0x7f:
						voc->tmovr=0x41;
						voc->tmocnt=work-0x7c;
						work=0x41;
						break;

					// no default: on purpose, do nothing if not magic!
					}
				}
				// now we have the current timestream command byte to process
				if (work&0x80) {
					// request to load a frequency
					unsigned char x = pMus->voicestr[voice].getbyte(&pMus->voicestr[voice]);
					if (voice == 3) {
						// noise channel
						x|=0xe0;
						// check if we are allowed to play - sfx or mask bit not set
						if (
#ifdef ENABLEFX
							(isSfx) || 
#endif
							((playmask&(1<<voice))==0)
							) {
							SOUND = x;
						}
#ifdef ENABLEFX
						// if not sfx, save it
						if (!isSfx) {
#else
						{
#endif
							musicout.tone[3] = x;
						}
					} else {
						// tone channel - need to look up in tone table
						unsigned int note;
						// pointer into frequency table
						unsigned char *tone = pMus->freqad;
						tone += x<<1;												// index of note
						note = *tone;
						note |= (*(tone+1))<<8;										// get note (cmd byte in LSB)
						note |= (voice*0x20)+0x80;									// or in the command bits
						// check if we are allowed to play - sfx or mask bit not set
						if (
#ifdef ENABLEFX
							(isSfx) || 
#endif
							((playmask&(1<<voice))==0)
							) {
							SOUND = note&0xff;
							SOUND = note>>8;
						}
#ifdef ENABLEFX
						// if not sfx, save it
						if (!isSfx) {
#else
						{
#endif
							musicout.tone[voice] = note;
						}
					}
				}
				if (work & 0x40) {
					// request to load volume
					unsigned char x = pMus->volstr[voice].getbyte(&pMus->volstr[voice]);
					x |= (voice*0x20)+0x90;		// or in the command bits
					// check if we are allowed to play - sfx or mask bit not set
					if (
#ifdef ENABLEFX
						(isSfx) || 
#endif
						((playmask&(1<<voice))==0)
						) {
						SOUND = x;
					}
					// if not sfx, save it
#ifdef ENABLEFX
					if (!isSfx) {
#else
					{
#endif
						musicout.vol[voice] = x;
					}
				}
				voc->tmcnt = (work & 0x3f);		// save off the delay count (not certain why we don't decrement here like the TI asm does...)
				if (voc->tmcnt == 0) ++voc->tmcnt;	// prevent init to zero to save a test
			}
		}
	}
}
