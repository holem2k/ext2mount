#include "stdafx.h"
#include <stdarg.h>
#include <stdio.h>

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

VOID
Ext2LogToFile(
             IN PCHAR Format,
             ...
             )
{
    if (KeGetCurrentIrql() > PASSIVE_LEVEL)
        return;

    // make string
    PCHAR Buf = (PCHAR)ExAllocatePool(PagedPool, 4096*3);
    if (!Buf)
        return;
    RtlZeroMemory(Buf, 4096*3); // û-û-û

    va_list va;
    va_start(va, Format);
    vsprintf(Buf, Format, va);
    va_end(va);

    // write string to file
    OBJECT_ATTRIBUTES ObjAttr;
    UNICODE_STRING LogName;
    RtlInitUnicodeString(&LogName, L"\\??\\C:\\ext2.log");
    InitializeObjectAttributes(&ObjAttr,
        (PUNICODE_STRING)&LogName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    HANDLE FileHandle;
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS status = ZwCreateFile(&FileHandle,
        FILE_WRITE_DATA,
        &ObjAttr,
        &IoStatus,
        0, 
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_WRITE,
        FILE_OPEN_IF,
        FILE_SYNCHRONOUS_IO_NONALERT | FILE_NO_INTERMEDIATE_BUFFERING,
        NULL,     
        0);

    if (NT_SUCCESS(status))
    {
        
        FILE_STANDARD_INFORMATION FileStdInfo;
        status = ZwQueryInformationFile(FileHandle,
            &IoStatus,
            &FileStdInfo,
            sizeof(FileStdInfo),
            FileStandardInformation);
        
        FILE_POSITION_INFORMATION FilePosInfo;
        ULONGLONG Offset = NT_SUCCESS(status) ? FileStdInfo.EndOfFile.QuadPart : 0;
        Offset = ALIGN(Offset, 512, ULONGLONG);
        FilePosInfo.CurrentByteOffset.QuadPart = Offset;
        status = ZwSetInformationFile(FileHandle,
            &IoStatus,
            &FilePosInfo,
            sizeof(FilePosInfo),
            FilePositionInformation);
        
        if (NT_SUCCESS(status))
        {
            ULONG Length = strlen(Buf);
            Length = ALIGN(Length, 512, ULONG);
            ZwWriteFile(FileHandle,
                NULL,
                NULL,
                NULL,
                &IoStatus,
                Buf,
                Length,
                NULL,
                NULL);
        }
        
        ZwClose(FileHandle);
    }
    
    ExFreePool(Buf);
}

#endif // #ifdef _DEBUG

/////////////////////////////////////////////////////////////////////////////

