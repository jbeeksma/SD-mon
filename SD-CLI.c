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

int SD_CLI(struct cTable cmdTable[], char *prompt)
{
char cmdLine[80];
int argc;
char argv[CLI_MAX_ARGS][CLI_MAX_ARGS];
int resultCode;
int (*selectedFunction)(int, char[CLI_MAX_ARGS][CLI_ARGLEN]);

    resultCode=E_CLI_OK;
    while (resultCode != E_CLI_QUIT) {
        printf("\n%s ", prompt);                            //Print the prompt (and a space)
        if (getline(cmdLine, CLI_CL_LENGTH)!=0) {           //Get a line from the terminal, 0 if nothing entered
            argc=getargs(cmdLine,argv);                     //Split the command line into args (unix-style)
printf("\nFound %d arguments.",argc);
//            selectedFunction=lookupCmd(argv[0]);            //First arguvent value is the command, return pointer to function
//            resultCode=(*selectedFunction)(argc, argv);     //Execute the selected function and pass arguments
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

    currentArg=0;
    strcpy(argv[0],"");
    cmdLineLength=strlen(cmdLine);
    cmdLineReady=0;
    cliIndex=0;
    while (cmdLineReady==false) {
printf("\nNew argument: ");
        while ((!whiteSpace(cmdLine[cliIndex])) && (cliIndex<cmdLineLength)) {  //as long as no whitespace encountered
printf("%c", cmdLine[cliIndex]) ;       
            strnccat(argv[currentArg], cmdLine[cliIndex++]);                //copy until whitespace or end of cmd line
        }                                                    //----- whitespace encountered.
        currentArg++;                                                       //Next argument
        while (whiteSpace(cmdLine[cliIndex]) && cliIndex<CLI_CL_LENGTH){    //as long as we encounter whitespace (or end of cmdline)
            cliIndex++;
        }
        if (cliIndex==cmdLineLength) cmdLineReady=true;
    } //While not ready, do another argv...
    return(currentArg);
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
void * lookupCmd(char token[CLI_ARGLEN])
{
    //TODO: Implement lookup function
    return(0);
}


/**
    Implementation of the 'dir' command
*/
int dir(int argc, char argv[CLI_MAX_ARGS][CLI_MAX_ARGS])
{
    //TODO: Implement dir command
    return(E_CLI_OK);                                              
}

/**
    Implementation of the 'quit' command
*/
int quit(int argc, char argv[CLI_MAX_ARGS][CLI_MAX_ARGS])
{
    //TODO: Implement dir command
    return(E_CLI_QUIT);                                     //Signal main loop that we want to quit                                          
}
