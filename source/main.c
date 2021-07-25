#include "Intellisense.h"
#include "gba_macros.h"
#include <gba_types.h>
#include "gba_gfx.h"
#include "gba_drawing.h"
#include "gba_mathUtil.h"
#include <gba_input.h>
#include <maxmod.h>
#include <gba.h>
#include <gba_sound.h>

#include <string.h>

#include "soundbank.h"
#include "soundbank_bin.h"

// image
#include "titlebg.h"
#include "titlebg2.h"

u8 myMixingBuffer[ MM_MIXLEN_16KHZ ] __attribute((aligned(4)));

void maxmodInit( void )
{
    irqInit();
    irqSet( IRQ_VBLANK, mmVBlank );
    irqEnable( IRQ_VBLANK );

    u8* myData;
    mm_gba_system mySystem;

    // allocate data for channel buffers & wave buffer (malloc'd data goes to EWRAM)
    // Use the SIZEOF definitions to calculate how many bytes to reserve
    myData = (u8*)malloc( 8 * (MM_SIZEOF_MODCH
                               +MM_SIZEOF_ACTCH
                               +MM_SIZEOF_MIXCH)
                          +MM_MIXLEN_16KHZ );

    // setup system info
    // 16KHz software mixing rate, select from mm_mixmode
    mySystem.mixing_mode       = MM_MIX_16KHZ;

    // number of module/mixing channels
    // higher numbers offer better polyphony at the expense
    // of more memory and/or CPU usage.
    mySystem.mod_channel_count = 8;
    mySystem.mix_channel_count = 8;

    // Assign memory blocks to pointers
    mySystem.module_channels   = (mm_addr)(myData+0);
    mySystem.active_channels   = (mm_addr)(myData+(8*MM_SIZEOF_MODCH));
    mySystem.mixing_channels   = (mm_addr)(myData+(8*(MM_SIZEOF_MODCH
                                                      +MM_SIZEOF_ACTCH)));
    mySystem.mixing_memory     = (mm_addr)myMixingBuffer;
    mySystem.wave_memory       = (mm_addr)(myData+(8*(MM_SIZEOF_MODCH
                                                      +MM_SIZEOF_ACTCH
                                                      +MM_SIZEOF_MIXCH)));
    // Pass soundbank address
    mySystem.soundbank         = (mm_addr)soundbank_bin;

    // Initialize Maxmod
    mmInit( &mySystem );
}

u16 __currKeys, __prevKeys;

#define KEY_MASK 0x03FF

inline void PollKeys()
{
    __prevKeys = __currKeys;
    __currKeys = ~REG_KEYINPUT & KEY_MASK;
}

inline u16		currentKeyState()           { return __currKeys; }
inline u16		prevKeyState()              { return __prevKeys; }

inline u16		keyHeld(u16 a_key)       { return (__currKeys & __prevKeys) & a_key; }
inline u16		keyReleased(u16 a_key)   { return (~__currKeys & __prevKeys) & a_key; }
inline u16		keyHit(u16 a_key)        { return (__currKeys & ~__prevKeys) & a_key; }

inline u16		keyStateChange(u16 a_key){ return (__currKeys ^ __prevKeys) & a_key; }

void FadeoutScreen(void);

void delayFrame(u16 frame){
    while(frame > 0){
        vsync();
        frame--;
    }
}

int main()
{
    memset(0x06000000, (u8)0xFF, 65535);

	//set GBA rendering context to MODE 4 Bitmap Rendering
	*(unsigned int*)0x04000000 = 0x0404;
    *(unsigned short*)0x05000002 = 0x7fff;
    *(unsigned short*)0x05000000 = 0x0000;

    maxmodInit();

    mmStart( MOD_END, MM_PLAY_LOOP );
    mmSetModuleTempo((mm_word)0x800);
    memcpy(SCREENBUFFER, titlebgBitmap, titlebgBitmapLen);
	while(1){
        mmFrame();
        vsync();
		PollKeys();
		if(keyReleased(KEY_START)) {
		    FadeoutScreen();
		    memset(0x06000000, (u8)0x00, 65535);
            *(unsigned int*)0x04000000 = 0x0403;
			memcpy(SCREENBUFFER, titlebg2Bitmap, titlebg2BitmapLen);
		}
	}
	return 0;
}

void FadeoutScreen(void){
    for(u8 h=0;h<31;h++){
        *(unsigned short*)0x05000002 -= 0x0421;
        delayFrame(3);
    }
    *(unsigned short*)0x05000002 = 0x0000;
    delayFrame(30);
}