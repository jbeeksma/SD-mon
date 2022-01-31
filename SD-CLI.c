/**
  SD-CLI is the command line interpreter for SD-Mon project
  
  - Display a prompt
  - Wait for a complete line to be input - allow in-line editing
  - Accept the line, break into 'tokens' at whitespace
  - lookup first token in command table
  - pass other tokens to function related to the command
*/

#include "SD-CLI.h"
#include <stdbool.h>
#include "jfs.h"

/**
    Initialize the CLI environment
*/

int cliInit()
{
    if ((activePartition=getPart(0))!=0) {                         //Put partition[0] block address in activePartition
        return (E_CLI_OK);
    } else {
        return (jfcstatus);
    }
}

int SD_CLI(struct cTable cmdTable[], char *prompt)
{
char cmdLine[80];
int argc, currentarg;
char argv[CLI_MAX_ARGS][CLI_ARGLEN];
int resultCode;
CLI_FUNC selectedFunction;

    resultCode=E_CLI_OK;
    while (resultCode != E_CLI_QUIT) {
        printf("\n%s ", prompt);                                                //Print the prompt (and a space)
        strcpy(cmdLine, "");                                                    //Start with an empty cmdline
        if (getline(cmdLine, CLI_CL_LENGTH)!=0) {                               //Get a line from the terminal, 0 if nothing entered
            argc=getargs(cmdLine,&argv);                                        //Split the command line into args (unix-style)
            if (selectedFunction=lookupCmd(cmdTable, argv[0])){                 //if non-0, got a pointer to function
            resultCode=(*selectedFunction)(argc, argv);                         //Execute the selected function and pass arguments
            } else {                                                            //If 0, we did not find a function
                printf("\a\n Unknown command");
            }
        }
    }
    return(resultCode);
}

/**
    Split up command line into chunks separated by whitespace
    1st argument (argv[0]) is the command
    Arguments are max CLI_ARGLEN long (30 chars incl terminating 0)
    Max 10 args (including command) will be recognised
*/
int getargs(char * cmdLine, char argv[CLI_MAX_ARGS][CLI_ARGLEN])
{
int cliIndex, currentArg;
int cmdLineLength;
bool cmdLineReady;

    for (currentArg=0; currentArg<CLI_MAX_ARGS; currentArg++){
        strcpy(argv[currentArg], "");                                           //Delete old args
    }
    currentArg=0;                                                               //start at argv[0]
    cmdLineLength=strlen(cmdLine);                                              //Determine actual length of cmdline
    cmdLineReady=0;                                                             //Reset termination condition for loop
    cliIndex=0;                                                                 //start at beginning of cmdline
    while (cmdLineReady==false) {                                               //Loop untilwhole cmdline processed
        while ((!whiteSpace(cmdLine[cliIndex])) && (cliIndex<cmdLineLength)) {  //as long as no whitespace encountered
            strnccat(argv[currentArg], cmdLine[cliIndex++]);                    //copy until whitespace or end of cmdline
        }   
        currentArg++;                                                           //Next argument
        while (whiteSpace(cmdLine[cliIndex]) && cliIndex<CLI_CL_LENGTH){        //as long as we encounter whitespace (or end of cmdline)
            cliIndex++;                                                         //Walk throug to next non-whitespace character
        }
        if (cliIndex==cmdLineLength) cmdLineReady=true;                         //Ready when whole cmdline is processed
    } //While not ready, do another argv...
    return(currentArg);                                                         //return the number of detected arguments
}

/**
    Determine if a char is whitespace
    Return true or false
*/
bool whiteSpace(char aCharacter)
{
    switch(aCharacter) {
    case ' ':
        return(true);
    case ',':
        return(true);
    case ';':
        return(true);
    default:
        return(false);
    }
}

/**
    Add a char to the end of string s
    Needed because CMOC does not support strncat
*/
char * strnccat(char* s, char newchar)
{
int curlen;                 //Length of string before concat

    curlen=strlen(s);       //Determine current length
    s[curlen]=newchar;      //Add new character
    s[curlen+1]=0;          //Terminate string
    return(s);
}

/**
    Looks up a command in a command table
    Returns a pointer to the associated function to carry out the command
*/
CLI_FUNC lookupCmd(struct cTable * cmdTable, char * token)
{
int cmdIndex;
    
    for (cmdIndex=0; cmdTable[cmdIndex].assoc_function != (CLI_FUNC)0; cmdIndex++) {
        if (strcmp(token, cmdTable[cmdIndex].command)==0)                               //If the command is in the list
        {
            return (cmdTable[cmdIndex].assoc_function);                                 //Call the associated function
        }
    } 
    return(0);
}

/**
    Print error text from lookup table
    Lookup error code in global table errorTable[]
    Print related error message text
*/
void eMessage(int errorCode)
{
int tableIndex;

    for (tableIndex=0; errorTable[tableIndex].eCode != EOT; tableIndex++) {
        if (errorTable[tableIndex].eCode==errorCode){
            printf("\n%s", errorTable[tableIndex].eText);
        }
    }
}  

