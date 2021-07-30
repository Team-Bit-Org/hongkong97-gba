
//{{BLOCK(textrom)

//======================================================================
//
//	textrom, 256x256@4, 
//	+ palette 256 entries, not compressed
//	+ 1024 tiles not compressed
//	Total size: 512 + 32768 = 33280
//
//	Time-stamp: 2021-07-30, 11:09:05
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.3
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_TEXTROM_H
#define GRIT_TEXTROM_H

#define textromTilesLen 32768
extern const unsigned short textromTiles[16384];

#define textromPalLen 512
extern const unsigned short textromPal[256];

#endif // GRIT_TEXTROM_H

//}}BLOCK(textrom)
