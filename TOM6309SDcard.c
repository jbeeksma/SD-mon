//
// C library functions for TOM6309SBC Sd card interace
// Jan 2021, Jacob Beeksma
// V0.1 -- work in progress
//

#include <cmoc.h> //include cmoc.h if not already loaded
#include <6309sbc.h> //include TOM6309.h if not already loaded
#include "TOM6309SDcard.h"
#include <stdbool.h>

// For debugging purposes ony: (These are addresses of the routines, not jump table entries!)
#define PUTCH	$E71F
#define PUTMSG	$E774
#define PUTBYTE	$E74E
#define PUTHEX	$E758
#define PUTCR	$E731
#define PUTWORD	$E745
#define PUTSPACE	$E73D
#define SPI_ReadBlock $EA51
#define SPI_Read	$EA49

#ifdef DEBUG
#define VERBOSE	1
#else
#define VERBOSE 	0
#endif

int SDInitRemaining;
int SDStat;

//
// InitSD tries n times to initialize the SD device
//
struct sdinfo SDInit(int NrTries)
{
unsigned char SDResult[5];
struct sdinfo CardInfo;
int NrTriesUsed;

	SDResult[0]=SDResult[1]=SDResult[2]=SDResult[3]=SDResult[4]=0;
	SDInitRemaining=NrTries;
	do {
		SDInitRemaining--;
		SDResult[0]=(unsigned char)SDInitRemaining;
		CardInfo=SD_Init(SDResult);
	} while(!(CardInfo.status==SDRDY) && SDInitRemaining>0);
	NrTriesUsed=NrTries-SDInitRemaining;
	if (NrTriesUsed==1) {
		printf("\nBingo!"); 
	}else if (SDInitRemaining==0) {
		printf("\nInit failed...");
	} else {
		printf("\nDone in %d tries",NrTriesUsed);
	}
	return(CardInfo);
}

//
// SD_init() tries to initialize the SD card interface
// returns TRUE (1) if successful
// returns FALSE (0) if no card or comm error
//
struct sdinfo SD_Init(unsigned char ResultBuffer[])
{
struct sdinfo ThisCard;
int InitStat;
int version;

	asm
	{
	PSHS	X,D		            //save index register and A,B
	PSHSW
	LDX	ResultBuffer
	LDF	#VERBOSE		        //type of status to console (depends on DEBUG)
	LDD	#SDRDY		            //positive result code
	STD	InitStat		        //Initially set status to SDRDY
	JSR	[SD_Initialise_ptr]	    //indirect via jump table
	BVC	InitDone		        //If V cleared --> success
// Error handling
	LDD	#SDERR		            //indicate failure
	STD	InitStat		        //alas, indicate failure InitSuccess=FALSE
InitDone	
// Check V2
	CLR	version		            //Assume card is V1 (false)
	PSHS	U
	LDU	SDCMD8		            //SDCMD8 command sequence
	JSR	[SD_SendCmd_ptr]	    //Execute CMD8
	PULS 	U
	OIM	IO_SDCS,IOPORT	        //Negate CS
	BITA	#R1ILLEGCMD	        //R1 bit 2 = Illegal Command
	BNE	@ISV2		            //Is V2, set version to true
	INC	version		            //set version to 1 (true)
@ISV2
	PULSW
	PULS	X,D		            //retrieve index register and A,B
	} 
	ThisCard.status=InitStat;
#ifdef DEBIG
printf("\nInitstat : %d",InitStat);
#endif
	ThisCard.version2=version;
	return(ThisCard);
}

int SDReadBlock(unsigned char CB[], unsigned char BlockBuffer[])
{
int ReadStat;

	
#ifdef DEBUG
	printf("\n SD_ReadBlock: Cmdbuf = [%02x %02x %02x %02x %02x %02x] &blockbuf=%x ",CB[0],CB[1],CB[2],CB[3],CB[4],CB[5],BlockBuffer );
#endif //DEBUG

	asm
	{
	PSHS	D,U,X,Y		    //save D,X,Y register
	PSHSW			        // and also W (E,F) register
	LDD	#SDRDY		        //positive result code
	STD	ReadStat		    //assume read will work
	LDX	CB		            //6 bit command buffer 0,x .. 3,x = block #
	LDY	BlockBuffer	        //buffer to receive data
	JSR	[SD_ReadBlock_ptr]	//indirect JSR via jump table
	BVC	ReadDone		    //all good skip error business
// Error handling
	CMPA	#0		        //check A register
	BEQ	NoToken		        //if A=0: No READ token
CMD17done	LDD	#SDREADFAIL	//if a<>0: Command 51 failed
	STD	ReadStat		    //save error code in status	
	BRA	ReadDone		    //exit
NoToken	LDD	#SDNOTOK		//no token received
	STD	ReadStat		    //save error code in status
ReadDone	
	PULSW			        //retrieve registers
	PULS	D,U,X,Y		    //retrieve registers
	}
	return(ReadStat);
}

