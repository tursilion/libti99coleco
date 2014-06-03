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

#include "player.h"

// music and sound effect (98 bytes each)
// if RAM is tight, we could store these in VDP and just
// copy the one we are working on back and forth
struct SPF {
	unsigned int streampos[12], streamref[12], streambase[12];
	unsigned char streamcnt[12];
	unsigned char tmcnt[4], tmocnt[4], tmovr[4];
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

// get a compressed byte from a stream (pointer to struct, stream index 0-11)
unsigned char getbyte(struct SPF* pMus, int idx) {
	unsigned char ret, cnt;

	if (pMus->streamcnt[idx] > 0) {
		// working on a count - streamref tells us what to get
		switch (pMus->streamref[idx]) {
		default:
			// in the middle of a run, just get the next byte
			ret = *(pMus->songad + (pMus->streamref[idx]++));
			pMus->streamcnt[idx]--;
			if (pMus->streamcnt[idx] == 0) {
				// run finished, zero the reference pointer
				pMus->streamref[idx]=0;
			}
			return ret;

		case 0x0000:
			// get a byte with increment from the main stream
			ret = *(pMus->songad + (pMus->streampos[idx]++));
			--pMus->streamcnt[idx];
			return ret;

		case 0xffff:
			// get a byte with no increment from the main stream
			ret = *(pMus->songad + (pMus->streampos[idx]));
			--pMus->streamcnt[idx];
			if (pMus->streamcnt[idx] == 0) pMus->streampos[idx]++;	// on the last byte, increment the stream
			return ret;
		}
	}

	// count is zero, need to get a new command
	cnt = *(pMus->songad + (pMus->streampos[idx]++));
	if (cnt == 0) {
		// song over
		return 0;
	}

	// else, check the top two bits to see what to set up
	switch (cnt & 0xc0) {
		case 0xc0:
			// two byte reference - song offset
			cnt &= 0x3f;		// mask count
			cnt--;				// account for the byte we will consume below
			pMus->streamcnt[idx]=cnt;	// save it
			pMus->streamref[idx]=*(pMus->songad + (pMus->streampos[idx]++))<<8;	// get MSB
			pMus->streamref[idx]|=*(pMus->songad + (pMus->streampos[idx]++));	// get LSB
			return *(pMus->songad + (pMus->streamref[idx]++));					// get back-referenced byte

		case 0x80:
			// single byte reference - stream offset
			cnt &= 0x3f;		// mask count
			cnt--;				// account for the byte we will consume below
			pMus->streamcnt[idx]=cnt;	// save it
			pMus->streamref[idx]=*(pMus->songad + (pMus->streampos[idx]++));		// get LSB
			pMus->streamref[idx]+=pMus->streambase[idx];					// add stream base
			return *(pMus->songad + (pMus->streamref[idx]++));				// get back referenced byte

		case 0x40:
			// this is going to be a single character repeated
			cnt &= 0x3f;					// mask it
			cnt--;							// account for the one we are going to take
			pMus->streamcnt[idx]=cnt;		// save it
			cnt = *(pMus->songad + (pMus->streampos[idx]));		// get byte - note no increment
			pMus->streamref[idx]=0xffff;	// flag as a repeated byte
			return cnt;

		case 0x00:
			// neither bit set, then it's just a plain run
			cnt--;			// account for the one we are taking
			pMus->streamcnt[idx]=cnt;	// save it
			cnt = *(pMus->songad + (pMus->streampos[idx]++));		// get byte
			pMus->streamref[idx]=0;		// zero the reference
			return cnt;
	}

	// should never get here
	return 0;
}

#ifdef ENABLEFX
// start a new sound effect
void sfxinit(unsigned char *pMod, unsigned char num, unsigned char pri) {
	int idx;
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
	sfx.songad = pMod;
	pWork = (*pMod)<<8;
	pWork |= *(pMod+1);
	pWork += num*24;		// 24 bytes per pointer table, add to offset

	// zero only the sfx part of the playmask
	playmask &= 0xff00;

	// init the playback registers
	for (idx = 0; idx < 12; idx++) {
		sfx.streampos[idx] = (*(sfx.songad + pWork))<<8;
		sfx.streampos[idx] |= *(sfx.songad + (pWork+1));				// get stream offset
		sfx.streambase[idx] = sfx.streampos[idx];						// save base
		pWork+=2;														// next stream
		sfx.streamref[idx] = 0;
		sfx.streamcnt[idx] = 0;
	}
	for (idx=0; idx<4; idx++) {
		sfx.tmcnt[idx] = 0;
		sfx.tmocnt[idx] = 0;
		sfx.tmovr[idx] = 0;
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
	music.songad = (unsigned char*)pMod; 
	pWork = (*pMod)<<8;
	pWork |= *(pMod+1);				// get big-endian address offset to pointer table
	pWork += (num*24);				// 24 bytes per pointer table, add to offset

	// init the playback registers
	for (idx = 0; idx < 12; idx++) {
		music.streampos[idx] = (*(music.songad + pWork)) << 8;
		music.streampos[idx] |= *(music.songad + (pWork+1));		// get stream offset
		music.streambase[idx] = music.streampos[idx];				// save base
		pWork+=2;													// next stream
		music.streamref[idx] = 0;
		music.streamcnt[idx] = 0;
	}
	for (idx=0; idx<4; idx++) {
		music.tmcnt[idx] = 0;
		music.tmocnt[idx] = 0;
		music.tmovr[idx] = 0;
	}

	// put sane values in the user feedback registers (not done for sfx)
	playmask |= 0xff00;		// set music playing (will be overwritten with real value on first int)

	for (idx=0; idx<4; idx++) {
		musicout.vol[idx]=0x9f + (idx*0x20);
		musicout.tone[idx] = 0;
	}

	lock = 0;
}

// call to stop the tune or initialize to silence
void ststop() {
	int idx;

	lock = 1;

	for (idx=0; idx<12; idx++) {
		music.streampos[idx] = 0;
		music.streambase[idx] = 0;
		music.streamref[idx] = 0;
		music.streamcnt[idx] = 0;
	}
	for (idx=0; idx<4; idx++) {
		music.tmcnt[idx] = 0;
		music.tmocnt[idx] = 0;
		music.tmovr[idx] = 0;
	}

	for (idx=0; idx<4; idx++) {
		musicout.vol[idx]=0x9f + (idx*0x20);
		musicout.tone[idx] = 0;
	}

	playmask &= 0x00ff;		// music not playing

	lock = 0;
}

#ifdef ENABLEFX
// call to stop the sfx or initialize to silence
void sfxstop() {
	int idx;

	lock = 1;

	if (sfxflag) {
		restorechans();
		sfxflag=0;
	}

	for (idx=0; idx<12; idx++) {
		sfx.streampos[idx] = 0;
		sfx.streambase[idx] = 0;
		sfx.streamref[idx] = 0;
		sfx.streamcnt[idx] = 0;
	}
	for (idx=0; idx<4; idx++) {
		sfx.tmcnt[idx] = 0;
		sfx.tmocnt[idx] = 0;
		sfx.tmovr[idx] = 0;
	}

	playmask &= 0xff00;		// sfx not playing

	lock = 0;
}
	
// tiny helper for sfx restore
void loadtone(unsigned int tone) {
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
		playmask = ((playmask>>8) | ((playmask&0xff)<<8));
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
	int voice;

	for (voice = 0; voice < 4; voice++) {
		if (pMus->streampos[8+voice]) {		// test time stream pointer (streams 8-11)
			playmask |= 0x0100 << voice;	// set the active bit
			pMus->tmcnt[voice]--;					// count down the voice
			if ((pMus->tmcnt[voice] == 0) || (pMus->tmcnt[voice] == 0xff)) {	// have to catch wraparound for certain start cases
				unsigned char work = 0;
				if (pMus->tmocnt[voice]) {
					// override count active
					pMus->tmocnt[voice]--;
					work = pMus->tmovr[voice];			// byte is a special run
				} else {
					work = getbyte(pMus, 8+voice);		// get compressed byte from the timestream
					if (work == 0) {
						// stream ended, no more work
						pMus->streampos[8+voice]=0;
						continue;
					}
					if ((work>=0x7a)&&(work<=0x7f)) {
						// magic override mapping - counts are minus 1 to account for the byte we use
						switch (work) {
						case 0x7a:
							pMus->tmovr[voice]=0x43;
							pMus->tmocnt[voice]=1;
							work=0x43;
							break;

						case 0x7b:
						case 0x7c:
							pMus->tmovr[voice]=0x42;
							pMus->tmocnt[voice]=work-0x7a;
							work=0x42;
							break;

						case 0x7d:
						case 0x7e:
						case 0x7f:
							pMus->tmovr[voice]=0x41;
							pMus->tmocnt[voice]=work-0x7c;
							work=0x41;
							break;
						}
					}
				}
				// now we have the current timestream command byte to process
				if (work&0x80) {
					// request to load a frequency
					unsigned char x = getbyte(pMus, voice);
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
					unsigned char x = getbyte(pMus, voice+4);
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
				pMus->tmcnt[voice] = (work & 0x3f);		// save off the delay count (not certain why we don't decrement here like the asm does...)
			}
		}
	}
}
