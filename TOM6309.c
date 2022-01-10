//
//CMOC I/O extensions for Tom LeMense's 6306 SBC
//ROM version 1.41
//This file goes into /usr/local/share/cmoc
//There is also a header file with prototypes that goes into /usr/local/share/cmoc/include

//console character output routine used by printf() etc.
//
void Outch()
{
	char ch;
	asm
	{
	CMPA	#$0A
	BNE	NOLF
	LBSR	$E731	PUTCR
	BRA	DONE 
NOLF	LBSR 	$E71F	PUTCH
DONE	
	}
}

// Waits for a key to be pressed and returns its code.
//
char waitkey()
{
	char ch;
   	asm
   	{
      		JSR	[$FF44]	//GETCH - Wait for a character
      		STA	:ch
   	}
   	return(ch);
}

// Checks if keyboard input available. Return 0 if not, char if available
//
char checkkey()
{
	char ch;
	asm
	{
		JSR	[$FF46]	//GETCH1 - attempt to get char
		BVC	@GOTCH	//Actually received a char
		CLRA		//Return NULL if nothing received
@GOTCH		STA	:ch	//done!
	}
	return ch;	
}

// Input a text line
// returns 0 if no line entered (ESC / ^z)
int getline(char* thestring, char maxlength)
{
int nrchars;
char newchar;

	nrchars=0;
	do {
		newchar=waitkey();
		switch (newchar)
		{
		case ESC:			//ESC -> cancel input
			nrchars=0;
			thestring[0]=0;	//End of String in 1st pos
			break;
		case CR:			//End input
			break;
		case BS:			//Backspace only if something in buffer
			if (nrchars>0){
				nrchars--;
				thestring[nrchars]=0; //erase last char	
				printf("\b \b"); //echo a backspace
			} else {		//begin of buffer reached
				printf("%c",BELL);
			}
			break;
		default:			//normal character
			printf("%c",newchar);	//echo the new character
			if (nrchars<maxlength){ //still room in buffer
				thestring[nrchars++]=newchar;	//add char and inc pointer
				thestring[nrchars]=0;	//add EoS
			} else {				//buffer full, do not accept
				printf("%c",BELL);
			}
		}
	} while ((newchar!=CR)&&(newchar!=ESC)); //this ends the input	
	return(nrchars);
}

/*/Replace the standars _exit routine from the usim library
void exit(int status)
{
	asm
	{
INISTK	IMPORT
	LDS	INISTK,PCR
	RTS
	}
}
*/