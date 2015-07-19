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

#include <string.h>
#include "player.h"

// music and sound effect (122 bytes each)
// if RAM is tight, we could store these in VDP and just
// copy the one we are working on back and forth
struct SPF;		// forward reference
struct stream {
	unsigned char (*getbyte)(struct SPF*, unsigned char);
	unsigned char *streampos, *streamref, *streambase;
	unsigned char streamcnt;
};
struct voice {
	unsigned char tmcnt, tmocnt, tmovr;
};

struct SPF {
	struct stream str[12];
	struct voice voc[4];
	unsigned char *songad;
} music
#ifdef ENABLEFX
	,sfx
#endif
	;

// this lock ensures that stplay never tries to execute
// while a setup function is being called. Since the
// CPU can only do one thing at a time, we don't
// need true atomic locking, the flag is enough
unsigned char lock=0;

// remembers the audio settings on the music track
// can be read for feedback. Do not modify, used to
// restore audio when a sound effect ends.
struct OUTPUT musicout;

// audio port
volatile __sfr __at 0xff SOUND;

// bitmask for channels to play/being played
// MSB is music, LSB is sfx
unsigned int playmask=0;

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

unsigned char getbyte_newcmd(struct SPF* pMus, unsigned char idx);
unsigned char getbyte_count_run(struct SPF* pMus, unsigned char idx);
unsigned char getbyte_count_inc(struct SPF* pMus, unsigned char idx);
unsigned char getbyte_count_fixed(struct SPF* pMus, unsigned char idx);

// get a compressed byte from a stream (pointer to struct, stream index 0-11)
// all the following functions form a mini-state machine to reduce real time testing
unsigned char getbyte_newcmd(struct SPF* pMus, unsigned char idx) {
	unsigned char cnt;
	unsigned char *p;
	struct stream *str = &pMus->str[idx];

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
			str->streamref = (((*p)<<8) | (*(p+1))) + pMus->songad;
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
unsigned char getbyte_count_run(struct SPF* pMus, unsigned char idx) {
	// if (pMus->str[idx].streamcnt > 0) && (pMus->str[idx].streamref == <default case>)
	unsigned char ret;
	struct stream *str = &pMus->str[idx];

	// in the middle of a run, just get the next byte
	ret = *(str->streamref++);
	if (--str->streamcnt == 0) {
		// run finished, zero the reference pointer
//		str->streamref=0;
		str->getbyte = getbyte_newcmd;
	}
	return ret;
}
unsigned char getbyte_count_inc(struct SPF* pMus, unsigned char idx) {
	// if (pMus->str[idx].streamcnt > 0) && (pMus->str[idx].streamref == 0x0000)
	unsigned char ret;
	struct stream *str = &pMus->str[idx];

	// get a byte with increment from the main stream
	ret = *(str->streampos++);
	if (--str->streamcnt == 0) {
		str->getbyte = getbyte_newcmd;
	}
	return ret;
}
unsigned char getbyte_count_fixed(struct SPF* pMus, unsigned char idx) {
	// if (pMus->str[idx].streamcnt > 0) && (pMus->str[idx].streamref == 0xffff)
	unsigned char ret;
	struct stream *str = &pMus->str[idx];

	// get a byte with no increment from the main stream
	ret = *(str->streampos);
	if (--str->streamcnt == 0) {
		++str->streampos;	// on the last byte, increment the stream
		str->getbyte = getbyte_newcmd;
	}
	return ret;
}

#ifdef ENABLEFX
// start a new sound effect
void sfxinit(unsigned char *pMod, unsigned char num, unsigned char pri) {
	unsigned char idx;
	unsigned int pWork;

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
	memset(&sfx, 0, sizeof(sfx));

	sfx.songad = pMod;
	pWork = (*pMod)<<8;
	pWork |= *(pMod+1);
	pWork += num*24;		// 24 bytes per pointer table, add to offset

	// zero only the sfx part of the playmask
	playmask &= 0xff00;

	// init the playback registers
	for (idx = 0; idx < 12; idx++) {
		sfx.str[idx].getbyte = getbyte_newcmd;
		sfx.str[idx].streampos = sfx.songad + (((*(sfx.songad + pWork))<<8) | (*(sfx.songad + (pWork+1))));	// stream address
		sfx.str[idx].streambase = sfx.str[idx].streampos;			// save base
		pWork+=2;													// next stream
	}

	lock = 0;
}
#endif

void stinit(unsigned char *pMod, unsigned char num) {
	unsigned int pWork;
	unsigned char idx;

	lock = 1;

#ifdef RUN30HZ
	frflag = 0;			// clear frame flag (important! not done for sfx)
#endif

	// prepare to run
	memset(&music, 0, sizeof(music));

	music.songad = (unsigned char*)pMod; 
	pWork = (*pMod)<<8;
	pWork |= *(pMod+1);				// get big-endian address offset to pointer table
	pWork += (num*24);				// 24 bytes per pointer table, add to offset

	// init the playback registers
	for (idx = 0; idx < 12; idx++) {
		music.str[idx].getbyte = getbyte_newcmd;
		music.str[idx].streampos = music.songad + (((*(music.songad + pWork)) << 8) | (*(music.songad + (pWork+1))));	// get stream address
		music.str[idx].streambase = music.str[idx].streampos;		// save base
		pWork+=2;													// next stream
		music.str[idx].streamref = 0;
		music.str[idx].streamcnt = 0;
	}
	for (idx=0; idx<4; idx++) {
		music.voc[idx].tmcnt = 0;
		music.voc[idx].tmocnt = 0;
		music.voc[idx].tmovr = 0;
	}

	// put sane values in the user feedback registers (not done for sfx)
	playmask |= 0xff00;		// set music playing (will be overwritten with real value on first loop)

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

	playmask &= 0x00ff;		// music not playing

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

	playmask &= 0xff00;		// sfx not playing

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
		// move the music data to the least significant byte temporary to store it
		// clear the MSB so the SFX processing can store its data there
		playmask = (playmask>>8);
		// play sfx
		playone(&sfx);
		// swap the playmask back to normal
		playmask = (playmask>>8) | (playmask<<8);
		if (((playmask&0xff) == 0) && (sfxflag)) {
			// SFX over
			sfxflag = 0;
			restorechans();
		}
#endif
#ifdef RUN30HZ
		// if not 30hz, also process the music
		return;
#endif
	}

	// if not SFX, do music
	playmask &= 0x00ff;		// clear music bits
	playone(&music);		// play music
}

