#include "stdafx.h"
#include "volumeinfo.h"
#include "versup.h"

/////////////////////////////////////////////////////////////////////////////
// Ext2QueryVolumeInfo(...)
// Expected IRQL: PASSIVE;
// Section: paged.
// Cause page errors: YES(internal)

extern "C"
NTSTATUS
Ext2QueryVolumeInformation(
                           PIRP_CONTEXT IrpContext
                           )
{
    PAGED_CODE();

    ASSERT(IrpContext);

    KdHeader(KD_LEV1, "Ext2QueryVolumeInformation");

    CSHORT Type = Ext2GetFcbType(IrpContext->FileObject);
    if (Type == UNK_TYPE)
        return Ext2CompleteRequest(IrpContext, STATUS_INVALID_DEVICE_REQUEST);

    NTSTATUS status = STATUS_SUCCESS;
    ULONG Length = 0;
    BOOLEAN VcbResAcquired = FALSE;
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(IrpContext->Irp);
    PVCB Vcb = Ext2GetVcb(IrpContext);

    __try
    {
        ExAcquireResourceSharedLite(&Vcb->MainResource, TRUE);
        VcbResAcquired = TRUE;

        // verify volume
        status = Ext2VerifyVcb(IrpContext, Vcb);
        if (!NT_SUCCESS(status))
            __leave;
        
        PVOID Buffer = IrpContext->Irp->AssociatedIrp.SystemBuffer;
        Length = IrpStack->Parameters.QueryVolume.Length;

        switch (IrpStack->Parameters.QueryVolume.FsInformationClass)
        {
        case FileFsVolumeInformation:
            status = Ext2FsVolumeInformation(Vcb, Buffer, &Length);
            break;

        case FileFsSizeInformation:
            status = Ext2FsSizeInformation(Vcb, Buffer, &Length);
            break;

        case FileFsDeviceInformation:
            status = Ext2FsDeviceInformation(Vcb, Buffer, &Length);
            break;

        case FileFsAttributeInformation:
            status = Ext2FsAttributeInformation(Vcb, Buffer, &Length);
            break;

        default:
            status = STATUS_INVALID_PARAMETER;
        }
    }
    __finally
    {
        if (VcbResAcquired)
            ExReleaseResourceLite(&Vcb->MainResource);

        if (Ext2CanComplete(IrpContext, status))
        {
            Ext2CompleteRequest(IrpContext, status,
                !NT_ERROR(status) ? Length : 0);
        }
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2FsVolumeInformation(
                        IN PVCB         Vcb,
                        PVOID           Buffer,
                        IN OUT PULONG   Length
                        )
{
    PAGED_CODE();

    ASSERT(Vcb && Buffer);

    KdHeader(KD_LEV2, "Ext2FsVolumeInformation");

    PFILE_FS_VOLUME_INFORMATION Info = (PFILE_FS_VOLUME_INFORMATION)Buffer;
    Info->VolumeCreationTime.QuadPart = Ext2TimeUnixToWin(Vcb->SuperBlock.MountTime);
    Info->VolumeSerialNumber = 0;
    Info->SupportsObjects = FALSE;
    Info->VolumeLabelLength = 0;

    *Length = sizeof(FILE_FS_VOLUME_INFORMATION);
    return STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2FsSizeInformation(
                      IN PVCB         Vcb,
                      PVOID           Buffer,
                      IN OUT PULONG   Length
                      )
{
    ASSERT(Vcb && Buffer);

    PAGED_CODE();

    KdHeader(KD_LEV2, "Ext2FsSizeInformation");

    PFILE_FS_SIZE_INFORMATION Info = (PFILE_FS_SIZE_INFORMATION)Buffer;

    Info->TotalAllocationUnits.QuadPart = Vcb->SuperBlock.BlocksCount;
    // read-only driver, not file system
    Info->AvailableAllocationUnits.QuadPart = Vcb->SuperBlock.FreeBlocksCount; 
    Info->SectorsPerAllocationUnit = Vcb->SuperBlock.BlockSize>>9;
    Info->BytesPerSector = 512;

    *Length = sizeof(FILE_FS_SIZE_INFORMATION);
    return STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2FsAttributeInformation(
                           IN PVCB         Vcb,
                           PVOID           Buffer,
                           IN OUT PULONG   Length
                           )
{
    ASSERT(Vcb && Buffer);

    PAGED_CODE();

    KdHeader(KD_LEV2, "Ext2FsAttributeInformation");

    PFILE_FS_ATTRIBUTE_INFORMATION Info = (PFILE_FS_ATTRIBUTE_INFORMATION)Buffer;

    Info->FileSystemAttributes = FILE_CASE_SENSITIVE_SEARCH; // ?
    Info->MaximumComponentNameLength = 255;  // make const

    NTSTATUS status = STATUS_SUCCESS;
    ULONG BytesToCopy = *Length - FIELD_OFFSET(FILE_FS_ATTRIBUTE_INFORMATION, FileSystemName);
    if (BytesToCopy < 8)
        status = STATUS_BUFFER_OVERFLOW;
    else
        BytesToCopy = 8;

    Info->FileSystemNameLength = BytesToCopy;

    BOOLEAN Ext3 = Vcb->SuperBlock.RevLevel == EXT2_DYNAMIC_REV &&
        FlagOn(Vcb->SuperBlock.FeatureCompat, EXT2_FEATURE_COMPAT_HAS_JOURNAL);
    RtlCopyMemory(Info->FileSystemName, Ext3 ? L"ext3" : L"ext2", BytesToCopy);

    *Length = FIELD_OFFSET(FILE_FS_ATTRIBUTE_INFORMATION, FileSystemName) + BytesToCopy;
    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2FsDeviceInformation(
                           IN PVCB         Vcb,
                           PVOID           Buffer,
                           IN OUT PULONG   Length
                           )
{
    ASSERT(Vcb && Buffer);

    PAGED_CODE();

    KdHeader(KD_LEV2, "Ext2FsDeviceInformation");

    PFILE_FS_DEVICE_INFORMATION Info = (PFILE_FS_DEVICE_INFORMATION)Buffer;
    Info->DeviceType = FILE_DEVICE_DISK;
    Info->Characteristics = Vcb->TargetDevice->Characteristics;
    *Length = sizeof(FILE_FS_ATTRIBUTE_INFORMATION);

    return STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
