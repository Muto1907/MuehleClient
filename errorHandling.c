#include <stdio.h>
#include "errorHandling.h"

void errPrintInvalidParam (char *paramName)
{
    printf("Invalid %s. \n", paramName);
}

void errFunctionFailed (char *funcName)
{
    printf("Function %s failed. \n", funcName);
}