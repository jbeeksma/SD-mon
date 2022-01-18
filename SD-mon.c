//
// SD-mon, disk monitor for the TOM6309SBC SD disk
//

// Uncomment this to compile debug code...

//#define DEBUG

#include <cmoc.h>
#include <6309sbc.h>
#include <stdbool.h>
#include "TOM6309SDcard.h"
//#include "../../Bootstrap/JFS/jfs.h"
#include "jfs.h"                                //Changed for use under GIT
#include "SD-CLI.h"

#define INITTRIES 999
//#define ESC 27              //ASCII code for ESC char

//Function Proto's for SD-Mon.c
unsigned char upcase(unsigned char ulc);
long GetBlockNr();
void PrepCS(unsigned char CmdStructure[],unsigned char Cmd, long BlockNr);
int readDisplayBlocks(int argc, char argv[10][30]);
void BlockDisplay(long BlockNr);
void fill_buffer(unsigned char value);

//global variables////////////////////////////////////////////////////////////////////////
static unsigned char BlockBuffer[512];
unsigned char *pBootBlock = 0;
//end global variables////////////////////////////////////////////////////////////////////

long GetBlockNr();
void PrepCS(unsigned char CmdStructure[],unsigned char Cmd, long BlockNr);

int main() {
unsigned char CmdStructure[6];
int SDInitRemaining;
struct sdinfo CardInfo;
int SDStat;
long BlockNr;
unsigned int BlockAddress;
char Command;
char scratch[25];
unsigned char Value;
int SizeMB;
struct csdregister CSData;
unsigned long CSTotalMBytes;
unsigned long StartBlock;

	printf ("\rSD-mon for TOM6309 SD card interface\n");
		
	Command='x';
	while (Command!='Q'){
		printf("\n\nMenu :\n====\n");
		printf("\n B - Write @0000 to boot block");
		printf("\n C - Start CLI");
		printf("\n F - Format SD card with JDOS FS");
		printf("\n I - Init");
		printf("\n R - Read block");
		printf("\n S - Status / info");
		printf("\n W - Write block");
		printf("\n\n Q - Quit SD-mon");
		printf("\n\n Select:");
		Command=upcase(waitkey());
		printf("%c",Command);
		switch (Command) {
		case 'B':
			printf("\nWrite memory to disk");
			BlockNr=0;
			PrepCS(CmdStructure,0,0);
			SDStat=SDWriteBlock(CmdStructure,pBootBlock);
			if (SDStat==SDRDY){
				printf("\nBlock 0x%08lx written",BlockNr);
			} else {
				switch (SDStat){
				case SDWRTFAIL:
					printf("\n Write error.\n");
					break;
				default:
					printf("\n Unknown error.\n");
				} //switch (SDStat...
			} //if (SDStat==SDRDY)
			break;
		case 'C':
		    SD_CLI(SDMonCmds, '>');
		    break;
		case 'F':
			printf("\nFormat SD");
			printf("\nAre you sure? : ");
			Command=upcase(waitkey());
			printf("%c",Command);
			if (Command=='Y') {
			    SDCardTotalBlocks=JDOS_erase(CSData.Csize+1);     //SDCardTotalBlocks is a global variable, this value is available elsewhere.
				printf("\n\aTotal # blocks intialized: %ld",SDCardTotalBlocks);
				break;
			} else {
			    printf("\nCancelled");
			}
			break;
		case 'I':
			CardInfo=SDInit(INITTRIES);
			if (CardInfo.status==SDRDY){
				printf("\n\a%c[1mSD card initialised",ESC);
				if (CardInfo.version2) printf("\nSD card V2"); else printf("\nSD card V1");
				CSData=SDReadCSD();
			} else {
				switch (CardInfo.status){
				case SDERR:
					printf("\nSD card not present or SD comm error.\n");
					break;
				case SDNRDY:
					printf("\nSD card not ready.\n");
					break;
				default:
					printf("\nUnknown error.\n");
				} //switch SDStat
			} //if (CardInfo.status...
			break; //case 'I'...
		case 'R':
			BlockNr=GetBlockNr();
			readDisplayBlocks(BlockNr,0);
			break;
		case 'S':
			printf("\n\nCard info:");
			CSData=SDReadCSD();
			printf("\nCSD structure: ");
			switch(CSData.CSDStructure){
			case (0):
				printf("V1 - Standard capacity");
				break;
			case (1):
				printf("V2 - High/Extended capacity");
				break;
			case (2):
			case (3):
				printf("(Reserved)");
				break;
			default:
				printf("Error in data");
			} //switch CSData.CSDStructure
			
			printf("\nTransfer speed: ");
			switch(CSData.TranSpeed){
			case(50):
				printf("25 Mbit/s");
				break;
			case(90):
				printf("50 Mbit/s");
				break;
			case(11):
				printf("(reserved)");
				break;
			case(43):
				printf("200 Mbit/s");
				break;
			default:
				printf("Error in data");
			} //switch CSData.TranSpeed
			
			CSTotalMBytes=((unsigned long)CSData.Csize+1)/2;
			printf("\nCard size : %l Mb.",CSTotalMBytes);
			
			if (CSData.Copy) {
				printf("\nData is a copy");
			} else {
				printf("\nData is original");
			}
			
			if (CSData.PermWP) {
				printf("\nCard is permanently Write-Protected");
			} 
			
			if (CSData.TempWP) {
				printf("\nCard is temporarily Write-Protected");
			} 
			
			break;
		case 'W':
			BlockNr=GetBlockNr();
			if (BlockNr!=-1){
				printf("\nWhat value to fill the block? (dec) ");
				if (getline(scratch,3)>0){
					Value=(unsigned char)(strtol(scratch,NULL,10))&255;
				} else {
					Value=0x55;
				} //if getline(...
				fillblock(BlockNr,Value);
/*				fill_buffer(BlockBuffer,Value);
				PrepCS(CmdStructure,SDCMDWriteBlock,BlockNr);
				SDStat=SDWriteBlock(CmdStructure,BlockBuffer);
				if (SDStat==SDRDY){
					printf("\nBlock 0x%08lx written",BlockNr);
				} else {
					switch (SDStat){
					case SDWRTFAIL:
						printf("\n Write error.\n");
						break;
					default:
						printf("\n Unknown error.\n");
					} //switch (SDStat...
				} //if (SDStat==SDRDY)
*/
			} //if (BlockNr...
			break;
		case 'Q':
			printf("\nOK, quitting...");
			exit(0);
			break;
		default:
			printf("\n\aIllegal command.");
		} //switch (command)
	} //while
	return(0); 
} //main
	
