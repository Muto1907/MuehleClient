#ifndef errorHandling
#define errorHandling

void errPrintInvalidParam (char *paramName);
void errFunctionFailed (char *funcName);
void errWithHost (char *failedfunc, char* hostname);
void errWithFile (char *failedfunc, char* filename);

void traceMsgRcvd(char* buf);

#endif
