/**
	SD-CLI.h is the header file for SD-CLI.c
	Command Line Interpreter for the SD-Mon project
*/

#ifndef _H_SDCLI
#define _H_SDCLI

//Constants
#define CLI_MAX_ARGS    10                      //Max nr of arguments on command line (incl arg0)
#define CLI_ARGLEN      30                      //Max length of an argument.
#define CLI_CL_LENGTH   80                      //Max length of command line

//CLI status code constants
#define E_CLI_OK        0                       //CLI status OK
#define E_CLI_SDERR     101                     //SD card error
#define E_CLI_SDCOMM    102                     //SD comm error

#define E_CLI_QUIT      199                     //CLI termination requested

//Structure of a command table: Command Name and Associated Function
struct cTable {
    char command[10];                           //The command text
    int (* assoc_function)(int, char[CLI_MAX_ARGS][CLI_ARGLEN]);       //pointer to associated function
};

//Definition of table with SDMon commands
struct cTable SDMonCmds[] = {
    { "read",   &readDisplayBlocks  },
    { "dir",    &dir                },
    { "exit",   &quit               }
};

//Function prototypes
int SD_CLI(struct cTable cmdTable[], char *prompt);
int getargs(char cmdLine[], char argv[CLI_MAX_ARGS][CLI_ARGLEN]);
bool whiteSpace(char aCharacter);
void * lookupCmd(char token[CLI_ARGLEN]);
char * strnccat(char* s, char newchar);

//Command execution functions
int dir(int argc, char argv[CLI_MAX_ARGS][CLI_ARGLEN]);
int quit(int argc, char argv[CLI_MAX_ARGS][CLI_ARGLEN]);

#endif //ifndef _H_SDCLI