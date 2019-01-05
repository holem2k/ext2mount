#include "stdafx.h"
#include "fscontrol.h"
#include "ext2api.h"
#include "disk.h"
#include "versup.h"

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2FileSystemControl(
                      PIRP_CONTEXT  IrpContext
                      )
{
    NTSTATUS status;
    switch (IrpContext->MinorFunction)
    {
    case IRP_MN_MOUNT_VOLUME:
        status = Ext2Mount(IrpContext);
        break;

    case IRP_MN_VERIFY_VOLUME:
        status = Ext2VerifyVolume(IrpContext);
        break;

    case IRP_MN_USER_FS_REQUEST:
        status = Ext2UserFsRequest(IrpContext);
        break;

    default:
        IrpContext->Irp->IoStatus.Status = status = STATUS_INVALID_DEVICE_REQUEST;
        IoCompleteRequest(IrpContext->Irp, IO_NO_INCREMENT); 
        Ext2DestroyIrpContext(IrpContext);
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2Mount(
          PIRP_CONTEXT  IrpContext
          )
{
    ASSERT(IrpContext);
    ASSERT(IrpContext->IsSync);

    KdHeader(KD_LEV1, "Ext2Mount");

    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN DataResAcquired = FALSE;
    PEXT2_SUPERBLOCK SuperBlock = NULL;
    PEXT2_GROUP_DESC GroupDesc = NULL;
    PDEVICE_OBJECT VolumeDevice = NULL;
    PVCB RemountVcb = NULL;
    ULONG GroupCount;

    __try
    {
        ExAcquireResourceExclusiveLite(&Ext2GlobalData.DataResource, TRUE);
        DataResAcquired = TRUE;

        Ext2ScanForDismount();

        SuperBlock = (PEXT2_SUPERBLOCK)ExAllocatePoolWithTag(PagedPool,
            sizeof(EXT2_SUPERBLOCK), EXT2_SB_TAG);

        if (SuperBlock == NULL)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(IrpContext->Irp);
        PVPB Vpb = IrpStack->Parameters.MountVolume.Vpb;
        PDEVICE_OBJECT TargetDevice = IrpStack->Parameters.MountVolume.DeviceObject;

        ULONG ChangeCount;
        status = Ext2PingDevice(TargetDevice, TRUE, &ChangeCount);
        if (!NT_SUCCESS(status))
            __leave;

        // read superblock
        status = Ext2ReadSuperBlock(TargetDevice, SuperBlock);
        if (!NT_SUCCESS(status))
            __leave;

        status = Ext2CheckSuperBlock(SuperBlock);
        if (!NT_SUCCESS(status))
            __leave;
        
        RemountVcb = Ext2TryRemount(SuperBlock, Vpb->RealDevice);
        if (RemountVcb == NULL)
        {
            // read group descriptors
            status = Ext2AllocateAndReadGroupDesc(TargetDevice,
                SuperBlock,
                &GroupCount,
                &GroupDesc);

            if (!NT_SUCCESS(status))
                __leave;
            
            status = IoCreateDevice(Ext2GlobalData.Driver, 
                sizeof(VCB),
                NULL,
                FILE_DEVICE_DISK_FILE_SYSTEM,
                0,
                FALSE,
                &VolumeDevice);
            
            if (!NT_SUCCESS(status))
                __leave;
            
            VolumeDevice->StackSize = TargetDevice->StackSize + 1;
            Vpb->DeviceObject = VolumeDevice;
            
            // initiliaze VCB
            PVCB Vcb = (PVCB)VolumeDevice->DeviceExtension;
            
            BOOLEAN VcbInit = Ext2InitVCB(Vcb,
                TargetDevice,
                VolumeDevice,
                Vpb,
                SuperBlock,
                GroupDesc,
                GroupCount,
                &Ext2GlobalData.CodePageData,
                ChangeCount);

            if (!VcbInit)
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
                __leave;
            }
            
            ClearFlag(VolumeDevice->Flags, DO_DEVICE_INITIALIZING);

            Vpb->ReferenceCount += 1;

            // if it's mount from verify->mount path then clear flag
            ClearFlag(Vpb->RealDevice->Flags, DO_VERIFY_VOLUME);
        }
        else
        {
            // make remount operation
            RemountVcb->ReferenceCount += MOUNT_REF;
            RemountVcb->VcbState = VCB_MOUNTED;

            // пляски с бубном
            // Vpb comes with zero ReferenceCount,
            // put old Vpb instead new, then delete new Vpb
            PVPB OldVpb = RemountVcb->Vpb;
            OldVpb->RealDevice->Vpb = OldVpb;
            RemountVcb->TargetDevice = TargetDevice;
            PVPB *IrpVpb = &IoGetCurrentIrpStackLocation(IrpContext->Irp)->Parameters.MountVolume.Vpb;
            if (*IrpVpb == Vpb) // (C) M$
            {
                *IrpVpb = RemountVcb->Vpb;
            }
            ExFreePool(Vpb);

            // if it's mount from verify->mount path then clear flag
            ClearFlag(OldVpb->RealDevice->Flags, DO_VERIFY_VOLUME);
        }

    }
    __finally
    {
        if (!NT_SUCCESS(status) && VolumeDevice)
            IoDeleteDevice(VolumeDevice);

        if (RemountVcb)
            ExReleaseResourceLite(&RemountVcb->MainResource);

        if (DataResAcquired)
            ExReleaseResourceLite(&Ext2GlobalData.DataResource);

        if (SuperBlock)
            ExFreePool(SuperBlock);

        if (!NT_SUCCESS(status) && GroupDesc)
            Ext2FreeGroupDesc(GroupDesc);

        if (!IrpContext->ExceptionInProgress)
            Ext2CompleteRequest(IrpContext, status);
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2VerifyVolume(
                 PIRP_CONTEXT  IrpContext
                 )
{
    PAGED_CODE();

    ASSERT(IrpContext);
    ASSERT(Ext2GetVcb(IrpContext)->VcbState == VCB_MOUNTED || Ext2GetVcb(IrpContext)->VcbState == VCB_DISMOUNTED);

    KdHeader(KD_LEV1, "Ext2VerifyVolume");
    KdPrint(("Volume - %X\n", Ext2GetVcb(IrpContext)->VolumeDevice));

    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(IrpContext->Irp);
    PVCB Vcb = (PVCB)IrpStack->Parameters.VerifyVolume.DeviceObject->DeviceExtension;
    PVPB Vpb = IrpStack->Parameters.VerifyVolume.Vpb;
    PDEVICE_OBJECT RealDevice = Vpb->RealDevice;
    BOOLEAN VcbResAcquired = FALSE;
    PEXT2_SUPERBLOCK SuperBlock = NULL;
    BOOLEAN Umount = FALSE;

    __try
    {
        ExAcquireResourceExclusiveLite(&Vcb->MainResource, TRUE);
        VcbResAcquired = TRUE;

        if (!IsFlagOn(RealDevice->Flags, DO_VERIFY_VOLUME))
        {
            // ok
            __leave;
        }

        if (Vcb->VcbState == VCB_DISMOUNTED)
        {
            //ClearFlag(RealDevice->Flags, DO_VERIFY_VOLUME);
            status = STATUS_WRONG_VOLUME;
            __leave;
        }

        ULONG ChangeCount;
        status = Ext2PingDevice(Vcb->TargetDevice, TRUE, &ChangeCount);
        if (!NT_SUCCESS(status))
            return status; // empty, not ready

        SuperBlock = (PEXT2_SUPERBLOCK)ExAllocatePool(PagedPool, sizeof(EXT2_SUPERBLOCK));
        if (SuperBlock == NULL)
        {
            Umount = TRUE;
            __leave;
        }

        status = Ext2ReadSuperBlock(Vcb->TargetDevice, SuperBlock);
        if (!NT_SUCCESS(status))
        {
            // force dismount
            Umount = TRUE;
            __leave;
        }

        Umount = !Ext2EqualSuperBlock(&Vcb->SuperBlock, SuperBlock);

        if (!Umount)
            ClearFlag(RealDevice->Flags, DO_VERIFY_VOLUME);
    }
    __finally
    {
        if (Umount)
        {
            status = STATUS_WRONG_VOLUME;
            Vcb->VcbState = VCB_NOT_MOUNTED;
            Vcb->ReferenceCount--;
            VcbResAcquired = !Ext2CheckForDismount(Vcb);
        }

        if (SuperBlock)
            ExFreePool(SuperBlock);

        if (VcbResAcquired)
            ExReleaseResourceLite(&Vcb->MainResource);

        if (!IrpContext->ExceptionInProgress) // 
            Ext2CompleteRequest(IrpContext, status);
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2UserFsRequest(
                  PIRP_CONTEXT  IrpContext
                  )
{
    PAGED_CODE();

    ASSERT(IrpContext);

    NTSTATUS status;
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(IrpContext->Irp);
    ULONG FsControlCode = IrpStack->Parameters.FileSystemControl.FsControlCode;

    switch (FsControlCode)
    {
    case FSCTL_IS_VOLUME_MOUNTED:
        status = Ext2IsVolumeMounted(IrpContext);
        break;

    case FSCTL_LOCK_VOLUME:
        status = Ext2LockVolume(IrpContext);
        break;

    case FSCTL_UNLOCK_VOLUME:
        status = Ext2UnlockVolume(IrpContext);
        break;

    case FSCTL_DISMOUNT_VOLUME:
        status = Ext2DismountVolume(IrpContext);
        break;

    default:
        status = Ext2CompleteRequest(IrpContext, STATUS_INVALID_DEVICE_REQUEST);
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2IsVolumeMounted(
                    PIRP_CONTEXT  IrpContext
                    )
{
    PAGED_CODE();

    ASSERT(IrpContext);

    KdHeader(KD_LEV1, "Ext2IsVolumeMounted");
    KdPrint(("Volume - %X\n", Ext2GetVcb(IrpContext)->VolumeDevice));

    CSHORT Type = Ext2GetFcbType(IrpContext->FileObject);
    if (Type == UNK_TYPE)
        Ext2CompleteRequest(IrpContext, STATUS_INVALID_PARAMETER);

    IrpContext->PopupOnError = FALSE;
    PVCB Vcb = Ext2GetVcb(IrpContext);
    NTSTATUS status = Ext2VerifyVcb(IrpContext, Vcb);
    if (Ext2CanComplete(IrpContext, status))
        Ext2CompleteRequest(IrpContext, status);

    return status;
}


/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2LockVolume(
               PIRP_CONTEXT  IrpContext
               )
{
    PAGED_CODE();

    ASSERT(IrpContext);

    KdHeader(KD_LEV1, "Ext2LockVolume");

    if (Ext2GetFcbType(IrpContext->FileObject) != VCB_TYPE)
        return Ext2CompleteRequest(IrpContext, STATUS_INVALID_PARAMETER);

    PVCB Vcb = Ext2GetVcb(IrpContext);
    
    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN VcbResAcquired = FALSE;
    PFILE_OBJECT FileObject = IrpContext->FileObject;

    __try
    {
        // TO DO
        // MM may still keep reference to file objects.
        // If so, it must be forced to release this ref.

        FsRtlNotifyVolumeEvent(FileObject, FSRTL_VOLUME_LOCK);

        ExAcquireResourceExclusiveLite(&Vcb->MainResource, TRUE);
        VcbResAcquired = TRUE;

        if (FlagOn(Vcb->Flags, VCB_LOCKED) &&
            Vcb->VolumeLockFile == FileObject)
        {
            // ok
            __leave;
        }

        if (Vcb->ReferenceCount == MOUNT_REF + 1 + 1)
        {
            // Fat driver OR VPB_LOCKED flag, but NTFS and CDFS doesn't... Confused ?
            UCHAR SavedIrql;
            IoAcquireVpbSpinLock(&SavedIrql);
            SetFlag(Vcb->Vpb->Flags, VPB_LOCKED);
            IoReleaseVpbSpinLock (SavedIrql);
            SetFlag(Vcb->Flags, VCB_LOCKED);
            Vcb->VolumeLockFile = FileObject;
        }
        else
            status = STATUS_ACCESS_DENIED;

    }
    __finally
    {
        if (VcbResAcquired)
            ExReleaseResourceLite(&Vcb->MainResource);

        if (!NT_SUCCESS(status))
            FsRtlNotifyVolumeEvent(FileObject, FSRTL_VOLUME_LOCK_FAILED);

        if (Ext2CanComplete(IrpContext, status))
            Ext2CompleteRequest(IrpContext, status);
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2UnlockVolume(
                 PIRP_CONTEXT  IrpContext
                 )
{
    PAGED_CODE();

    ASSERT(IrpContext);

    KdHeader(KD_LEV1, "Ext2UnlockVolume");
    
    PVCB Vcb; PFCB Fcb;
    Ext2GetFCBandVCB(IrpContext, &Vcb, &Fcb);
    
    if (Fcb->NodeTypeCode != VCB_TYPE)
        return Ext2CompleteRequest(IrpContext, STATUS_INVALID_PARAMETER);
 
    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN VcbResAcquired = FALSE;

    __try
    {
        ExAcquireResourceExclusiveLite(&Vcb->MainResource, TRUE);
        VcbResAcquired = TRUE;

        PFILE_OBJECT FileObject = IrpContext->FileObject;

        if (FlagOn(Vcb->Flags, VCB_LOCKED) &&
            FileObject == Vcb->VolumeLockFile)
        {
            UCHAR SavedIrql;
            IoAcquireVpbSpinLock(&SavedIrql);
            ClearFlag(Vcb->Vpb->Flags, VPB_LOCKED);
            IoReleaseVpbSpinLock(SavedIrql);
            ClearFlag(Vcb->Flags, VCB_LOCKED);
            Vcb->VolumeLockFile = NULL;
            FsRtlNotifyVolumeEvent(FileObject, FSRTL_VOLUME_UNLOCK);
        }
        else
            status = STATUS_NOT_LOCKED;

    }
    __finally
    {
        if (VcbResAcquired)
            ExReleaseResourceLite(&Vcb->MainResource);

        if (Ext2CanComplete(IrpContext, status))
            Ext2CompleteRequest(IrpContext, status);
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2DismountVolume

extern "C"
NTSTATUS
Ext2DismountVolume(
                   PIRP_CONTEXT IrpContext
                   )
{
    PAGED_CODE();

    ASSERT(IrpContext);

    KdHeader(KD_LEV1, "Ext2DismountVolume");

    if (Ext2GetFcbType(IrpContext->FileObject) != VCB_TYPE)
        return Ext2CompleteRequest(IrpContext, STATUS_INVALID_PARAMETER);

    NTSTATUS status = STATUS_SUCCESS;
    PVCB Vcb = Ext2GetVcb(IrpContext);
    BOOLEAN VcbResAcquired = FALSE;

    __try
    {
        ExAcquireResourceExclusiveLite(&Vcb->MainResource, TRUE);
        VcbResAcquired = TRUE;

        if (Vcb->VcbState == VCB_DISMOUNTED)
        {
            status = STATUS_VOLUME_DISMOUNTED;
            __leave;
        }

        FsRtlNotifyVolumeEvent(IrpContext->FileObject, FSRTL_VOLUME_DISMOUNT);

        if (Vcb->VcbState == VCB_MOUNTED)
        {
            // kill mount reference.
            // volume will be really dismounted, when                                
            // last user reference released.
            Vcb->ReferenceCount--;
            // set verify flag;
            // Ext2VerifyVolume returns STATUS_WRONG_VOLUME on
            // dismounted volumes.
            SetFlag(Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME);
        }

        Vcb->VcbState = VCB_DISMOUNTED;
    }
    __finally
    {
        if (VcbResAcquired)
            ExReleaseResourceLite(&Vcb->MainResource);

        if (!IrpContext->ExceptionInProgress)
            Ext2CompleteRequest(IrpContext, status);
    }

    return status;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2TryRemount
// LOCKS Ext2GlobalData [SHARE]

extern "C"
PVCB
Ext2TryRemount(
               IN PEXT2_SUPERBLOCK  SuperBlock,
               IN PDEVICE_OBJECT    RealDevice
               )
{
    PVCB Vcb = NULL;
    PLIST_ENTRY Entry = Ext2GlobalData.VcbList.Flink;
    while (Entry != &Ext2GlobalData.VcbList)
    {
        PVCB ContVcb = CONTAINING_RECORD(Entry, VCB, ListEntry);
        ExAcquireResourceExclusive(&ContVcb->MainResource, TRUE);
        if (ContVcb->VcbState == VCB_NOT_MOUNTED)
        {
            if (Ext2EqualSuperBlock(&ContVcb->SuperBlock, SuperBlock) &&
                ContVcb->Vpb->RealDevice == RealDevice)
            {
                Vcb = ContVcb;
                break;
            }
        }
        ExReleaseResourceLite(&ContVcb->MainResource);
        Entry = Entry->Flink;
    }
    return Vcb;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2ScanForDismount
// LOCKS Ext2GlobalData [EXCL]

extern "C"
VOID
Ext2ScanForDismount(
                    )
{
    PVCB DeadVcb = NULL;
    PLIST_ENTRY Entry = Ext2GlobalData.VcbList.Flink;
    while (Entry != &Ext2GlobalData.VcbList)
    {
        PVCB ContVcb = CONTAINING_RECORD(Entry, VCB, ListEntry);
        if (ContVcb->VcbState == VCB_DEAD)
        {
            DeadVcb = ContVcb;
            break;
        }
        Entry = Entry->Flink;
    }
    
    if (DeadVcb)
    {
        // Ext2CheckForDismount needs locked Vcb
        ExAcquireResourceExclusive(&DeadVcb->MainResource, TRUE);
        BOOLEAN VcbResAcquired = TRUE;
        VcbResAcquired = !Ext2CheckForDismount(DeadVcb);
        if (VcbResAcquired)
            ExReleaseResourceLite(&DeadVcb->MainResource);
    }
}

/////////////////////////////////////////////////////////////////////////////