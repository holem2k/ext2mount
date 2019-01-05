#ifndef __HELPERS_H__
#define __HELPERS_H__
#include "common.h"

typedef struct CODE_PAGE_DATA
{
    CODE_PAGE  CodePage;
} CODE_PAGE_DATA, *PCODE_PAGE_DATA;

extern "C"
LONGLONG
Ext2TimeUnixToWin(
                  IN ULONG  UnixTime
                  );

extern "C"
NTSTATUS
Ext2MapBuffer(
              PIRP  Irp,
              PVOID *Buffer
              );

extern "C"
NTSTATUS
Ext2LockBuffer(
               PIRP     Irp,
               ULONG    Length
               );

extern "C"
BOOLEAN
Ext2LoadCodePage(
                PCODE_PAGE_DATA RecodeData,
                CODE_PAGE       CodePage
                );

extern "C"
VOID
Ext2FreeCodePage(
                 PCODE_PAGE_DATA CodePage
                 );

extern "C"
VOID
Ext2RecodeBytes(
                PCODE_PAGE_DATA CodePage,
                PUCHAR          SrcBuffer,
                PUCHAR          DestBuffer,
                ULONG           Length
                );

#endif //__HELPERS_H__