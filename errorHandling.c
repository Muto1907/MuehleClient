#include <stdio.h>
#include "errorHandling.h"

static char errMsg[1024];

void errPrintInvalidParam (char *paramName)
{
    printf("Invalid %s. \n", paramName);
}

void errFunctionFailed (char *funcName)
{
    printf("Function %s failed. \n", funcName);
}

void errWithHost (char *failedfunc, char* hostname)
{
    sprintf(errMsg, "%s failed, Host [%s]", failedfunc, hostname);
    perror(errMsg);
     
}

void errWithFile (char *failedfunc, char* filename)
{
    sprintf(errMsg, "%s failed, File [%s]", failedfunc, filename);
    perror(errMsg);
     
}


void traceMsgRcvd(char* buf)
{
    printf("Message received: [%s]\n", buf);
}
