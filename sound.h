// Helpers for direct and console interrupt sound processing

//*********************
// direct sound chip access
//*********************
volatile __sfr __at 0xff SOUND;

//*********************
// console sound interrupt pointers (not supported)
//*********************
// pointer to load the address (in VDP or GROM) of your sound list
//#define SOUND_PTR	*((volatile unsigned int*)0x83cc)

// countdown byte - set to '1' to start processing on the next interrupt
// You can also read this back, if it is zero, then the list is complete.
//#define SOUND_CNT	*((volatile unsigned char*)0x83ce)

// Flag byte - set SOUND_VDP_MASK to indicate a sound list is in VDP instead of GROM
//#define SOUND_VDP	*((volatile unsigned char*)0x83fd)
//#define SOUND_VDP_MASK	0x01

// Command nibbles
#define TONE1_FREQ	0x80
#define TONE1_VOL	0x90
#define TONE2_FREQ	0xA0
#define TONE2_VOL	0xB0
#define TONE3_FREQ	0xC0
#define TONE3_VOL	0xD0
#define NOISE_MODE	0xE0
#define NOISE_VOL	0xF0

//*********************
// console sound interrupt helpers (remember you still have to enable interrupts in your main loop!)
// (not supported)
//*********************

// set the address of your sound list
//inline void SET_SOUND_PTR(unsigned int x)	{	SOUND_PTR = x;					}

// set that the sound list is in VDP
//inline void SET_SOUND_VDP()				{	SOUND_VDP |= SOUND_VDP_MASK;	}

// set that the sound list is in GROM
//inline void SET_SOUND_GROM()				{	SOUND_VDP &= ~SOUND_VDP_MASK;	}

// start processing sound list on the next interrupt
//inline void START_SOUND()					{	SOUND_CNT = 1;					}

// mute all channels
inline void MUTE_SOUND()					{ SOUND=TONE1_VOL|0x0f; SOUND=TONE2_VOL|0x0f; SOUND=TONE3_VOL|0x0f; SOUND=NOISE_VOL|0x0f; }

//*********************
// AY sound chip access (if SGM installed)
//*********************
volatile __sfr __at 0x50 AY_REGISTER;
volatile __sfr __at 0x51 AY_DATA_WRITE;
volatile __sfr __at 0x52 AY_DATA_READ;

#define AY_PERIODA_LOW  0
#define AY_PERIODA_HIGH 1
#define AY_PERIODB_LOW  2
#define AY_PERIODB_HIGH 3
#define AY_PERIODC_LOW  4
#define AY_PERIODC_HIGH 5
#define AY_NOISE        6
#define AY_MIXER        7
#define AY_VOLA         8
#define AY_VOLB         9
#define AY_VOLC         10
#define AY_ENV_LOW      11
#define AY_ENV_HIGH     12
#define AY_ENV_SHAPE    13
#define AY_PORTA        14
#define AY_PORTB        15



