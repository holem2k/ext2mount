#include "stdafx.h"
#include "helpers.h"
#include "codetables.h"

/////////////////////////////////////////////////////////////////////////////
// Ext2TimeUnixToWin()
// Expected IRQL: < DISPATCH
// Section: NONpaged.
// Cause page errors: NO

extern "C"
LONGLONG
Ext2TimeUnixToWin(
                  IN ULONG  UnixTime
                  )
{
    LONGLONG WinTime;
    RtlSecondsSince1970ToTime(UnixTime, (PLARGE_INTEGER)&WinTime);
    return WinTime;
    // return (LONGLONG)UnixTime * 10000000 + 116444736000000000;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2MapBuffer(...)
// IRQL : PASSIVE, APC

extern "C"
NTSTATUS
Ext2MapBuffer(
              PIRP  Irp,
              PVOID *Buffer
              )
{
    PAGED_CODE();

    PVOID Buf = Irp->MdlAddress ? 
        MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority) : Irp->UserBuffer;

    if (Buf == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;

    *Buffer = Buf;
    return STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2LockBuffer(...)
// IRQL : PASSIVE, APC
// Context: user process

extern "C"
NTSTATUS
Ext2LockBuffer(
               PIRP     Irp,
               ULONG    Length
               )
{
    PAGED_CODE();

    if (Irp->MdlAddress != NULL)
        return STATUS_SUCCESS; // already locked

    NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
    PMDL Mdl = IoAllocateMdl(Irp->UserBuffer, Length,
        FALSE, FALSE, NULL);
    if (Mdl)
    {
        __try
        {
            MmProbeAndLockPages(Mdl, Irp->RequestorMode, IoWriteAccess);
            Irp->MdlAddress = Mdl;
            status = STATUS_SUCCESS;
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            IoFreeMdl(Mdl);
            status = GetExceptionCode();
            if (!FsRtlIsNtstatusExpected(status))
                status = STATUS_INVALID_USER_BUFFER;
        }
    }

    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOLEAN
Ext2LoadCodePage(
                PCODE_PAGE_DATA CodePageData,
                CODE_PAGE       CodePage
                )
{
    CodePageData->CodePage = CodePage >= CP_INVALID ? CP_KOI8R : CodePage;
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2FreeCodePage(
                 PCODE_PAGE_DATA CodePage
                 )
{
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2RecodeBytes(
                PCODE_PAGE_DATA CodePage,
                PUCHAR          SrcBuffer,
                PUCHAR          DestBuffer,
                ULONG           Length
                )
{
    for (ULONG i = 0; i < Length; i++)
    {
        DestBuffer[i] = SrcBuffer[i] < 0x80 ?
            SrcBuffer[i] : CodeTables[CodePage->CodePage][SrcBuffer[i] - 0x80];
    }
}

/////////////////////////////////////////////////////////////////////////////