#include "stdafx.h"


/////////////////////////////////////////////////////////////////////////////

NTSTATUS
Ext2ExceptionHandler(
                     PIRP_CONTEXT   IrpContext,
                     NTSTATUS       Status
                     )
{
    IrpContext->Irp->IoStatus.Status = Status;
    IrpContext->Irp->IoStatus.Information = 0;
    IoCompleteRequest(IrpContext->Irp, IO_NO_INCREMENT);
    Ext2DestroyIrpContext(IrpContext);
    return Status;
}

/////////////////////////////////////////////////////////////////////////////

int
Ext2ExceptionFilter(
                    PIRP_CONTEXT    IrpContext,
                    NTSTATUS        Status
                    )
{
    if (FsRtlIsNtstatusExpected(Status))
    {
        IrpContext->ExceptionInProgress = TRUE;
        return EXCEPTION_EXECUTE_HANDLER;
    }
    else
        return EXCEPTION_CONTINUE_SEARCH; // bsod
}


/////////////////////////////////////////////////////////////////////////////