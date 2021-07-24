#include "Intellisense.h"
#include "gba_macros.h"
#include "gba_types.h"
#include "gba_gfx.h"
#include "gba_drawing.h"
#include "gba_mathUtil.h"
#include "gba_input.h"

#include <string.h>

// image
#include "titlebg.h"
#include "titlebg2.h"

u16 __currKeys, __prevKeys;

int main()
{
	//set GBA rendering context to MODE 3 Bitmap Rendering
	*(unsigned int*)0x04000000 = 0x0403;

	memcpy(SCREENBUFFER, titlebgBitmap, titlebgBitmapLen);

	while(1){
		vsync();
		PollKeys();
		if(keyReleased(START)) {
			memcpy(SCREENBUFFER, titlebg2Bitmap, titlebg2BitmapLen); 
		}
	}
	return 0;
}