unsigned char upcase(unsigned char ulc) 
{
	if(ulc>'_') ulc=ulc-0x20;
	return(ulc);
}

long GetBlockNr()
{
char cmdline[11];
long BlockNr;

	printf("\nEnter block #");
	if (getline(cmdline, 10)!=0)
	{
		BlockNr=strtol(cmdline,NULL,10);
		return (BlockNr); //returns true
	} else{
		return (-1); //returns false - nothing input
	}

}

void PrepCS(unsigned char CmdStructure[],unsigned char Cmd, long BlockNr)
{
	//The CS_ReadBlock and CS_WriteBlock get a Command Sructure with just 
	//the block number in byte 0..3, the routines compile the correct command structure from that
	CmdStructure[0] = (unsigned char)(BlockNr/16777216)%255;	//1st byte of block #
	CmdStructure[1] = (unsigned char)(BlockNr/65536)%255;	//2nd byte of block #
	CmdStructure[2] = (unsigned char)(BlockNr/256)%255;	//3rd byte of block #
	CmdStructure[3] = (unsigned char)BlockNr%255;		//last byte of block #
	CmdStructure[4] = 0; //CRC but not checked...
	CmdStructure[5] = 0; //CRC but not checked...
#ifdef DEBUG
	printf("\nCMDBuf assembled : %02x %02x %02x %02x %02x %02x",CmdStructure[0],CmdStructure[1],CmdStructure[2],CmdStructure[3],CmdStructure[4],CmdStructure[5]);
#endif
}

/**
    readDisplayBlocks
    Will read block firstblock throug lastblock and display the contents in hex format
    If lastblock == 0, only firstblock will be displayed.
*/
    
int readDisplayBlocks(int argc, char argv[10][30])
{
unsigned char CmdStructure[6];
long blockNr,firstblock,lastblock;
int SDStat;
int CLIStat;

    firstblock=(long)atol(argv[1]);                                     //Interpret argv[1] as first block
    lastblock=(long)atol(argv[2]);                                      //Interpret argv[2] as last block
    for (blockNr=firstblock; blockNr<=lastblock; blockNr++) {           //Loop through, if lastblock==0 then only 1 block is displayed
        PrepCS(CmdStructure,SDCMDReadBlock,blockNr);                    //Prepare the command structure
        SDStat=SDReadBlock(CmdStructure,BlockBuffer);                   //Call low-level read routine, passes BlockBuffer to ASM routine
        if (SDStat==SDRDY){                                             //Check the return code
            BlockDisplay(blockNr);                                      //If OK, display the block
            CLIStat=E_CLI_OK;
        } else {
            switch (SDStat){                                            //Print error message as appropriate
            case SDERR:
                CLIStat=E_CLI_SDERR;
                printf("\n SD card not present or SD comm error.\n");
                break;
            case SDNRDY:
                printf("\n SD card not ready.\n");
            } //switch (SDStat...
            return(CLIStat);
        } //if (SDStat==SDRDY)
    }  
}

/**
    BlockDisplay dumps the contents of a 512 byte disk block in a 32x16 matrix in hex and ASCII format to the terminal
*/
void BlockDisplay(long BlockNr)                                         //Uses the global BlockBuffer[]
{
int Lines, Cols;
unsigned char Cell;

	printf("\n");
	for (Lines=0;Lines<32;Lines++){                                     //Print 32 lines of 16 columns
		//if (Lines%8==0) printf("\n");
		printf("\n0x%08lx %04x\t",BlockNr,16*Lines);
		for (Cols=0;Cols<16;Cols++){
			if ((Cols%8)==0) printf(" ");                               //Print gutter after 8 cols
			printf("%02x ",BlockBuffer[16*Lines+Cols]);                 //Print hex value
		}
		for (Cols=0;Cols<16;Cols++){
			if ((Cols%8)==0) printf(" ");                               //Print gutter after 8 cols
			Cell=BlockBuffer[16*Lines+Cols];
			//if (Cell>127) Cell='.';                                     //Over ASCII 127 chars are undefined
			if (Cell<0x20) Cell='.';                                    //Replace unprintable chars with dots
			printf("%c", Cell);                                         //Print char value
		}
	}	
}

void fill_buffer(unsigned char value)                                   //Uses the global BlockBuffer
{
int count;

	for(count=0;count<512;count++)
	{
		BlockBuffer[count]=value;
	}
}

//Replace the standard _exit routine from the usim library
void exit(int status)
{
	asm
	{
INISTK	IMPORT
	LDS	INISTK,PCR
	JMP	WOZMON
	}
} 

#include "TOM6309SDcard.c"
#include "jfs.c"
#include "SD-CLI.c"

//#include <../TOM6309.c>