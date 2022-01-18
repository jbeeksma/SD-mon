/*
	JFS.c
	
	Source code for Jacobs File System for 6309 SBC SD card.
	V0.0 (c) 2021 Jacob Beeksma.
*/

#include "jfs.h"
#include "TOM6309SDcard.h"
#include <stdbool.h>

/**
    JDOS_erase will format an SD card filesystem.
    It will first erase the boot block, abort if that fails.
    If success, it will then initialize the empty chain header.
        Test the header block first, abort if fail.
            Test the empty chain header block, abort if fail.
                Initialize the empty chain, one block at a time.
                ...
*/
long JDOS_erase(long maxblocks)
{
long blocknr, blockcnt;
long newdir,newpart;
unsigned char blocktype=T_EMPTYBLK;

	if (!erase_test_block(A_BOOTBLOCK)) {		//erase and test boot block
		printerr("Boot block can not be initialized.\nAborted.");
	} else {
        printf("\nBootblock erased");
		if (!erase_test_block(A_EMPTYCHN)) {	//erase and test empty chain header
		printerr("Empty chain can not be initialized.\nAborted.");
	    } else {
	        printf("\nEmpty chain header block erased");
//	        init_ec_header(); --> only after first empty block is added to the chain. all 0's is good for now.
	    	if (!erase_test_block(A_PARTMAP)) {	//erase and test partition map block
		        printerr("Partion Map can not be initialized.\nAborted.");
        	} else {
	            printf("\nPartmap erased");
	            if (!erase_test_block(A_BADBLKHDR)) {	//erase and test bad block header
		            printerr("Bad block header can not be initialized.\nAborted.");
        	    } else {
        	        init_badblk_hdr();
        	        blockcnt=0;
        	        printf("\n");
//FIXME: formatting whole device takes too long, cut down to 1024 blocks for testing
//      	        for (blocknr=4;blocknr<=maxblocks;blocknr++) {  
        	        for (blocknr=4;blocknr<=10;blocknr++) {       //10 = test value saves time during dev ;-)
                        if (!erase_test_block(blocknr)) {
                            printf("\nBlock %ld bad.\n",blocknr);
                            add_bad_block(blocknr);
                        } else {
                            printf("\r%08lx",blocknr);
                            blockcnt++;
                            add_to_ec(blocknr);
                        } // !erase_test         	        
        	        } //for (blocknr... - loop to initialize empty chain
                    //Empty Chain intialized, now finish rest of fs initialization
        	        init_partmap();
printf("\nPartmap initialized.");
        	        newdir=createDir("/",NOATTRIB, NOPARENT);
printf("\nCreated root dir.");
        	        newpart=createPartition('c',"Root",NOTBOOTABLE,newdir);         //drive letter c:, not bootable yet, add root dir
printf("\nPartion header created, root dir added.");
        	        addpart(newpart);               //Add partititon to partition table
        	    }
            }
        }
	return blockcnt; //return nr of successfully erased blocks
	}	
}

bool erase_test_block(long BlockNr)
{
int TestStat;

    if (TestStat=fillblock(BlockNr,0x0F)==SDRDY)
    {
        if (testblock(BlockNr,0x0F)==SDTESTOK)
        {
            if (fillblock(BlockNr,0x00)==SDRDY)
            {
                if (TestStat=(testblock(BlockNr,0x00)==SDTESTOK))
                {
                    //printf("\rBlock %ld tested OK.",BlockNr);
                    return(true);
                } else {
                    //printf("\rBlock %ld error %0X", BlockNr, TestStat);
                }
            }
        }
    } else {
        return (TestStat);
    }
}
	
void printerr(const char * errormessage)
{
    printf("\n\a%s\n",errormessage);
}

int fillblock(long BlockNr, unsigned char Value)
{
int SDStat;
    fill_buffer(Value);
    SDStat=writeblock(BlockNr);
    return SDStat;
}

int writeblock(long BlockNr)
{ 
unsigned char CmdStructure[6];
   
    PrepCS(CmdStructure,SDCMDWriteBlock,BlockNr);
    SDStat=SDWriteBlock(CmdStructure,BlockBuffer);      //BlockBuffer is global!
    if (SDStat!=SDRDY){
        switch (SDStat){
        case SDWRTFAIL:
            printf("\n Write error.\n");
            break;
        default:
            printf("\n Unknown error.\n");
        } //switch (SDStat...
    } //if (SDStat!=SDRDY)
    return SDStat;
}

