#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "paramConfig.h"
#include "errorHandling.h"

void InitConfigParam ( PARAM_CONFIG_T* config )
{
    strcpy(config->hostname, "sysprak.priv.lab.nm.ifi.lmu.de");
    config->port = 1357;
    strcpy(config->gamename, "NMMorris");
}

static char* trim(char* line)
{
    //printf("Trim [%s]->",line);
    char* res = line;

    while(*res == ' ')
        res++;
    
    char* p = res;
    while(*p)
    {
        if(*p == ' ' || *p == '\r' || *p == '\n' )
        {
            *p = '\0';
            break;
        }
        else
            p++;
    }
    //printf("[%s]\n",res);
    return res;
}

static int processLine(PARAM_CONFIG_T* config, char* line)
{
    int res = -1;

    if (*line != '#')
    {
        char *cutEq = strchr(line, '=');
        if(cutEq)
        {
            *cutEq = '\0';
            //printf("line: [%s]\n",line);
            if(strstr(line, "Hostname"))
            {
                strcpy(config->hostname, trim(cutEq+1));
            }
            else if(strstr(line, "Port"))
            {
                config->port = atoi(trim(cutEq+1));
            }
            else if(strstr(line, "Gamename"))
            {
                strcpy(config->gamename, trim(cutEq+1));
            }
        }
    }
    return res;
}

int LoadConfigParam (PARAM_CONFIG_T* config,  char *fileName)
{
    // Q: file unreadable?
    int res = -1;

    FILE* fp = fopen (fileName, "r");
    if(fp == NULL)
    {
        errWithFile ("fopen", fileName);
    }
    else
    {
        char * line = NULL;
        size_t len = 0;
        ssize_t read;

        while ((read = getline(&line, &len, fp)) != -1) 
        {
            //printf("Line read: [%s]\n", line);
            if(processLine(config, line) == 0)
                res = 0;
        }

        fclose(fp);
        if (line)
            free(line);
    }

    return res;
}

void DumpConfig ( PARAM_CONFIG_T* config )
{
    printf("Actual Configuration\n");
    printf("Hostname: [%s]\n", config->hostname);
    printf("Port    :  %d\n", config->port);
    printf("Gamename: [%s]\n", config->gamename);
}