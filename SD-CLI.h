/**
	SD-CLI.h is the header file for SD-CLI.c
	Command Line Interpreter for the SD-Mon project
*/

#ifndef _H_SDCLI
#define _H_SDCLI
//#include "RTC7301.h"                            //Definitions for real time clock

//Constants
#define CLI_MAX_ARGS    10                      //Max nr of arguments on command line (incl arg0)
#define CLI_ARGLEN      30                      //Max length of an argument.
#define CLI_CL_LENGTH   80                      //Max length of command line
#define ERR_TEXT_LEN    30                      //Max length of an error text
#define MAX_PATH_LEN    128                     //Max length of a full path

#define PATHSEPARATOR   "/"                     //Separator between elements of a path (dirs)

//CLI status code constants [100..199] range - 0 is OK no error
#define E_CLI_OK        0                       //CLI status OK
#define E_CLI_SDERR     101                     //SD card error
#define E_CLI_SDCOMM    102                     //SD comm error

#define E_CLI_QUIT      199                     //CLI termination requested
#define EOT             255                     //Signifies end of error table

//typedef for CLI executable functions
typedef int (* CLI_FUNC)(int, char[CLI_MAX_ARGS][CLI_ARGLEN]);

//Structure of a command table: Command Name and Associated Function
struct cTable {
    char command[10];                           //The command text
    CLI_FUNC assoc_function;                    //pointer to associated function
};

//Definition of table with SDMon commands
static struct cTable SDMonCmds[] = {
    { "date",   date               },
    { "dir",    dir                },
    { "exit",   quitCLI            },
    { "help",   help,              },
    { "part",   part,              },
    { "read",   readDisplayBlocks  },
    { "-end-",  (void *) 0         }
};

//Definition of table with part(ition) subcommands
static struct cTable partCmds[] = {
    { "list",   listParts         }
};

//Structure of an error code - error text lookup table

struct eTable {
    int eCode;                                  //Error code see above and in jfs.h
    char eText[ERR_TEXT_LEN];                   //Associated error text
};

//Definition of error codes and messages
static struct eTable errorTable[] = {
    { E_JFC_OK,             "OK"                            },
    { E_JFC_PARTMAPFULL,    "Partition map full"            },
    { E_JFC_NOBLOCKFORDIR,  "No block available for dir"    },
    { E_PART_NONEX,         "Non-existing partition number" },
    { E_PART_UNREAD,        "Partition map unreadable"      },
    { E_CLI_SDERR,          "SD card error"                 },
    { E_CLI_SDCOMM,         "SD Communication error"        },
    { E_CLI_QUIT,           "CLI termination requested"     },
    { EOT,                  "End-of-table"                  }
};

//Global variables
long activePartition;                           //Block address of the currently active partition.
long currentDirectory;                          //Current directory

//Function prototypes
int cliInit();
int SD_CLI(struct cTable cmdTable[], char *prompt);
int getargs(char cmdLine[], char argv[CLI_MAX_ARGS][CLI_ARGLEN]);
bool whiteSpace(char aCharacter);
CLI_FUNC lookupCmd(struct cTable * cmdTable, char * token);
char * strnccat(char* s, char newchar);
void eMessage(int errorCode);                                       //Print error text from lookup table
void printargs(int argc, char argv[CLI_MAX_ARGS][CLI_ARGLEN]);      //Print argument list
char * fullPath(long dirPtr, char path[MAX_PATH_LEN]);              //Backtrack path from directory header address

//Command execution functions
int help(int argc, char argv[CLI_MAX_ARGS][CLI_ARGLEN]);            //Help command, lists default cTable
int dir(int argc, char argv[CLI_MAX_ARGS][CLI_ARGLEN]);             //Dir command
int quitCLI(int argc, char argv[CLI_MAX_ARGS][CLI_ARGLEN]);         //Exit t he CLI
int part(int argc, char argv[CLI_MAX_ARGS][CLI_ARGLEN]);            //Partition command
    int listParts(int argc, char argv[CLI_MAX_ARGS][CLI_ARGLEN]);   //Subcommand of part
    long getPartHdr(int partno);                                    //Get partition [x] header block address

#endif //ifndef _H_SDCLI