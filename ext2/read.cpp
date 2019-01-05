#include "stdafx.h"
#include "read.h"
#include "dispatch.h"
#include "ext2api.h"
#include "versup.h"

/////////////////////////////////////////////////////////////////////////////
// Ext2Read(...)
// Expected IRQL: PASSIVE, APC, DISPATCH(NT4?)
// Section: NONpaged.

extern "C"
NTSTATUS
Ext2Read(
         PIRP_CONTEXT    IrpContext
         )
{
    PAGED_CODE();

    ASSERT(IrpContext);

    KdHeader(KD_LEV1, "Ext2Read");

    CSHORT Type = Ext2GetFcbType(IrpContext->FileObject);
    if (Type == UNK_TYPE)
        return Ext2CompleteRequest(IrpContext, STATUS_INVALID_PARAMETER);

    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(IrpContext->Irp);
    PFILE_OBJECT FileObject = IrpContext->FileObject;
    PIRP Irp = IrpContext->Irp;
    ULONG Length = IrpStack->Parameters.Read.Length;

    if (FlagOn(IrpContext->MinorFunction, IRP_MN_COMPLETE))
    {
        Ext2MdlReadComplete(FileObject, Irp);
        return Ext2CompleteRequest(IrpContext, STATUS_SUCCESS);
    }

    NTSTATUS status = STATUS_SUCCESS;
    ULONG BytesRead = 0;
    PERESOURCE Resource = NULL;

    __try
    {
        PVCB Vcb = Ext2GetVcb(IrpContext);

        status = Ext2VerifyFcbOperation(IrpContext);
        if (!NT_SUCCESS(status))
            __leave;

        if (FlagOn(IrpContext->MinorFunction, IRP_MN_DPC))
        {
            ClearFlag(IrpContext->MinorFunction, IRP_MN_DPC);
            status = STATUS_PENDING;
            __leave;
        }

        ULONGLONG Offset = IrpStack->Parameters.Read.ByteOffset.QuadPart;
        ULONG Flags = IrpContext->Irp->Flags;
        BOOLEAN PagingIo = IsFlagOn(Flags, IRP_PAGING_IO);
        BOOLEAN NonCachedIo = IsFlagOn(Flags, IRP_NOCACHE);
        BOOLEAN Wait = IrpContext->IsSync;
        BOOLEAN SyncIo = IsFlagOn(FileObject->Flags, FO_SYNCHRONOUS_IO);

        //KdPrint(("Paged = %u, Name = %ws, Offset = %u, Length = %u\n",
          //  PagingIo, FileObject->FileName.Buffer, (ULONG)Offset, Length));

        if (Length == 0)
        {
            // ok
            __leave;
        }

        // 
        // Handle volume read here
        //

        if (Type == VCB_TYPE)
        {
            if (!Wait)
            {
                status = STATUS_PENDING;
                __leave;
            }

            ExAcquireResourceSharedLite(&Vcb->MainResource, TRUE);
            Resource = &Vcb->MainResource;

            if (Offset & (SECTOR_SIZE - 1) || Length & (SECTOR_SIZE - 1))
            {
                status = STATUS_INVALID_PARAMETER;
                __leave;
            }

            ULONGLONG VolumeSize = Vcb->SuperBlock.BlocksCount * 
                Vcb->SuperBlock.BlockSize;

            if (Offset > VolumeSize)
            {
                status = STATUS_END_OF_FILE;
                __leave;
            }

            if (Offset + Length > VolumeSize)
                Length = (ULONG)(VolumeSize - Offset);

            status = Ext2LockBuffer(Irp, Length);
            if (!NT_SUCCESS(status))
                __leave;

            PVOID Buffer;
            status = Ext2MapBuffer(Irp, &Buffer);
            if (!NT_SUCCESS(status))
                __leave;

            status = Ext2ReadVolume(IrpContext, &Offset, Length);

            if (NT_SUCCESS(status))
                BytesRead = Length;

            __leave;
        }
       
        // 
        // Handle file read
        //

        PCOMMON_FCB Fcb = Ext2GetCommonFcb(IrpContext->FileObject);

        Resource = PagingIo ? Fcb->PagingIoResource : Fcb->Resource;
        if (!ExAcquireResourceSharedLite(Resource, Wait))
        {
            Resource = NULL;
            status = STATUS_PENDING;
            __leave;
        }

        ULONGLONG FileSize = Fcb->FileSize.QuadPart;

        if (Offset >= FileSize)
        {
            status = STATUS_END_OF_FILE;
            __leave;
        }

        if (Offset + Length > FileSize)
            Length = (ULONG)(FileSize - Offset);

        if (!PagingIo)
        {
            // TO DO 
            // check locks
        }

        if (NonCachedIo)
        {
            if (!Wait)
            {
                status = STATUS_PENDING;
                __leave;
            }

            // we need Mdl for disk driver,
            // lock buffer anyway
            status = Ext2LockBuffer(Irp, Length);
            if (!NT_SUCCESS(status))
                __leave;

            PVOID Buffer;
            status = Ext2MapBuffer(Irp, &Buffer);
            if (!NT_SUCCESS(status))
                __leave;

            status = Ext2NonCachedRead(IrpContext, Vcb, FileObject, Type, &Offset,
                Length, Buffer);

            if (NT_SUCCESS(status))
            {
                BytesRead = Length;
                if (SyncIo && !PagingIo)
                    FileObject->CurrentByteOffset.QuadPart = Offset + Length;
            }
        }
        else
        {
            // cached i/o
            if (FileObject->PrivateCacheMap == NULL)
            {
                // Initialize cache map only for _user_ file,
                // internal file stream cache map was init.
                // during stream creation.
                ASSERT(Type == FCB_TYPE);

                CC_FILE_SIZES FileSizes;
                FileSizes.AllocationSize = Fcb->AllocationSize;
                FileSizes.FileSize = Fcb->FileSize;
                FileSizes.ValidDataLength = Fcb->ValidDataLength;
                CcInitializeCacheMap(FileObject, &FileSizes, FALSE,
                    &Ext2GlobalData.CacheMgrCallbacks, Fcb);
            }

            if (FlagOn(IrpContext->MinorFunction, IRP_MN_MDL))
            {
                status = Ext2MdlRead(FileObject, &Offset, Length, Irp, 
                    &BytesRead);
                __leave;
            }

            // user buffer has already been probed by NT
            PVOID Buffer;
            status = Ext2MapBuffer(Irp, &Buffer);
            if (!NT_SUCCESS(status))
                __leave;

            IO_STATUS_BLOCK IoStatus;
            __try
            {
                if (!CcCopyRead(FileObject, (PLARGE_INTEGER)&Offset,
                    Length, Wait, Buffer, &IoStatus))
                {
                    status = STATUS_PENDING;
                    __leave;
                }
                status = IoStatus.Status;
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                status = GetExceptionCode();
            }

            if (NT_SUCCESS(status))
            {
                BytesRead = IoStatus.Information;
                if (SyncIo && !PagingIo)
                    FileObject->CurrentByteOffset.QuadPart = Offset + BytesRead;
            }
        }
    }
    __finally
    {
        if (Resource)
            ExReleaseResourceLite(Resource);

        if (Ext2CanComplete(IrpContext, status))
        {
            if (status == STATUS_PENDING)
            {
                status = Ext2LockBuffer(Irp, Length);
                if (NT_SUCCESS(status))
                    status = Ext2QueueIrp(IrpContext);
            }
            if (status != STATUS_PENDING)
            {
                Ext2CompleteRequest(IrpContext, status,
                    NT_SUCCESS(status) ? BytesRead : 0);
            }
        }
        else
        {
            // user unduced error happens
            // we must lock buffer
            NTSTATUS LockStatus = Ext2LockBuffer(Irp, Length);
            if (!NT_SUCCESS(LockStatus))
            {
                status = Ext2CompleteRequest(IrpContext, LockStatus);
            }
        }
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2MdlRead(
            IN PFILE_OBJECT FileObject,
            IN PULONGLONG   Offset,
            IN ULONG        Length,
            IN OUT PIRP     Irp,
            OUT PULONG      BytesLocked
            )
{
    NTSTATUS status;
    __try  // CcMdlRead can raise
    {
        IO_STATUS_BLOCK IoStatus;
        CcMdlRead(FileObject, (PLARGE_INTEGER)Offset, Length, &Irp->MdlAddress, &IoStatus);
        status = IoStatus.Status;
        *BytesLocked = IoStatus.Information;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        status = GetExceptionCode();
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2MdlReadComplete(
                    PFILE_OBJECT    FileObject,
                    PIRP            Irp
                    )
{
    CcMdlReadComplete(FileObject, Irp->MdlAddress);
    Irp->MdlAddress = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2Read(...)
// Expected IRQL: PASSIVE, APC
// Section: NONpaged.

extern "C"
NTSTATUS
Ext2NonCachedRead(
                  IN PIRP_CONTEXT  IrpContext,
                  IN PVCB          Vcb,
                  IN PFILE_OBJECT  FileObject,
                  IN CSHORT        Type,
                  IN PULONGLONG    Offset,
                  IN ULONG         Length,
                  IN PVOID         Buffer
                  )
{
    NTSTATUS status = STATUS_SUCCESS;
    switch (Type)
    {
    case FCB_TYPE:
        {
            // handle simple file or symlink read
            PFCB Fcb;
            Ext2GetFcb(FileObject, &Fcb);
            if (S_ISREG(Fcb->Inode->Mode))
            {
                status = Ext2ReadFile(IrpContext, Vcb, Fcb->Inode, *Offset, 
                    Length, Buffer);
            }
            else if (S_ISLNK(Fcb->Inode->Mode))
            {
                status = Ext2ReadSymbolicLink(Vcb, Fcb->Inode, (ULONG)*Offset,
                    Length, Buffer);
            }
            else
                status = STATUS_ACCESS_DENIED;
        }
        break;

    case MFCB_TYPE:
        {
            // handle meta data read
            PMFCB Fcb;
            Ext2GetFcb(FileObject, &Fcb);
            status = Ext2ReadVolume(IrpContext, Offset, Length);
        }
        break;

    default:
        ASSERT(0);
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////