//
//Testblock reads block *BlockNr* from the disk and tests if it is filled with *value*
//Returns SDTESTOK if OK, SDTESTNOK if any byte is not equal to *value*
//Returns SDREADFAIL if there was an error reading the block from disk
//
int testblock(long blocknr,unsigned char value)
{
int bytenr;
    
    SDStat=readblock(blocknr);
    if (SDStat==SDRDY){
        for (bytenr=0;bytenr<SDBlockSize;bytenr++){
            if (BlockBuffer[bytenr]!=value) return SDTESTNOK; //Terminate with error
        }
    } else {
        return(SDREADFAIL);                                   //Terminate with error  
    }
    return SDTESTOK;                                          //Block is OK - end.
}

int readblock(long blocknr)
{
unsigned char CmdStructure[6];
int SDStat;

    PrepCS(CmdStructure,SDCMDReadBlock,blocknr);
    SDStat=SDReadBlock(CmdStructure,BlockBuffer);           //Beware: the blockbuffer is a global variable!
    return SDStat;                                          //but assembler routine needs the address
}

/** 
    Initialize the partition Map
    At this stage the partmap is empty, the first entry is added when the root partition is created
*/
void init_partmap()
{
union pm_transfer pm_t;

    pm_t.buffer=&BlockBuffer[0];            //Link pm_t.buffer to physical address of BlockBuffer
    pm_t.pmdata->blocktype=T_PARTHDR;       //Define block as Bad Block Header
    pm_t.pmdata->no_parts=0;                //No partitions yet
    pm_t.pmdata->parthdr[0]=0;              //Indicates no (more) partitions
    writeblock(A_PARTMAP);                  //Write the data to appropriate block
}

/**
    Initialize bad block header. 
    At this point it is empty because the disk is not formatted yet.
*/
void init_badblk_hdr()
{
union bbh_transfer bbh_t;

    bbh_t.buffer=&BlockBuffer[0];           //Link pm_t.buffer to physical address of BlockBuffer
    bbh_t.bbhdata->blocktype=T_BADBLKHDR;   //Define block as Bad Block Header
    bbh_t.bbhdata->extb_block=0;            //No extension blocks yet
    bbh_t.bbhdata->nrbadblocks=0;           //No bad blocks yet
    bbh_t.bbhdata->badblock[0]=0;           //Indicates end of list
    writeblock(A_BADBLKHDR);                //Write the data to appropriate block
} 

/**
    Add a bad block to the bad block list. 
    First determine where 
*/
void add_bad_block(long blocknr)
{
union bbh_transfer bbh_t;
union bbx_transfer bbx_t;
long currentbblock;
char bbindex;
long nrbadblocks;

    currentbblock=A_BADBLKHDR;                      //start by looking in the bad block header
    bbh_t.buffer=&BlockBuffer[0];                   //Link bbh_t.buffer to physical address of BlockBuffer
    readblock(currentbblock);                       //read data from the Bad Block Header/Extension 
    nrbadblocks=bbh_t.bbhdata->nrbadblocks;         //Remember the total # bad block known
    while (bbh_t.bbhdata->extb_block != 0){         //There is a next Bad Block List block
        currentbblock=bbh_t.bbhdata->extb_block;    //Determine which is the next block
        readblock(currentbblock);                   //Read new data from the new block
    }                                               //---At this point we have the data drom the last Bad Block List block
    bbindex=(char)nrbadblocks%MAXBBLOCKS;           //index of the last stored bad block in this block
    bbh_t.bbhdata->badblock[bbindex+1]=blocknr;     //Add the number of the bad block to the list
    writeblock(currentbblock);                      //write modified data back
    
    // now update total # of bad blocks in BB Header
    
    readblock(A_BADBLKHDR);                         //Read the bad block header
    bbh_t.bbhdata->nrbadblocks=nrbadblocks+1;       //Set the new value
    writeblock(A_BADBLKHDR);                        //Write back updated Bad Block Header
printf("\n0x%08lx is a bad block, %ld total bad blocks", blocknr, bbh_t.bbhdata->nrbadblocks);
}

/**
    Add te new block to the empty chain.
    Blocks are appended to the end of the empty chain
*/
void add_to_ec(long newblock)
{
//blockbuffer is statical/global defined
long prevlastblock;

    prevlastblock=GetLastECBlockNr();               //Get block number of last block in Empty Chain
    if (prevlastblock==0) {                         //Then this is the first empty block in chain
        init_ec_header(newblock);                   //New block is attached to EC Header
    } else {
        UpdateLastECBlock(prevlastblock,newblock);  //New block is attached to last block of EC
    }
    UpdateCurrentBlock(prevlastblock,newblock);     //Add Blocktype and address of previous empty block
    UpdateECHeader(newblock);                       //Write new last block into EC Header
}

