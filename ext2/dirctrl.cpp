#include "stdafx.h"
#include "dirctrl.h"
#include "helpers.h"
#include "dispatch.h"
#include "versup.h"
#include "ext2api.h"

/////////////////////////////////////////////////////////////////////////////
// Ext2DirectoryControl(...)
// Expected IRQL: PASSIVE
// Section: paged.

extern "C"
NTSTATUS
Ext2DirectoryControl(
                     PIRP_CONTEXT   IrpContext
                     )
{
    PAGED_CODE();

    ASSERT(IrpContext);

    NTSTATUS status;
    switch (IrpContext->MinorFunction)
    {
    case IRP_MN_QUERY_DIRECTORY:
        status = Ext2QueryDirectory(IrpContext);
        break;

    case IRP_MN_NOTIFY_CHANGE_DIRECTORY:
        status = Ext2NotifyChangeDirectory(IrpContext);
        break;

    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2QueryDirectory(
                     PIRP_CONTEXT   IrpContext
                     )
{
    ASSERT(IrpContext);

    PAGED_CODE();

    PVCB Vcb; PFCB Fcb;
    Ext2GetFCBandVCB(IrpContext, &Vcb, &Fcb);

    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(IrpContext->Irp);
    ULONG Length = IrpStack->Parameters.QueryDirectory.Length;
    ULONG Information = 0;

    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN FcbResAcquired = FALSE;

    __try
    {
        if (!ExAcquireResourceExclusiveLite(&Fcb->MainResource, IrpContext->IsSync))
        {
            status = STATUS_PENDING;
            __leave;
        }
        FcbResAcquired = TRUE;

        status = Ext2VerifyFcbOperation(IrpContext);
        if (!NT_SUCCESS(status))
            __leave;

        // Get buffer address. If executing in user thread context,
        // use address directly, access to buffer will be protected by
        // try... except. In system thread context, map _locked_ buffer
        // to system address space.
        PVOID Buffer;
        status = Ext2MapBuffer(IrpContext->Irp, &Buffer);
        if (!NT_SUCCESS(status))
            __leave;

        PCCB Ccb;
        Ccb = Ext2GetCCB(IrpContext);

        // get parameters
        FILE_INFORMATION_CLASS FileInformationClass = IrpStack->Parameters.QueryDirectory.FileInformationClass;
        PUNICODE_STRING FileName = (PUNICODE_STRING)IrpStack->Parameters.QueryDirectory.FileName;
        ULONG FileIndex = IrpStack->Parameters.QueryDirectory.FileIndex;
        BOOLEAN RestartScan = IrpStack->Flags & SL_RESTART_SCAN ? TRUE : FALSE;
        BOOLEAN SingleEntry = IrpStack->Flags & SL_RETURN_SINGLE_ENTRY ? TRUE : FALSE;
        BOOLEAN IndexSpecified = IrpStack->Flags & SL_INDEX_SPECIFIED ? TRUE : FALSE;

        // In CDFS driver Ccb can be init only once, do the same thing here...
        BOOLEAN FirstQuery = Ccb->SearchPattern.Buffer == NULL && !Ccb->MatchAll;

        if (FirstQuery)  
        {
            if (FileName == NULL ||
                FileName->Buffer == NULL || 
                FileName->Length == 0 || 
                (FileName->Length == sizeof(WCHAR) &&
                FileName->Buffer[0] == L'*'))
            {
                Ccb->MatchAll = TRUE;
            }
            else
            {
                Ccb->MatchAll = FALSE;
                status = Ext2SetCCBSearchPattern(Ccb, FileName);
                if (!NT_SUCCESS(status))
                    __leave;

                Ccb->ReallyWild = FsRtlDoesNameContainWildCards(&Ccb->SearchPattern);
            }
        }

        // get start index
        ULONG Index;
        if (IndexSpecified)
            Index = FileIndex;
        else if (RestartScan)
            Index = 0;
        else 
            Index = Ccb->Index;

        // if user buffer is bad...
        __try
        {
            status = Ext2ScanDirectory(Vcb, Fcb->Inode, Fcb->InodeNum == EXT2_ROOT_INO,  &Ccb->SearchPattern, 
                &Index, FirstQuery, SingleEntry, Ccb->MatchAll, Ccb->ReallyWild,
                Buffer, Length, FileInformationClass, &Information);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            status = GetExceptionCode();
            Information = 0;
        }

        if (NT_ERROR(status)) // STATUS_BUFFER_OVERFLOW is valid
            __leave;

        // update CCB
        Ccb->Index = Index;
    }
    __finally
    {
        if (FcbResAcquired)
            ExReleaseResourceLite(&Fcb->MainResource);

        if (Ext2CanComplete(IrpContext, status))
        {
            if (status == STATUS_PENDING)
            {
                status = Ext2LockBuffer(IrpContext->Irp, Length);
                if (NT_SUCCESS(status))
                    status = Ext2QueueIrp(IrpContext);
            }
            
            if (status != STATUS_PENDING)
                Ext2CompleteRequest(IrpContext, status, Information);
        }
        else
        {
            NTSTATUS LockStatus = Ext2LockBuffer(IrpContext->Irp, Length);
            if (!NT_SUCCESS(LockStatus))
                status = Ext2CompleteRequest(IrpContext, LockStatus);
        }

    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2ScanDirectory(
                  IN PVCB                   Vcb,
                  IN PEXT2_INODE            DirInode,
                  IN BOOLEAN                IsRootDir,
                  IN PUNICODE_STRING        SearchPattern,
                  IN OUT PULONG             Index,
                  IN BOOLEAN                FirstQuery,
                  IN BOOLEAN                SingleEntry,
                  IN BOOLEAN                MatchAll,
                  IN BOOLEAN                ReallyWild,
                  IN OUT PVOID              Buffer,
                  IN ULONG                  Length,
                  IN FILE_INFORMATION_CLASS FileInformationClass,
                  OUT PULONG                ResultLength
                  )
{
    NTSTATUS status = STATUS_SUCCESS;
    EXT2_DIR Dir;
    Ext2InitializeDir(&Dir);
    PEXT2_DIR_ENTRY Entry = NULL; 

    __try
    {
        if (!Ext2IsInformationClassSupported(FileInformationClass))
        {
            status = STATUS_INVALID_INFO_CLASS;
            __leave;
        }

        Entry = (PEXT2_DIR_ENTRY)ExAllocatePoolWithTag(PagedPool,
            sizeof(EXT2_DIR_ENTRY), EXT2_ENTRY_TAG);

        if (Entry == NULL)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        status = Ext2ReadDir(Vcb, DirInode, !IsRootDir, &Dir);
        if (!NT_SUCCESS(status))
            __leave;

        *ResultLength = 0;
        ULONG LastEntryLength, PrevOffset, Offset = 0;
        ULONG EntryCount = Ext2GetEntryCount(&Dir);
        BOOLEAN StopScan = FALSE;
        BOOLEAN Find = FALSE;
        ULONG LastIndex = *Index;
        GENERATE_NAME_CONTEXT Context;
        RtlZeroMemory(&Context, sizeof(GENERATE_NAME_CONTEXT));

        for (ULONG i = LastIndex; i < EntryCount && !StopScan; i++)
        {
            Ext2GetEntry(&Dir, i, Entry);
            if (MatchAll || Ext2CompareNameWithWildCard(&Entry->Name, SearchPattern, ReallyWild, FALSE))
            {
                EXT2_INODE EntryInode;
                BOOLEAN InodeRequired = Ext2IsInodeRequired(FileInformationClass);
                if (InodeRequired)
                {
                    status = Ext2ReadInode(Vcb, Entry->Inode, &EntryInode);
                    if (!NT_SUCCESS(status))
                        __leave;
                }

                status = Ext2GetDirEntryInformation(Entry, &EntryInode,
                    Buffer, Offset, Length, FileInformationClass, Vcb->SuperBlock.BlockSize,
                    &LastEntryLength, &Context);

                if (LastEntryLength != 0)
                {
                    PrevOffset = Offset;
                    *ResultLength = Offset + LastEntryLength;
                    Offset = ALIGN(*ResultLength, 8, ULONG); // quadword align
                    SET_ENTRY_OFFSET(Buffer, PrevOffset, Offset - PrevOffset);
                    LastIndex = i;
                }

                StopScan = status == STATUS_BUFFER_OVERFLOW ||
                    LastEntryLength == 0 || // not enough space
                    SingleEntry ||
                    Offset >= Length; // align result

                Find = TRUE;
            }
        }
        if (Find)
        {
            SET_ENTRY_OFFSET(Buffer, PrevOffset, 0); // last entry
            *Index = LastIndex + 1;
        }
        else
        {
            status = FirstQuery ? STATUS_NO_SUCH_FILE 
                : STATUS_NO_MORE_FILES;
        }
    }
    __finally
    {
        Ext2UninitializeDir(&Dir);

        if (Entry)
            ExFreePool(Entry);
    }
    return status;
};

/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOLEAN
Ext2IsInodeRequired(
                    FILE_INFORMATION_CLASS FileInformationClass
                    )
{
    return FileInformationClass != FileNamesInformation;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOLEAN
Ext2IsInformationClassSupported(
                                IN FILE_INFORMATION_CLASS FileInformationClass
                                )
{
    BOOLEAN Result = FALSE;
    switch (FileInformationClass)
    {
    case FileDirectoryInformation:
    case FileFullDirectoryInformation:
    case FileBothDirectoryInformation:
    case FileNamesInformation:
        Result = TRUE;
        break;
    }
    return Result;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOLEAN
Ext2CompareNameWithWildCard(
                            IN PUNICODE_STRING Name,
                            IN PUNICODE_STRING WildCard,
                            IN BOOLEAN ReallyWild,
                            IN BOOLEAN CaseInsensetive
                            )
{
    if (ReallyWild)
        return FsRtlIsNameInExpression(Name, WildCard, CaseInsensetive, NULL);
    else
        return RtlEqualUnicodeString(Name, WildCard, CaseInsensetive);
}

/////////////////////////////////////////////////////////////////////////////
// Ext2GetDirEntryInformation(...)
// Rem: if there is space for entry, fill it.
//      If not, and it's first entry in buffer fill as much
//      as possible, then return STATUS_BUFFER_OVERFLOW.
//      Else, simply return STATUS_SUCCESS.

extern "C"
NTSTATUS
Ext2GetDirEntryInformation(
                           IN PEXT2_DIR_ENTRY        Entry,
                           IN PEXT2_INODE            Inode,
                           IN PVOID                  Buffer,
                           IN ULONG                  Offset,
                           IN ULONG                  Length,
                           IN FILE_INFORMATION_CLASS FileInformationClass,
                           IN ULONG                  AllocSize,             
                           OUT PULONG                EntryLength,
                           OUT PGENERATE_NAME_CONTEXT Context
                           )
{
    // get entry length
    ULONG BaseLength;
    switch (FileInformationClass)
    {
    case FileDirectoryInformation:
        BaseLength = FIELD_OFFSET(FILE_BOTH_DIR_INFORMATION, FileName[0]);
        break;

    case FileFullDirectoryInformation:
        BaseLength = FIELD_OFFSET(FILE_FULL_DIR_INFORMATION, FileName[0]);
        break;

    case FileBothDirectoryInformation:
        BaseLength = FIELD_OFFSET(FILE_BOTH_DIR_INFORMATION, FileName[0]);
        break;

    case FileNamesInformation:
        BaseLength = FIELD_OFFSET(FILE_NAMES_INFORMATION, FileName[0]);
        break;
        
    default:
        ASSERT(0);
    }

    ULONG FullEntryLength = BaseLength + Entry->Name.Length;

    // is there enough space in buffer ?
    NTSTATUS status = STATUS_SUCCESS;
    if (FullEntryLength > Length - Offset)
    {
        if (Offset == 0)
        {
            // IO manager garantees that buffer 
            // is at least BaseLength bytes length
            FullEntryLength = Length;
            FullEntryLength &= ~0x1; // unicode string
            status = STATUS_BUFFER_OVERFLOW;
        }
        else
            FullEntryLength = 0;
    }

    if (FullEntryLength != 0)
    {
        
        ULONG FileNameLength = FullEntryLength - BaseLength;

        PFILE_BOTH_DIR_INFORMATION Info =
            (PFILE_BOTH_DIR_INFORMATION)BUFFER(Buffer, Offset);

        switch (FileInformationClass)
        {
        case FileDirectoryInformation:
        case FileFullDirectoryInformation:
        case FileBothDirectoryInformation:
            Info->FileIndex = Entry->Inode;
            Info->CreationTime.QuadPart = Ext2TimeUnixToWin(Inode->CTime);
            Info->LastAccessTime.QuadPart = Ext2TimeUnixToWin(Inode->ATime);
            Info->LastWriteTime.QuadPart = Ext2TimeUnixToWin(Inode->MTime);
            Info->ChangeTime.QuadPart = Ext2TimeUnixToWin(Inode->MTime);
            Info->EndOfFile.QuadPart = Inode->Size;
            Info->AllocationSize.QuadPart = ALIGN(Inode->Size, AllocSize, ULONGLONG);
            Info->FileAttributes = Ext2GetInodeAttributes(Inode);
            Info->FileNameLength = FileNameLength;
            break;

        case FileNamesInformation:
            {
                PFILE_NAMES_INFORMATION NamesInfo = 
                    (PFILE_NAMES_INFORMATION)BUFFER(Buffer, Offset);
                NamesInfo->FileIndex = Entry->Inode;
                NamesInfo->FileNameLength = FileNameLength;
            }
            break;
        }

        if (FileInformationClass == FileFullDirectoryInformation)
        {
            Info->EaSize = 0;
        }
        else if (FileInformationClass == FileBothDirectoryInformation)
        {
            Info->EaSize = 0;
            if (RtlIsNameLegalDOS8Dot3(&Entry->Name, NULL, NULL))
            {
                Info->ShortNameLength = 0;
            }
            else
            {
                UNICODE_STRING ShortName;
                ShortName.Buffer = Info->ShortName;
                ShortName.MaximumLength = sizeof(Info->ShortNameLength);
                ShortName.Length = 0;
                RtlGenerate8dot3Name(&Entry->Name, FALSE, Context, &ShortName);
                Info->ShortNameLength = (UCHAR)ShortName.Length;
            }
        }

        if (FileNameLength != 0)
        {
            RtlCopyMemory(BUFFER(Buffer, Offset + BaseLength),
                Entry->Name.Buffer, FileNameLength);
        }
    }

    *EntryLength = FullEntryLength;
    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2NotifyChangeDirectory(
                          PIRP_CONTEXT   IrpContext
                          )
{
    PAGED_CODE();

    ASSERT(IrpContext);

    PVCB Vcb; PFCB Fcb;
    Ext2GetFCBandVCB(IrpContext, &Vcb, &Fcb);

    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN VcbResAcquired = FALSE;

    __try
    {
        if (Fcb->NodeTypeCode != FCB_TYPE)
        {
            status = STATUS_INVALID_PARAMETER;
            __leave;
        }

        if (!S_ISDIR(Fcb->Inode->Mode))
        {
            status = STATUS_INVALID_PARAMETER;
            __leave;
        }

        ExAcquireResourceSharedLite(&Vcb->MainResource, TRUE);
        VcbResAcquired = TRUE;

        PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(IrpContext->Irp);
        ULONG CompletionFilter = IrpStack->Parameters.NotifyDirectory.CompletionFilter;
        BOOLEAN WatchTree = IrpStack->Flags & SL_WATCH_TREE ? TRUE : FALSE;
        PCCB Ccb = Ext2GetCCB(IrpContext);

        FsRtlNotifyFullChangeDirectory(Vcb->NotifySync,
            &Vcb->DirNotifyList,
            Ccb,
            (PSTRING)&IrpStack->FileObject->FileName,
            WatchTree,
            FALSE,
            CompletionFilter,
            IrpContext->Irp,
            NULL,
            NULL);

        status = STATUS_PENDING;
    }
    __finally
    {
        if (VcbResAcquired)
            ExReleaseResourceLite(&Vcb->MainResource);

        if (Ext2CanComplete(IrpContext, status))
        {
            if (status != STATUS_PENDING)
                Ext2CompleteRequest(IrpContext, status);
            else
                Ext2DestroyIrpContext(IrpContext);
        }

    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////