/**
    Print argument list for debugging purposes
*/
void printargs(int argc, char argv[CLI_MAX_ARGS][CLI_ARGLEN])
{
int count;

    printf("\n argc: %d", argc);
    for (count=0; count <argc; count++){
        printf("\nargv[%d] = %s", count, argv[count]);
    }
}   

/**
    fullPath accepts a block address of a directory header, and backtracks up to the root dir
    assembles and returns the full path name
    Max length of the full path is MAX_PATH_LEN
*/
char * fullPath(long dirPtr, char path[MAX_PATH_LEN])
{
union dh_transfer dh_t;

    readblock(dirPtr);                              //Read the data for the directory header
    dh_t.buffer=&BlockBuffer[0];                    //Map directory header data structure onto raw data block
    if (dh_t.dhdata->parentdir==0){                 //There is no higher dir
        path=strcat(dh_t.dhdata->dirname,path);     //Add the current dir name in front of the path.
        return(path);                               //Return the assembled string
    } else {                                        //There is a parent dir
        path=strcat(PATHSEPARATOR,path);            //Add separator (typically a /) 
        path=fullPath(dh_t.dhdata->parentdir,path); //Recurse into next higher level           
    }
}             

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////// CLI Commands implementations ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
    Implementation of the "help" command
*/
int help(int argc, char argv[CLI_MAX_ARGS][CLI_ARGLEN])
{
int cmdIndex;

    //TODO: Needs elaboration: Help for specific commands
    //Rudimentary version shows exiting commands
    for (cmdIndex=0; SDMonCmds[cmdIndex].assoc_function != (CLI_FUNC)0; cmdIndex++){
        printf("\n %s", SDMonCmds[cmdIndex].command);
    }
    printf("\n");
}

/**
    Implementation of the 'dir' command
*/
int dir(int argc, char argv[CLI_MAX_ARGS][CLI_MAX_ARGS])
{
union ph_transfer ph_t;
union dh_transfer dh_t;
union fh_transfer fh_t;
long partCurDir;
int dirIndex;

    readblock(activePartition);                                     //Get the raw data from the partHdr
    ph_t.buffer=&BlockBuffer[0];                                    //Map the ph_t structure onto the data   
    printf("\nVolume in drive %c is %s ", ph_t.phdata->driveletter, ph_t.phdata->volname);
    readblock(ph_t.phdata->curdir);                                 //Read the directory header for the *current* dir on this partition
    dh_t.buffer=&BlockBuffer[0];                                    //Map the dh_t structure onto the data
    printf("\nDirectory of %s :\n", dh_t.dhdata->dirname);    
    for (dirIndex=0; dh_t.dhdata->file[dirIndex]!=0;dirIndex++) {   //Walk through the list of file header addresses
        readblock(dh_t.dhdata->file[dirIndex]);                     //Get the data for the file header
        fh_t.buffer=&BlockBuffer[0];                                //Map fileheader data structure onto raw data
        printf("\n%s \t", fh_t.fhdata->filename);                   //Print file name
    }
    return(E_CLI_OK);                                              
}

/// Get the address of partition header 'partno'
///
long getPartHdr(int partno)
{
union pm_transfer pm_t;                    

    readblock(A_PARTMAP);                                       //Read the partition map raw data
    pm_t.buffer=&BlockBuffer[0];                                //Map the partmap structure onto the data
printf("\n%s PM code = %02x", __func__, pm_t.pmdata->blocktype);
printf("\n%s # partitions = %d", __func__, pm_t.pmdata->no_parts);
printf("\n%s Address of part 0 = 0x%08lx", __func__, pm_t.pmdata->parthdr[0]);
printf("\n%s Requested part no: %d", __func__, partno);
    return (pm_t.pmdata->parthdr[partno]);                      //Return the block address of the partition header
}


/**
    Implementation of the 'quit' command
*/
int quitCLI(int argc, char argv[CLI_MAX_ARGS][CLI_MAX_ARGS])
{
    return(E_CLI_QUIT);                                     //Signal main loop that we want to quit                                          
}

/**
    @brief Implementation of the 'part' command
    
    manipulate and inspect disk partitions
*/    
int part(int argc, char argv[CLI_MAX_ARGS][CLI_ARGLEN])
{
union ph_transfer ph_t;

    readblock(activePartition);                                     //Get the raw data from the partHdr
    ph_t.buffer=&BlockBuffer[0];                                    //Map the partition header dat structure onto the data   
    printf("\nActive partition @0x%08lx, drive %c, volume %s", activePartition, ph_t.phdata->driveletter, ph_t.phdata->volname);
printf("\n%s cur dir @0x%08lx", __func__, ph_t.phdata->curdir);
    printf("\nCurrent dir is %s", fullPath(ph_t.phdata->curdir, ""));
    readblock(ph_t.phdata->curdir);
    return(0);
}

/// Subcommand of part to list defined partitions
///
int listParts(int argc, char argv[CLI_MAX_ARGS][CLI_ARGLEN])
{
    //TODO: Implement part list command
    return(0);
}
