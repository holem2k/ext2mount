#include "stdafx.h"
#include "versup.h"
#include "disk.h"
#include "dispatch.h"

/////////////////////////////////////////////////////////////////////////////

extern "C"
inline
BOOLEAN 
Ext2IsStatusRemovable(
                      PIRP_CONTEXT  IrpContext,
                      NTSTATUS      Status
                      )
{
    if (Status == STATUS_SUCCESS)
        return FALSE;

    return IoIsErrorUserInduced(Status) &&
        !(IrpContext->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL &&
        (IrpContext->MinorFunction == IRP_MN_VERIFY_VOLUME || IrpContext->MinorFunction == IRP_MN_MOUNT_VOLUME));
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2ProcessRemovableStatus(
                           IN PIRP_CONTEXT IrpContext,
                           IN NTSTATUS     Status,
                           IN OUT PBOOLEAN Again
                           )
{
    ASSERT(IrpContext);
    ASSERT(Ext2IsStatusRemovable(IrpContext, Status));
    
    *Again = FALSE;
    PIRP Irp = IrpContext->Irp;

    if (IrpContext->TopLevelIrp != NULL)
        return Ext2CompleteRequest(IrpContext, Status);

    // get real device
    PDEVICE_OBJECT Device = IoGetDeviceToVerify(IrpContext->Irp->Tail.Overlay.Thread);
    IoSetDeviceToVerify(IrpContext->Irp->Tail.Overlay.Thread, NULL);
    if (Device == NULL)
    {
        Device = IoGetDeviceToVerify(PsGetCurrentThread());
        IoSetDeviceToVerify(PsGetCurrentThread(), NULL);
        if (Device == NULL)
            return Ext2CompleteRequest(IrpContext, STATUS_DRIVER_INTERNAL_ERROR);
    }

    if (Status == STATUS_VERIFY_REQUIRED)
    {
        // currently RawMount is always FALSE
        Status = IoVerifyVolume(Device, FALSE);

        //PVCB Vcb = Ext2GetVcb(IrpContext);
        //if (Status == STATUS_WRONG_VOLUME && Vcb->VcbState == VCB_MOUNTED)
            //Status = STATUS_SUCCESS;

        if (IrpContext->MajorFunction == IRP_MJ_CREATE && 
            IrpContext->FileObject->RelatedFileObject == NULL)
        {
            return Ext2CompleteRequest(IrpContext, STATUS_REPARSE, IO_REMOUNT);
        }

        if (IoIsErrorUserInduced(Status))
        {
            IoSetHardErrorOrVerifyDevice(IrpContext->Irp, Device);
            *Again = TRUE;
            return Status;
        }
        else if (!NT_SUCCESS(Status))
            Ext2CompleteRequest(IrpContext, Status);
        else
        {
            Ext2QueueIrp(IrpContext);
            return STATUS_PENDING;
        }
    }

    // process user unduced errors
    //

    if (!IrpContext->PopupOnError)
        return Ext2CompleteRequest(IrpContext, Status);
    
    PVPB Vpb = IrpContext->FileObject ? IrpContext->FileObject->Vpb : NULL;
    
    // Irp will be completed in context of another thread
    // so mark it pending...
    IoMarkIrpPending(Irp);
    
    // make popup
    Irp->IoStatus.Status = Status; 
    IoRaiseHardError(Irp, Vpb, Device);
    
    Ext2DestroyIrpContext(IrpContext);
    
    return STATUS_PENDING;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2VerifyFcbOperation

extern "C"
NTSTATUS
Ext2VerifyFcbOperation(
                       PIRP_CONTEXT  IrpContext
                       )
                       
{
    PVCB Vcb = Ext2GetVcb(IrpContext);

    if (Vcb->VcbState == VCB_DISMOUNTED || Vcb->VcbState == VCB_DEAD)
        return STATUS_FILE_INVALID;

    PDEVICE_OBJECT RealDevice = Vcb->Vpb->RealDevice;

    if (FlagOn(RealDevice->Flags, DO_VERIFY_VOLUME))
    {
        IoSetHardErrorOrVerifyDevice(IrpContext->Irp, RealDevice);
        return STATUS_VERIFY_REQUIRED;
    }

    if (Vcb->VcbState == VCB_NOT_MOUNTED)
    {
        IoSetHardErrorOrVerifyDevice(IrpContext->Irp, RealDevice);
        return STATUS_WRONG_VOLUME;
    }

    return STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2VerifyVcb(...)
// Locks: Vcb[SHARED]

extern "C"
NTSTATUS
Ext2VerifyVcb(
              PIRP_CONTEXT  IrpContext,
              PVCB          Vcb
              )
{
    PDEVICE_OBJECT RealDevice = Vcb->Vpb->RealDevice;

    if (FlagOn(RealDevice->Characteristics, FILE_REMOVABLE_MEDIA))
    {
        ULONG ChangeCount;
        NTSTATUS status = Ext2PingDevice(Vcb->TargetDevice, FALSE, &ChangeCount);
        if (status == STATUS_VERIFY_REQUIRED)
            return status;

        if (!NT_SUCCESS(status) && 
            (Vcb->VcbState == VCB_MOUNTED || Vcb->VcbState == VCB_NOT_MOUNTED))
        {
            return status;
        }

        if (Vcb->VcbState == VCB_NOT_MOUNTED)
        {
            IoSetHardErrorOrVerifyDevice(IrpContext->Irp, RealDevice);
            return STATUS_WRONG_VOLUME;
        }
    }

    if (FlagOn(RealDevice->Flags, DO_VERIFY_VOLUME))
    {
        IoSetHardErrorOrVerifyDevice(IrpContext->Irp, RealDevice);
        return STATUS_VERIFY_REQUIRED;
    }

    if (Vcb->VcbState == VCB_DISMOUNTED || Vcb->VcbState == VCB_DEAD)
        return STATUS_FILE_INVALID;

    return STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
