#ifndef paramConfig_h
#define paramConfig_h

#define HOSTNAME_MAX (256)
#define GAMENAME_MAX (20)

typedef struct
{
    char hostname [HOSTNAME_MAX];
    unsigned int port;
    char gamename [GAMENAME_MAX];
} PARAM_CONFIG_T;

void InitConfigParam ( PARAM_CONFIG_T* config );
int LoadConfigParam ( PARAM_CONFIG_T* config, char *fileName );
void DumpConfig ( PARAM_CONFIG_T* config );

#endif
