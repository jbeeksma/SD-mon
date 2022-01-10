//
// Defines and protos for TOM6309SDcard.c
// Jan 2021, Jacob Beeksma
//

#ifndef _H_TOM6309SDcard
#define _H_TOM6309SDcard
#include <stdbool.h>

//Pointer table to low level routines

#define SPI_ReadBlock_ptr	0xFF96
#define SPI_Read_ptr	    0xFF94
#define SD_Initialise_ptr	0xFFA4
#define SD_SendCmd_ptr	    0xFFA6
#define SD_GetR7_ptr	    0xFFA8
#define SD_ReadBlock_ptr	0xFFAA
#define SD_WriteBlock_ptr	0xFFAC
#define SD_WaitReady_ptr	0xFFAE

//command buffers for various SD commands
//#define	SD_XCMD8		0xEA77 make your own, list in monitor is incomplete
//The numbered commands are complete command structures used in asm routines
//The named commands are command numbers used in C BuildCMDStructure

const unsigned char SDCMD8[]={0x48,0x00,0x00,0x01,0xAA,0x87};
const unsigned char SDCMD9[]={0x49,0x00,0x00,0x00,0x00,0x00};		
const unsigned char SDCMDReadBlock = 17;
const unsigned char SDCMDWriteBlock = 24;

//R1 Error bits table
#define R1SDBUSY		0x80
#define R1PARAMERR		0x40
#define R1ADDRERR		0x20
#define R1ERSEQERR		0x10
#define R1COMCRCERR		0x08
#define R1ILLEGCMD		0x04
#define R1ERARESET		0x02
#define R1IDLE		    0x01

//I/O port constants
#define IOPORT		    0xE050
#define IO_SDCS  		%00010000
#define IO_SDBSY		%01000000
#define SDPORT		    0xE030

//SD status/error codes - return values

#define SDRDY		    0	    //SD card ready - interface initialized
#define SDERR		    1	    //SD not present or comm error
#define SDNRDY		    2	    //SD not ready
#define SDCMD17FAIL	    3	    //SD CMD17 failed
#define SDNOTOK		    4	    //SD no read token received
#define SDWRTFAIL	    5	    //SD write failed
#define SDTESTOK        6       //SD block readback test OK
#define SDTESTNOK       7       //SD block readback test not OK
#define SDREADFAIL      8       //SD block read failed

//SD command codes 
#define	SD_SEND_CSD	    0x49	//SD Cmd 9 +$40

//SD card data block size in bytes
#define SDBlockSize     512 

//structures for SD card info
typedef struct{
	int status;
	bool version2;
} sdinfo;			//basic infro from SDInit

typedef struct{
	unsigned char 	CSDStructure;
	unsigned char	TranSpeed;
	unsigned long 	Csize;
	bool		Copy;
	bool		PermWP;
	bool		TempWP;
} csdregister;

//SD card related global variables
long SDCardTotalBlocks;

//function protos
struct sdinfo SDInit(int NrTries);					                        //try NrTries to init SD
struct sdinfo SD_Init(unsigned char ResultBuffer[]);                        //initialize SD-card interface
int SDReadBlock(unsigned char CmdBuffer[],unsigned char BlockBuffer[]);     //Read block
int SDWriteBlock(unsigned char CmdBuffer[],unsigned char BlockBuffer[]); 	//Write block
struct csdregister SDReadCSD();                                             //Read CSD data

#endif //_H_TOM6309SDcard