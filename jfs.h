/*
	JFS.h 
	
	Header file for Jacobs File System for 6309 SBC SD card
	V0.0 (c) 2021 Jacob Beeksma.
*/

#ifndef _H_JFS
#define _H_JFS
#include <stdbool.h>

/* Block type definitions */

#define T_EMPTYHDR	0x00	//Empty block chain header
			//	1 byte:		0x00 = Empty block header
			//	4 bytes:	Address of first empty block in chain (0 if none)
			//	4 bytes:	Address of last empty block in chain.
#define T_EMPTYBLK	0x01	//Empty block
			//	1 byte:		0x01 = Empty block
			//	4 bytes:	Address of next empty block in chain (0 if none)
#define T_PARTMAP	0x10	//Partition map block
			//	1 byte:		0x01 = Partition map
			//	1 byte:		#of partitions
			//	[Groups of 4 bytes]: Address of partition header (0 after last partition)
#define T_PARTHDR	0xA0	//Partition header
			//	1 byte:		0xA0 = Partition header
			//	1 byte:		Assigned drive letter
			//	32 bytes:	Volume name
			//	4 bytes:	Address of boot file (0 if not bootable)
			//  4 bytes:    Address of root dir header		
#define T_BADBLKHDR	0xB0	//Bad blocks header
			//	1 byte: 	0xB0 = Bad block list header
			//	4 bytes:	#bad blocks in list
			//	4 bytes:	Address of extension block (0 if none)
			// [125 groups of 4 bytes]:	Addresses of bad blocks (0 after last bad block)
#define T_BADBLKEXT	0xBE	//Bad blocks extension block
			//	1 byte:		0xBE = Bad block list extension
			//	5 bytes:	Address of previous extension block
			//	5 bytes:	Address of next extension block (0 if none)
			// [125 groups of 4 bytes]:	Addresses of bad blocks (0 after last bad block)
#define T_DIRHDR	0xD0	//Directory header block
			//	1 byte:		0xD0
			//	1 byte:		Directory attributes
			//			    writeable/hidden/virtual (link)
			//	32 bytes:	Dir name
			//	4 bytes:	Address of parent dir ..
			//  4 bytes:    Address of extension block (0 if none)
			//	3 bytes:	Last write date (6 char BCD)
			//	3 bytes:	Last write time (6 char BCD)
			// [116 groups of 4 bytes]:	Addresses of file headers (0 after last file)
#define T_DIRLINK	0xD1	//Virtual directory (link)
            //  1 byte:     0xD1
			//	1 byte:		Directory attributes
			//			    writeable/hidden/virtual (link)
            //  32 bytes:   Directory name
			//  4 bytes:    Address (block #) of parent dir
            //  4 bytes:    Link to "Real" directory header
#define T_DIREXT	0xDE	//Directory extension block
			//	1 byte:		0xDE
			//  4 bytes:    Address (block #) of previous dir block
			//  4 bytes:    Addres of next extension block (0 if no more)
			// [126 groups of 4 bytes]:	Addresses of file headers (0 after last file)
#define T_FILEHDR	0xF0	//File header block
			//	1 byte: 	0xF0
			//	1 byte:		File attributes
			//			    writeable/hidden/virtual (link)/executable/system
			//	32 bytes:	File name
			//	4 bytes:	Address of next block in file chain (0 if none)
			//	3 bytes:	Last write date (6 char BCD)
			//	3 bytes:	Last write time (6 char BCD)
			//	4 bytes:	File size (data only) Max size = 4Mb
			// (max 465) bytes:		File data
#define T_FILEEXT	0xFE	//File extension block
			//	1 byte: 	0xFE
			//	4 bytes:	Address of previous block in file chain
			//	4 bytes:	Address of next block in file chain (0 if none)
			// (max 503) bytes:		File data

// Predefined block numbers
#define A_BOOTBLOCK	0	//Boot block address
#define A_EMPTYCHN	1	//Empty block chain address
#define A_PARTMAP	2	//Partition map address
#define A_BADBLKHDR	3	//Bad block header address

// File system related constants
#define MAXPARTS    10  /**Max # of partitions on a volume*/
#define MAXBBLOCKS  125 /**Nr of bad blocks that fit into a BBHeader of BBExt block*/
#define DHMAXFILES  116 /**Max # of file entries in directory header*/
#define DEMAXFILES  126 /**Max # of file entries in directory extension*/
#define FHMAXBYTES  465 /**Max # of bytes in file header*/
#define FEMAXBYTES  503 /**Max # of bytes in file extension*/

// Constants for partitions and directories
#define NOATTRIB    0   //Specifies no dir attributes
#define NOPARENT    0   //No parent dir
#define NOTBOOTABLE 0   //Partition is not bootable

// Data structures

/**
    Data structure for partition map
*/

struct s_partmap {
    unsigned char blocktype;
    unsigned char no_parts;
    long parthdr[MAXPARTS];
};

/**
    Data structure for empty chain header
*/
struct s_emptyhdr {                             
    unsigned char   blocktype;                  //T_EMPTYHDR or 0x00
    long            first_eb;                   //Address of first known empty block or 0 if none
    long            last_eb;                    //Address of last known empty block or 0 if none
};	

/**
    Data structure for empty block
*/
struct s_eblock {
    unsigned char   blocktype;                  //T_EMPTYBLK or 0x01
    long            next_eb;                    //Address of next empty block in chain or 0 if none
    long            prev_eb;                    //Address of previous empty block in chain or 0 if none
};

