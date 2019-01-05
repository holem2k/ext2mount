#include "stdafx.h"
#include "devcontrol.h"

/////////////////////////////////////////////////////////////////////////////
// Ext2DeviceControlCompletionRoutine
// Section: nonpaged

extern "C"
NTSTATUS
Ext2DeviceControlCompletionRoutine(
                                   IN PDEVICE_OBJECT    DeviceObject,
                                   IN PIRP              Irp,
                                   IN PVOID             Context
                                   )
{
    if (Irp->PendingReturned)
        IoMarkIrpPending(Irp);

    return STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2DeviceControl(
                  PIRP_CONTEXT IrpContext
                  )
{
    PAGED_CODE();

    ASSERT(IrpContext);

    KdHeader(KD_LEV1, "Ext2DeviceControl");

    if (Ext2GetFcbType(IrpContext->FileObject) != VCB_TYPE)
        return Ext2CompleteRequest(IrpContext, STATUS_INVALID_PARAMETER);

    PIRP Irp = IrpContext->Irp;
    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp, Ext2DeviceControlCompletionRoutine,
        NULL, TRUE, TRUE, TRUE);

    PVCB Vcb = Ext2GetVcb(IrpContext);
    NTSTATUS status = IoCallDriver(Vcb->TargetDevice, Irp);

    Ext2DestroyIrpContext(IrpContext);

    return status;
}

/////////////////////////////////////////////////////////////////////////////