int SDWriteBlock(unsigned char CB[],unsigned char BlockBuffer[])
{
int WriteStat;

#ifdef DEBUG
	printf("\n SD_WriteBlock: Cmdbuf = [%02x%02x %02x%02x %02x%02x] &blockbuf=%x ",CB[0],CB[1],CB[2],CB[3],CB[4],CB[5],BlockBuffer );
#endif //DEBUG

	asm
	{
	PSHS	D,U,X,Y		//save D,X,Y register
	PSHSW			// and also W (E,F) register
	LDD	#SDRDY		//positive result code
	STD	WriteStat		//assume read will work
	LDX	CB		//6 bit command buffer 0,x .. 3,x = block #
	LDY	BlockBuffer	//buffer to receive data
	JSR	[SD_WriteBlock_ptr]	//indirect JSR via jump table
	BVC	WriteDone		//all good skip error business
// Error handling
WriteFail	LDD	#SDWRTFAIL	//if a<>0: Command 17 failed
	STD	WriteStat		//save error code in status	
WriteDone	JSR	[SD_WaitReady_ptr]	//Wait until SD ready
	PULSW			//retrieve registers
	PULS	D,U,X,Y		//retrieve registers
	}
	return(WriteStat);
}

struct csdregister SDReadCSD()
{
struct csdregister ThisCard;
unsigned char CmdBuffer[6];
unsigned int RESBUF[2];
unsigned char CSDBuffer[16];
unsigned long DevSize;
unsigned char ByteNo;
unsigned int ResultCode;
unsigned long p1, p2, p3;

	CmdBuffer[0]=SD_SEND_CSD;	//command code
	CmdBuffer[1]=0;
	CmdBuffer[2]=0;
	CmdBuffer[3]=0;
	CmdBuffer[4]=0;
	CmdBuffer[5]=1;		//Cmd buffer filled...
	ResultCode=0x00;		//Preload Result code with OK...

for (ByteNo=0;ByteNo<16;ByteNo++) {
	CSDBuffer[ByteNo]=ByteNo;
}
	asm
	{
	PSHS	X,Y,D		        //Save X, Y, D

	LEAY	CSDBuffer		    //Point Y to CSD buffer
	PSHS	U		            //Preserve U!!!
	LEAU	CmdBuffer		    //Point U to CmdBuffer
	JSR	[SD_SendCmd_ptr]	    //Send SD command
	STA	ResultCode	            //Clears error if succesful
	PULS	U		            //Restore U
	LDB	#10		                //init attempt counter
READRSP	JSR	SPI_Read		    //read the response
	TSTA			            //examine the received byte
	BPL	GOTRSP		            //non-negative, got a response!
	DECB 			            //negative, decrement attempt counter
	BNE	READRSP		            //keep trying for 10 attempts
GOTRSP	STA	,y+		            //save first non-negative byte
	LDX	#15		                //put transfer count (16) into X
	JSR	[SPI_ReadBlock_ptr]	    //transfer (X) bytes to buffer at Y
	OIM	#%00010000,IOPORT	    //negate SD card select  
	                       
	PULS	X,Y,D		        //Retrieve X, Y, D 
	}

#ifdef DEBUG
printf("\nCSD data\t: ");
for (ByteNo=0;ByteNo<16;ByteNo++) {
	printf(" %02x", CSDBuffer[ByteNo]);
}
#endif //ifdef DEBUG
	
	ThisCard.CSDStructure=CSDBuffer[0]>>6;	//First 2 bits of byte 0
	
	ThisCard.TranSpeed=CSDBuffer[3];	    //byte 3

	p1=(unsigned long)CSDBuffer[7]&63;	    //lowest 6 bits of byte 7
	p1=p1<<16;			                    //make room for another 16 bits
	p2=(unsigned long)CSDBuffer[8]<<8;	    //add whole byte 8
	p3=(unsigned long)CSDBuffer[9];	        //plus byte 9
	DevSize=p1+p2+p3;
	ThisCard.Csize=DevSize;                 //Size of card in 512 byte blocks?
	
	ThisCard.Copy=(CSDBuffer[14]&64); 	    //true if bit 7 is set
	
	ThisCard.PermWP=(CSDBuffer[14]&32); 	//true if bit 6 is set
	
	ThisCard.TempWP=(CSDBuffer[14]&16); 	//true if bit 5 is set

	return(ThisCard);	
}