long GetLastECBlockNr()                             //Get block number of last block in Empty Chain
{
union ech_transfer ech_t ;                          //blockbuffer is static/global defined

    readblock(A_EMPTYCHN);                          //Read the EC Header block
    ech_t.buffer=&BlockBuffer[0];                   //pointer to first byte of buffer
    return ech_t.ecdata->last_eb;
}

/**
    Initialises the Empty Chain Header. This is done after the first empty block is initialized. 
    Its address is added as both the first and the last empty block in the chain.
    lastEBlock will be modified ater by UpdateECHeader().
*/
void init_ec_header(long firstEBlock)
{
union ech_transfer ech_t ;

    
    ech_t.buffer=&BlockBuffer[0];           //Link ech_t buffer to physical address of BlockBuffer
    ech_t.ecdata->blocktype=T_EMPTYHDR;     //Set block type to Empty Chain Header
    ech_t.ecdata->first_eb=firstEBlock;     //Set link to first empty block in chain
    ech_t.ecdata->last_eb=firstEBlock;      //is also last empty block 
    writeblock(A_EMPTYCHN);                 //empty chain header initialized with first empty block
}

/**
    In order to add a new block to the empty chain, the previous last block is linked.
    The 0 indicating it was the (former) last block in the chain is replaced by
    the block number of the new last block in the Empty Chain
*/
void UpdateLastECBlock(long prevlastblock,long newblock)
{
union eb_transfer eb_t ;

    readblock(prevlastblock);               //read old contents of previous last block
    eb_t.buffer=&BlockBuffer[0];            //et_t.buffer is now a pointer to the start of BlockBuffer
    eb_t.ebdata->next_eb=newblock;          //modify content: (add new last block in the chain)
    writeblock(prevlastblock);              //write back previous last block
}

/**
    A new block is linked into the empty chain.
    since it is the last block in the chain, next_eb is set to 0
    prev_eb is the backlink to the previous block in the chain.
*/
void UpdateCurrentBlock(long prevlastblock, long newblock)
{
union eb_transfer eb_t ;

    eb_t.buffer=&BlockBuffer[0];            //Link eb_t buffer to physical address of BlockBuffer
    eb_t.ebdata->blocktype=T_EMPTYBLK;      //Initialize the empty block data structure
    eb_t.ebdata->next_eb=0;                 //Is now the last block in chain
    eb_t.ebdata->prev_eb=prevlastblock;     //Link back to previous block in chain. 
    writeblock(newblock);                   //write initialized new block in chain
}

void UpdateECHeader(long newblock)
{
union ech_transfer ech_t ;

    readblock(A_EMPTYCHN);                  //Read current content of empty chain block header    
    ech_t.buffer=&BlockBuffer[0];           //Link ech_t buffer to physical address of BlockBuffer   
    ech_t.ecdata->last_eb=newblock;         //Modify EBHeader data structure with new last block  
    writeblock(A_EMPTYCHN);                 //Empty chain header initialized with first empty block
}

/**
    Get an empty block from the empty chain.
    Empty blocks are taken from the start of the chain.
*/
long getblock()
{
union ech_transfer ech_t;
long emptyblock;

    readblock(A_EMPTYCHN);                  //Read current content of empty chain block header    
    ech_t.buffer=&BlockBuffer[0];           //Link ech_t buffer to physical address of BlockBuffer  
    emptyblock=ech_t.ecdata->first_eb;      //Read address of first available empty block
printf("\nFirst empty block available is 0x%08lx.",emptyblock);
    if (emptyblock!=0) {                    //Empty block available
        eb_unlink(emptyblock);              //Remove it from the empty chain
        return(emptyblock);                 //Return the block address
    } else {                                //No empty block available
        return(0);
    }
}

/**
    Remove a block from the empty chain
    blocknr must be a valid block number from the empty chain.
    The predecessor and successor are derived from the empty block itself. 
*/
void eb_unlink(long blocknr)
{
union eb_transfer eb_t;                     //Empty block data structure
union ech_transfer ech_t;                   //Empty chain header structure
long pred,succ;                             //Predecessor and successor blocks
    
    readblock(blocknr);                     //Get the specified empty block
    eb_t.buffer=&BlockBuffer[0];            //Map eb_t onto the data block
    succ=eb_t.ebdata->next_eb;              //Retrieve block address of successor
    pred=eb_t.ebdata->prev_eb;              //Retrieve block address of predecessor
printf("\neb_unlink block 0x%08lx, pred= 0x%08lx, succ= 0x%08lx",blocknr,pred,succ);
    if (succ==0) {                           //This was the last empty block in the chain
        UpdateECHeader(pred);               //Record predecessor as last block in empty chain
    } else {                                //If not, the successor must be updated
        readblock(succ);                    //Get the data (eb_t.buffer ASSUMED to be already OK)
        eb_t.ebdata->prev_eb=pred;          //Backlink to the predecessor of the removed block
        writeblock(succ);                   //Successor block updated
    }                               //So far the successor part.
    if (pred==0){                           //blocknr was the first in the empty chain
printf("\neb_unlink: was first eb in chain, setting start of ec to 0x%08lx.", succ);
        ec_modfirst(succ);                  //Register succ as new first empty block in the empty chian
    } else {                                //blocknr was not the first empty block
        readblock(pred);                    //Get the pred block
        eb_t.ebdata->next_eb=succ;          //Register the successor of blocknr as the new successor of pred
        writeblock(pred);                   //Update pred block
    }                                       //Bookkeeping done!
}