// process music (struct to use is passed in)
// handles muting channels using the LSB of playmask,
// and stores the channels being used in the MSB (caller must clear)
void playone(struct SPF* pMus) {
	char voice;
	struct voice *voc;

	for (voice = 3; voice >= 0; --voice) {
		voc = &pMus->voc[voice];
		
		if (pMus->str[8+voice].streampos) {		// test time stream pointer (streams 8-11)
			playmask |= 0x0100 << voice;		// set the active bit
			--voc->tmcnt;			// count down the voice
			if ((voc->tmcnt == 0) || (voc->tmcnt == 0xff)) {	// have to catch wraparound for certain start cases
				unsigned char work = 0;
				if (voc->tmocnt) {
					// override count active
					voc->tmocnt--;
					work = voc->tmovr;			// byte is a special run
				} else {
					work = pMus->str[8+voice].getbyte(pMus, 8+voice);		// get compressed byte from the timestream
					if (work == 0) {
						// stream ended, no more work
						pMus->str[8+voice].streampos=0;
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
					unsigned char x = pMus->str[voice].getbyte(pMus, voice);
					if (voice == 3) {
						// noise channel
						x|=0xe0;
						// check if we are allowed to play - sfx or mask bit not set
						if (
#ifdef ENABLEFX
							(pMus==&sfx) || 
#endif
							((playmask&(1<<voice))==0)
							) {
							SOUND = x;
						}
#ifdef ENABLEFX
						// if not sfx, save it
						if (pMus != &sfx) {
#else
						{
#endif
							musicout.tone[3] = x;
						}
					} else {
						// tone channel - need to look up in tone table
						unsigned int note;
						unsigned int off = *(pMus->songad+2)<<8;
						off |= *(pMus->songad+3);										// offset to frequency table
						off += x<<1;													// index of note
						note = *(pMus->songad+off);
						note |= (*(pMus->songad+(off+1)))<<8;							// get note (cmd byte in LSB)
						note |= (voice*0x20)+0x80;										// or in the command bits
						// check if we are allowed to play - sfx or mask bit not set
						if (
#ifdef ENABLEFX
							(pMus==&sfx) || 
#endif
							((playmask&(1<<voice))==0)
							) {
							SOUND = note&0xff;
							SOUND = note>>8;
						}
#ifdef ENABLEFX
						// if not sfx, save it
						if (pMus != &sfx) {
#else
						{
#endif
							musicout.tone[voice] = note;
						}
					}
				}
				if (work & 0x40) {
					// request to load volume
					unsigned char x = pMus->str[voice+4].getbyte(pMus, voice+4);
					x |= (voice*0x20)+0x90;		// or in the command bits
					// check if we are allowed to play - sfx or mask bit not set
					if (
#ifdef ENABLEFX
						(pMus==&sfx) || 
#endif
						((playmask&(1<<voice))==0)
						) {
						SOUND = x;
					}
					// if not sfx, save it
#ifdef ENABLEFX
					if (pMus != &sfx) {
#else
					{
#endif
						musicout.vol[voice] = x;
					}
				}
				voc->tmcnt = (work & 0x3f);		// save off the delay count (not certain why we don't decrement here like the asm does...)
			}
		}
	}
}
