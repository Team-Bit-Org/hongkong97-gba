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
#include "hk97main.h"
#include "textrom.h"

#include "prologue1.h"
#include "prologue2.h"
#include "prologue3.h"
#include "prologue4.h"

// text
#include "textBank01.h"

u8 myMixingBuffer[ MM_MIXLEN_16KHZ ] __attribute((aligned(4)));

u16* tileBuffer = 0x06000000;

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
void FadeinScreen(void);
void SetLetterInScreen(u16 letter, u8 x, u8 y);

void delayFrame(u16 frame){
    while(frame > 0){
        mmFrame();
        vsync();
        frame--;
    }
    return;
}

int main()
{
    u8 currentMap = 0;

    memset(tileBuffer, (u8)0x00, sizeof tileBuffer);

    memset(0x06000000, (u8)0xFF, 65535);

	//set GBA rendering context to MODE 4 Bitmap Rendering
	*(unsigned int*)0x04000000 = 0x0404;

    maxmodInit();

    mmSetModuleVolume(2048);
    mmStart( MOD_BG, MM_PLAY_LOOP );
    memcpy(SCREENBUFFER, titlebgBitmap, titlebgBitmapLen);
    FadeinScreen();

	while(1){
        mmFrame();
        vsync();
		PollKeys();
		if(keyReleased(KEY_START) && currentMap == 0) {
		    FadeoutScreen();
		    memset(0x06000000, (u8)0x00, 65535);
		    memcpy(0x05000000, titlebg2Pal, titlebg2PalLen);
			memcpy(0x06000000, titlebg2Bitmap, titlebg2BitmapLen);
			currentMap = 1;
		}
        else if(keyReleased(KEY_START) && currentMap == 1) {
            FadeoutScreen();
            memset(0x06000000, (u8)0x00, 65535);
            memcpy(0x05000000, hk97mainPal, hk97mainPalLen);
            memcpy(0x06000000, hk97mainBitmap, hk97mainBitmapLen);
            currentMap = 2;
        }
        else if(keyReleased(KEY_START) && currentMap == 2) {
            FadeoutScreen();
            memset(0x06000000, (u8)0x00, 65535);
            memcpy(0x05000000, textromPal, textromPalLen);
            *(unsigned int*)0x04000000 = 0x0100; // tile render mode
            *(unsigned int*)0x04000008 = 0x0004;
            memcpy(0x06004000, textromTiles, textromTilesLen);
            currentMap = 3;

            for(u8 k=0;k<4;k++){
                for(u16* l=0x05000002;l<0x05000200;l++){
                    *l = 0x7fff;
                }
                memset(0x06000000, (u8)0x00, 0x7FF);

                u8 yOff = 0;
                for(u16 i=0;textBank01[k][i + (yOff * 15)]!=255;i++){
                    if((i == 15)){
                        i = 0;
                        yOff++;
                        SetLetterInScreen(textBank01[k][i + (yOff * 15)], (i * 2 + (yOff * 32)), i / 16 + yOff);
                    }else{
                        SetLetterInScreen(textBank01[k][i + (yOff * 15)], (i * 2 + (yOff * 32)), i / 16 + yOff);
                    }
                    delayFrame(5);
                }
                delayFrame(12);
                FadeoutScreen();
                delayFrame(6);
            }
        }
	}
	return 0;
}

void SetLetterInScreen(u16 letter, u8 x, u8 y){
    *(tileBuffer + (x) + (64 * y)) = (letter * 2 + (32 * (letter / 16)));
    x++;
    *(tileBuffer + (x) + (64 * y)) = (letter * 2 + 1 + (32 * (letter / 16)));
    x += 31;
    *(tileBuffer + (x) + (64 * y)) = (letter * 2 + 32 + (32 * (letter / 16)));
    x++;
    *(tileBuffer + (x) + (64 * y)) = (letter * 2 + 33 + (32 * (letter / 16)));
}

void FadeinScreen(void){
    for(u8 h=0;h<31;h++){
        *(unsigned short*)0x05000002 += 0x0421;
        delayFrame(3);
    }
    *(unsigned short*)0x05000002 = 0x7fff;
    return;
}

void FadeoutScreen(void){
    for(u8 h=0;h<31;h++){
        for(unsigned short* i=0x05000000;i<=0x05000200;i++){
            if(*i < 0x0421){
                *i = 0x0000;
            }else{
                *i -= 0x0421;
            }
        }
        delayFrame(3);
    }
    delayFrame(30);
    return;
}