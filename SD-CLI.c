/**
  SD-CLI is the command line interpreter for SD-Mon project
  
  - Display a prompt
  - Wait for a complete line to be input - allow in-line editing
  - Accept the line, break into 'tokens' at whitespace
  - lookup first token in command table
  - pass other tokens to function related to the command
*/

#include "SD-CLI.h"

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
            selectedFunction=lookupCmd(argv[0]);            //First arguvent value is the command, return pointer to function
            resultCode=(*selectedFunction)(argc, argv);     //Execute the selected function and pass arguments
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
int getargs(char cmdLine[], char argv[CLI_MAX_ARGS][CLI_ARGLEN])
{
int nrArgs;
int cliIndex, currentArg;
int cmdLineLength;
bool cmdLineReady;

    currentArg=0;
    argv[0]="";
    cmdLineLength=strlen(cmdLine);
    cmdLineReady=false;
    while (!cmdLineReady) {
        while (!whitespace(cmdLine[cliIndex]) && cliIndex<CLI_CL_LENGTH) {  //as long as no whitespace encountered
            strncat(argv[currentArg], &cmdLine[cliIndex], 1);               //copy until whitespace or end of cmd line
            cliIndex++;                                                     //next character
        } 
printf("\nArg %d = \"%s\"",currentArg,argv[currentArg]);                                                                  //----- whitespace encountered.
        currentArg++;                                                       //Next argument
        while (whitespace(cmdLine[cliIndex]) && cliIndex<CLI_CL_LENGTH){    //as long as we encounter whitespace (or end of cmdline)
            cliIndex++;
        }
    } //While not ready, do another argv...
    return(nrArgs);
}

/**
    Determine if a char is whitespace
    Return true or false
*/
bool whitespace(char aCharacter)
{
    switch(aCharacter) {
    case ' ':
        return(true);
        break;
    case ',':
        return(true);
        break;
    case ';':
        return(true);
        break;
    Default:
        return(false);
    }
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
