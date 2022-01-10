/*  TOM6309.h - Utility functions for interface with with Tom DeMenses 6309 SBC

	By Jacob Beeksma <jacob@beeksma.nl>
	This file is in the public domain.
*/
/*
	CMOC I/O extensions prototypes for Tom DeMense's 6306 SBC
	ROM version 1.41
	This file goes into /usr/local/share/cmoc/include
	There is also a source file TOM6309.h with source code that goes into /usr/local/share/cmoc
*/

#ifndef _H_TOM6309
#define _H_TOM6309

//implement ISO C bool (defines bool type, true en false)
//
#include <stdbool.h>

//console character output routine used by printf() etc.
//
void Outch();

// Check if a key is pressed on the console.
// Returns 0 if no key is currently pressed. Returns keycode if pressed.
//
char checkkey();

// Waits for a console key to be pressed and returns its code.
//
char waitkey();

// Input a text line from console into buffer
// returns 0 if no line was input, nr characters when actual input is present
//
int getline(char * buffer, char maxlength);

//
// Names for values that can be passed to isKeyPressed()
// to test if a CoCo key is down or not.
// These values do not apply on the Dragon, whose keyboard grid is different.
//

#define	CR	0x0D
#define	BS	0x08	
#define	ESC	0x1B
#define	BELL	0x07

//memory for old putch routine address
void *oldCHROOT;

#define WOZMON	0xF0B0

#endif //_H_TOM6309