/**
    Data structure for bad block header
*/
struct s_bblockh {                              /** Bad block list header block */
    unsigned char   blocktype;                  //T_BADBLKHDR or 0xB0
    long            nrbadblocks;                //Number of known bad blocks listed in header + ext blocks
    long            extb_block;                 //Address of extension block for Bad Block List or 0 if none
    long            badblock[MAXBBLOCKS];       //Array of adresses of bad blocks
};

/**
    Data structure for bad block extension
*/
struct s_bblockx {                              /** Bad block list extension block */
    unsigned char   blocktype;                  //T_BADBLKEXT or 0xB1
    long            prevbblock;                 //Previous Bad Block list block
    long            extb_block;                 //Address of extension block for Bad Block List or 0 if none
    long            badblock[MAXBBLOCKS];       //array of adresses of bad blocks
};

/**
    Data structure for partition header
*/
struct s_parth {                                /** Partition header block structure*/
    unsigned char   blocktype;                  //T_PARTHDR or 0xA0
    char            driveletter;                //Drive letter if assigned, else 0
    char            volname[32];                //Volume name string max 32 chars
    long            bootfile;                   //Adress of fileheader block for boot file, or 0 if none
    long            rootdir;                    //Address of root directory header block
};

/**
    Data structure for directory header
*/
struct s_dirh {                                 /** Directory header block structure */
    unsigned char   blocktype;                  //T_DIRHDR or 0xD0
    unsigned char   attibutes;                  //Directory attributes
    char            dirname[32];                //Directory name
    long            parentdir;                  //Address of parent dir or 0 if none
    long            dirext;                     //Address of extension block or 0 if none  
    char            moddate[6];                 //Date of last change
    char            modtime[6];                 //Time of last change
    long            file[DHMAXFILES];           //The first 116 files in dir (0 after last used)
};

/**
    Data structure for directory extension
*/
struct s_dirx {                                 /** Directory extension block structure */
    unsigned char   blocktype;                  //T_DIREXT or 0xDE
    long            prevdblock;                 //Address of previous dir block
    long            nextdblock;                 //Address of next extension block or 0 if none  
    long            file[DXMAXFILES];           //Additional 126 files in dir (0 after last used)
};

/** union used to map empty chain header structure onto raw disk block */
union ech_transfer {
    struct s_emptyhdr* ecdata;
    unsigned char* buffer;
};

/*Union used to map empty block data structure onto raw disk block*/
union eb_transfer {
    struct s_eblock* ebdata;
    unsigned char* buffer;
};
		
/**Union used to map partition map block data structure onto raw disk block*/
union pm_transfer {
    struct s_partmap* pmdata;
    unsigned char* buffer;
};

/** Union used to map bad block header data structure onto raw disk block*/
union bbh_transfer {
    struct s_bblockh *  bbhdata;
    unsigned char * buffer;
};
					
/** Union used to map bad block extension block data structure onto raw disk block*/
union bbx_transfer {
    struct s_bblockx *  bbxdata;
    unsigned char * buffer;
};

/**Union used to map partition header structure onto raw disk block*/
union ph_transfer {
    struct s_parth * phdata;
    unsigned char * buffer;
};

/**union used to map directory header structure onto raw disk block*/
union dh_transfer {
    struct s_dirh * dhdata;
    unsigned char * buffer;
};
					
/**Union used to map directory extension structure onto raw disk block*/
union dx_transfer {
    struct s_dirx * dhdata;
    unsigned char * buffer;
};
					
// Function prototypes

long JDOS_erase(long maxblocks);		                        //erase whole disk, create empty chain
bool erase_test_block(long BlockNr);                            //erase block, then test
void printerr(const char * errormmessage);                      //print error message with bell and newlines
int fillblock(long BlockNr, unsigned char Value);               //fill block with value
int writeblock(long blocknr);                                   //write the contents of the buffer into block BlockNr
int testblock(long BlockNr, unsigned char Value);               //test if block is filled with value
int readblock(long blocknr);                                    //read block (blocknr) into global blockbuffer
void init_ec_header(long firstEBlock);                          //initialise empty chain header block
void init_partmap();                                            //initialise the partition map block
void init_badblk_hdr();                                         //initialise the bad block header block
void add_bad_block(long blocknr);                               //add block that failed to initialise to bad block list
void add_to_ec(long blocknr);                                   //append block to empty chain
long GetLastECBlockNr();                                        //Get block number of last block in Empty Chain
void UpdateLastECBlock(long prevlastblock,long newblock);       //Add pointer to new block in last block of EC
void UpdateCurrentBlock(long newblobck,long prevlastblock);     //Add Blocktype and address of previous empty block
void UpdateECHeader(long newblock);                             //Write new last block into EC Header
long createDir(char* dirname, unsigned char attribs, long parentdir);                   //create a directory with name, attribs, parent dir
long createPartition(char driveletter, char* partname, long bootfile, long rootdir);    //Create a partition with drive letter, bootable flag, root dir
int addpart(long newpart);                                      //Add newly created partition to partmap return # of partitions, or 0 if error
long getblock();                                                //Get an empty block from empty chain, or 0 if none available
void eb_unlink(long blocknr);                                   //Remove (blocknr) from empty chain
void ec_modfirst(long blocknr);                                 //Register blocknr as first eb in empty chain

//Global variables for jfc
unsigned char jfcstatus;                                        //Global variable to pass error codes

//jfc status and error codes
#define E_JFC_OK            0;                                  //0 = OK
#define E_JFC_PARTMAPFULL   100;                                //Partition map full, no more new partitions
#define E_JFC_NOBLOCKFORDIR 101;                                //Dir creation failed - no free disk block
#endif //_H_JFSH