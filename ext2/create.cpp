#include "stdafx.h"
#include "create.h"
#include "ext2api.h"
#include "versup.h"

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2Create(
            PIRP_CONTEXT    IrpContext
            )
{
    ASSERT(IrpContext);

    NTSTATUS status;
    PFILE_OBJECT FileObject = IrpContext->FileObject;
    PFILE_OBJECT RelFileObject = FileObject->RelatedFileObject;

    if (IrpContext->Device == Ext2GlobalData.Device)
        status = Ext2CreateFileSystem(IrpContext);
    else if (FileObject->FileName.Length == 0 && 
        (RelFileObject == NULL || (RelFileObject != NULL && Ext2GetFcbType(RelFileObject) == VCB_TYPE)))
    {
        status = Ext2CreateVolume(IrpContext);
    }
    else 
        status = Ext2CreateFile(IrpContext);

    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2CreateFileSystem(
                     PIRP_CONTEXT    IrpContext
                     )
{
    PAGED_CODE();

    ASSERT(IrpContext);

    KdHeader(KD_LEV1, "Ext2CreateFileSystem");

    return Ext2CompleteRequest(IrpContext, STATUS_SUCCESS, FILE_OPENED);
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2CreateVolume(
                 PIRP_CONTEXT    IrpContext
                 )
{
    PAGED_CODE();

    ASSERT(IrpContext);

    KdHeader(KD_LEV1, "Ext2CreateVolume");

    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN VcbResAcquired = FALSE;
    PVCB Vcb = Ext2GetVcb(IrpContext);

    __try
    {
        PIO_STACK_LOCATION  IrpStack = IoGetCurrentIrpStackLocation(IrpContext->Irp);
        ULONG Options = IrpStack->Parameters.Create.Options & FILE_VALID_OPTION_FLAGS;
        ULONG Disposition = (IrpStack->Parameters.Create.Options >> 24) & 0xFF;
        BOOLEAN OpenTargetDir = IrpStack->Flags & SL_OPEN_TARGET_DIRECTORY;
        PFILE_OBJECT FileObject = IrpContext->FileObject;

        if (Disposition != FILE_OPEN && Disposition != FILE_OPEN_IF)
        {
            status = STATUS_ACCESS_DENIED;
            __leave;
        }

        if (Options & FILE_DIRECTORY_FILE)
        {
            status = STATUS_NOT_A_DIRECTORY;
            __leave;
        }

        if (OpenTargetDir)
        {
            status = STATUS_INVALID_PARAMETER;
            __leave;
        }

        ExAcquireResourceExclusiveLite(&Vcb->MainResource, TRUE);
        VcbResAcquired = TRUE;

        // verify volume
        status = Ext2VerifyVcb(IrpContext, Vcb);
        if (!NT_SUCCESS(status))
            __leave;

        IrpContext->FileObject->FsContext = Vcb;
        IrpContext->FileObject->ReadAccess = TRUE;

        Vcb->ReferenceCount++;
    }
    __finally
    {
        if (VcbResAcquired)
            ExReleaseResourceLite(&Vcb->MainResource);

        if (Ext2CanComplete(IrpContext, status))
        {
            Ext2CompleteRequest(IrpContext, status,
                status == STATUS_SUCCESS ? FILE_OPENED : 0);
        }
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2CreateFile(
               PIRP_CONTEXT    IrpContext
               )
{
    ASSERT(IrpContext);

    PAGED_CODE();

    KdHeader(KD_LEV1, "Ext2CreateFile");
    
    /*
    PVCB Vcb = (PVCB)IrpContext->Device->DeviceExtension;
    UNICODE_STRING s;
    ULONG i = 0;
    NTSTATUS status;
    
      RtlInitUnicodeString(&s, L"\\dev\\rd");
      status = Ext2NameToInode(Vcb, &s, &i);
      KdPrint(("Status = %x, Inode = %X\n", status, i));
      
        EXT2_INODE In;
        status = Ext2ReadInode(Vcb, i, &In);
        PCHAR Buf = (PCHAR)ExAllocatePool(PagedPool, 0x2000);
        status = Ext2ReadFile(Vcb, &In, 0x100, 0x2000, Buf);
        ExFreePool(Buf);
        
          Buf = (PCHAR)ExAllocatePool(PagedPool, 0x800);
          status = Ext2ReadFile(Vcb, &In, 0x200, 0x800, Buf);
          ExFreePool(Buf);
    */
    PVCB Vcb = (PVCB)IrpContext->Device->DeviceExtension;
    ASSERT(Vcb);
    ASSERT(Vcb->NodeTypeCode == VCB_TYPE);
    PFCB Fcb;
    
    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN VcbResAcquired = FALSE;
    BOOLEAN FcbResAcquired = FALSE;
    PWSTR FileName = NULL;
    BOOLEAN NewFcb = FALSE;

    __try
    {
        PIO_STACK_LOCATION  IrpStack = IoGetCurrentIrpStackLocation(IrpContext->Irp);

        ULONG Options = IrpStack->Parameters.Create.Options & FILE_VALID_OPTION_FLAGS;
        ULONG Disposition = (IrpStack->Parameters.Create.Options >> 24) & 0xFF;
        BOOLEAN DirOnly = Options & FILE_DIRECTORY_FILE ? TRUE : FALSE;
        BOOLEAN FileOnly = Options & FILE_NON_DIRECTORY_FILE ? TRUE : FALSE;
        BOOLEAN OpenByID = Options & FILE_OPEN_BY_FILE_ID ? TRUE : FALSE;
        BOOLEAN OpenTargetDir = IrpStack->Flags & SL_OPEN_TARGET_DIRECTORY;
        BOOLEAN PageFile = IrpStack->Flags & SL_OPEN_PAGING_FILE;
        BOOLEAN CaseSens = IrpStack->Flags & SL_CASE_SENSITIVE;
        BOOLEAN ExtrAttrBuf = (BOOLEAN)IrpContext->Irp->AssociatedIrp.SystemBuffer;
 
        if (Disposition != FILE_OPEN && Disposition != FILE_OPEN_IF)
        {
            status = STATUS_ACCESS_DENIED;
            __leave;
        }

        if (OpenTargetDir || ExtrAttrBuf || OpenByID)
        {
            status = STATUS_INVALID_PARAMETER;
            __leave;
        }

        ExAcquireResourceExclusiveLite(&Vcb->MainResource, TRUE);
        VcbResAcquired = TRUE;

        // check if volume locked
        if (FlagOn(Vcb->Flags, VCB_LOCKED))
        {
            status = STATUS_ACCESS_DENIED;
            __leave;
        }

        // verify volume
        status = Ext2VerifyVcb(IrpContext, Vcb);
        if (!NT_SUCCESS(status))
            __leave;

        // get inode number
        ULONG InodeNum;

        BOOLEAN TrailSlash;
        status = Ext2AllocateName(IrpContext->FileObject, 
            &FileName, &TrailSlash);
        if (!NT_SUCCESS(status))
            __leave;
        
        UNICODE_STRING Name;
        RtlInitUnicodeString(&Name, FileName);
        status = Ext2NameToInode(Vcb, &Name, &InodeNum);
        if (!NT_SUCCESS(status))
            __leave;

        // get FCB
        Fcb = Ext2LookupFCB(Vcb, InodeNum);
        if (Fcb == NULL)
        {
            Fcb = Ext2CreateFCB(InodeNum, FileName);
            if (Fcb == NULL)
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
                __leave;
            }

            NewFcb = TRUE;

            // read inode
            status = Ext2ReadInode(Vcb, InodeNum, Fcb->Inode);
            if (!NT_SUCCESS(status))
            {
                __leave;
            }

            Ext2SetFcbFileSizes(Fcb, Vcb->SuperBlock.BlockSize);
        }
        PFILE_OBJECT FileObject = IrpContext->FileObject;
        FileObject->SectionObjectPointer = &Fcb->SectionObject;
        FileObject->Vpb = Vcb->Vpb;
        FileObject->ReadAccess = TRUE;
        FileObject->WriteAccess = FALSE;
        FileObject->DeleteAccess = FALSE;

        ExAcquireResourceExclusiveLite(&Fcb->MainResource, TRUE);
        FcbResAcquired = TRUE;

        // check inode
        if ((TrailSlash || DirOnly) && !S_ISDIR(Fcb->Inode->Mode))
        {
            status = STATUS_NOT_A_DIRECTORY;
            __leave;
        }

        if (FileOnly && S_ISDIR(Fcb->Inode->Mode))
        {
            status = STATUS_FILE_IS_A_DIRECTORY;
            __leave;
        }

        PCCB Ccb = Ext2CreateCCB();
        if (Ccb == NULL)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        if (NewFcb)
            Ext2AddFCB(Vcb, Fcb);

        FileObject->FsContext2 = Ccb;
        FileObject->FsContext = Fcb;

        if (FlagOn(IrpStack->Parameters.Create.Options, FILE_NO_INTERMEDIATE_BUFFERING))
            SetFlag(FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING);
        else
            SetFlag(FileObject->Flags, FO_CACHE_SUPPORTED);

        Fcb->OpenHandleCount++;
        Fcb->ReferenceCount++;
        Vcb->ReferenceCount++;

        KdPrint(("Create: Inode = %x, Handle = %x, Ref = %x\n", Fcb->InodeNum, Fcb->OpenHandleCount, Fcb->ReferenceCount));
    }
    __finally
    {
        if (FileName && !NewFcb)
            ExFreePool(FileName);

        if (FcbResAcquired)
            ExReleaseResourceLite(&Fcb->MainResource);

        if (!NT_SUCCESS(status) && NewFcb)
            Ext2FreeFCB(Fcb);

        if (VcbResAcquired)
            ExReleaseResourceLite(&Vcb->MainResource);

        if (Ext2CanComplete(IrpContext, status))
        {
            Ext2CompleteRequest(IrpContext, status,
                status == STATUS_SUCCESS ? FILE_OPENED : 0);
        }
    }

    return status;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2AllocateName
// IRQL: PASSIVE

extern "C"
NTSTATUS
Ext2AllocateName(
                 IN PFILE_OBJECT    FileObject,
                 OUT PWSTR          *FileNameRes,
                 OUT PBOOLEAN       TrailSlash
                 )
{
    PWSTR FullFileName;
    BOOLEAN TrailingSlash = FALSE;
    PUNICODE_STRING FileName = &FileObject->FileName;
    PFILE_OBJECT RelFileObject = FileObject->RelatedFileObject;
    if (RelFileObject)
    {
        BOOLEAN ValidRel = FALSE;
        PFCB Fcb;
        if (Ext2GetFcbType(RelFileObject) == FCB_TYPE)
        {
            Ext2GetFcb(RelFileObject, &Fcb);
            if (S_ISDIR(Fcb->Inode->Mode))
                ValidRel = TRUE;
        }

        if (!ValidRel)
            return STATUS_INVALID_PARAMETER;

        ULONG Length = FileName->Length;
        // trailing '\' ? 
        if (Length >= sizeof(WCHAR) && FileName->Buffer[Length/2 - 1] == '\\')
        {
            TrailingSlash = TRUE;
            Length -= sizeof(WCHAR);
        }

        PUNICODE_STRING RelFileName = &Fcb->Name;
        ULONG RelLength = RelFileName->Length;
        ULONG SeparatorLength = 0;

        if (Length >= sizeof(WCHAR))
        {
            if (FileName->Buffer[0] == '\\')
                return STATUS_INVALID_PARAMETER;
            SeparatorLength = RelLength > sizeof(WCHAR) ? sizeof(WCHAR) : 0;
        }

        FullFileName = (PWSTR)ExAllocatePoolWithTag(PagedPool,
            RelLength + SeparatorLength + Length + sizeof(WCHAR), EXT2_FILENAME_TAG);

        if (FullFileName == NULL)
            return STATUS_INSUFFICIENT_RESOURCES;

        RtlCopyMemory(FullFileName, RelFileName->Buffer, RelLength);
        if (Length > 0)
        {
            RtlCopyMemory(FullFileName + (RelLength + SeparatorLength)/2,
                FileName->Buffer, Length);
        }
        if (SeparatorLength > 0)
            FullFileName[RelLength/2] = '\\';
        FullFileName[(RelLength + SeparatorLength + Length)/2] = '\0';
    }
    else
    {
        if (FileName->Buffer == NULL || FileName->Length < sizeof(WCHAR))
            return STATUS_INVALID_PARAMETER;

        ULONG Length = FileName->Length;

        if (FileName->Buffer[0] != '\\')
            return STATUS_INVALID_PARAMETER;

        // trailing '\' ? 
        if (Length > sizeof(WCHAR) && FileName->Buffer[Length/2 - 1] == '\\')
        {
            TrailingSlash = TRUE;
            Length -= sizeof(WCHAR);
        }

        FullFileName = (PWSTR)ExAllocatePoolWithTag(PagedPool,
            Length + sizeof(WCHAR), EXT2_FILENAME_TAG);

        if (FullFileName == NULL)
            return STATUS_INSUFFICIENT_RESOURCES;

        RtlCopyMemory(FullFileName, FileName->Buffer, Length);
        FullFileName[Length/2] = '\0';
    }

    *FileNameRes = FullFileName;
    *TrailSlash = TrailingSlash;

    return STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////