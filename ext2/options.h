#ifndef __OPTIONS_H__
#define __OPTIONS_H__

typedef struct EXT2_OPTIONS 
{
    BOOLEAN     ResolveLinks;
    CODE_PAGE   CodePage;
} EXT2_OPTIONS, *PEXT2_OPTIONS;

extern "C"
BOOLEAN
Ext2LoadOptions(
                PEXT2_OPTIONS   Options,
                PUNICODE_STRING RegPath
                );

extern "C"
VOID
Ext2LoadDefaultOptions(
                       PEXT2_OPTIONS Options
                       );

#endif //__OPTIONS_H__