/**
    Add blocknr as the first empty block in the empty chain
*/
void ec_modfirst(long blocknr)
{
union ech_transfer ech_t ;

    readblock(A_EMPTYCHN);                  //Read current content of empty chain block header    
    ech_t.buffer=&BlockBuffer[0];           //Link ech_t buffer to physical address of BlockBuffer   
    ech_t.ecdata->first_eb=blocknr;         //Modify EBHeader data structure with new last block  
    writeblock(A_EMPTYCHN);                 //Empty chain header initialized with first empty block
}

/**
    Create a new directory with specified name and attributes under parentdir
    createDir returns the block address of the new dir structure, 
    it must be added to the parent dir separately
*/
long createDir(char* dirname, unsigned char attribs, long parentdir)
{
union dh_transfer dh_t;
long diraddress;

   if ((diraddress=getblock())!=0){             //non-zero means a block has been made available, 0 means no block
        dh_t.buffer=&BlockBuffer[0];            //Link dh_t buffer to physical address of BlockBuffer
        dh_t.dhdata->blocktype=T_DIRHDR;        //Blocktype directory header or 0xD0
        dh_t.dhdata->attibutes=attribs;         //Assign the specified attribs
        strcpy(dh_t.dhdata->dirname,dirname);   //Specify directory name
        dh_t.dhdata->parentdir=parentdir;       //Specify where to create the dir
        dh_t.dhdata->dirext=0;                  //No extension block yet
        dh_t.dhdata->file[0]=0;                 //No files yet
        writeblock(diraddress);                 //Write partition header to disk 
printf("\nCreated dir %s at block 0x%08lx", dirname, diraddress);
        return (diraddress);                    //Return the address of the new partition header
    } else {                                    //no block available
        jfcstatus=E_JFC_NOBLOCKFORDIR;          //Signal could not find an empty block for the dir
        return (0);
    }
}

/**
    Create a partition. The partition header is created on disk, but not yet entered in the partition table
    Create the / dir before calling createPartition(), and pass its dir header block as parameter
*/
long createPartition(char driveletter, char* partname, long bootfile, long rootdir)
{
union ph_transfer ph_t;
long ph_address;

    if ((ph_address=getblock())!=0){        //non-zero means a block has been made available, 0 means no block
        ph_t.buffer=&BlockBuffer[0];            //Link ph_t buffer to physical address of BlockBuffer
        ph_t.phdata->blocktype=T_PARTHDR;       //Block type is T_PARTHDR of 0xA0
        ph_t.phdata->driveletter=driveletter;   //assign the give drive letter
        strcpy(ph_t.phdata->volname, partname); //copy partition name into data structure
        ph_t.phdata->bootfile=bootfile;         //copy block address of boot file (or 0 if none)
        ph_t.phdata->rootdir=rootdir;           //copy block address of root dir (must be created in advance)
        writeblock(ph_address);             //Write partition header to disk
printf("\nCreated partition %s with drive letter %c at block 0x%08lx", partname, driveletter, ph_address);
        return (ph_address);                //Return the address of the new partition header
    } else {                                //no block available
printf("\nFailed to allocate block for partition header.");
        return (0);
    }
}

int addpart(long newpart)
{
union pm_transfer pm_t;                     

    readblock(A_PARTMAP);                   //Read the partition map raw data
    pm_t.buffer=&BlockBuffer[0];            //Map the partmap structure onto the data
    if (pm_t.pmdata->no_parts>=MAXPARTS) {  //Max number of partitions reached
        jfcstatus=E_JFC_PARTMAPFULL         //Message that partition map is full
        return (0);                         //Return error code
    } else {                                //Ready to add it
        pm_t.pmdata->parthdr[pm_t.pmdata->no_parts]=newpart;    //Add the address of the new partition header
        pm_t.pmdata->no_parts++;            //Increase the number of defined partitions          
    }
}