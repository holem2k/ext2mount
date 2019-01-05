#include "stdafx.h"
#include "except.h"
#include "dispatch.h"
#include "fscontrol.h"
#include "create.h"
#include "cleanup.h"
#include "volumeinfo.h"
#include "fileinfo.h"
#include "dirctrl.h"
#include "close.h"
#include "read.h"
#include "versup.h"
#include "devcontrol.h"

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2CommonDispatch(
                   PDEVICE_OBJECT DeviceObject,
                   PIRP Irp
                   )
{
    NTSTATUS status = STATUS_SUCCESS;
    PIRP TopLevelIrp;
    PIRP_CONTEXT IrpContext;

    __try
    {
        FsRtlEnterFileSystem();

        TopLevelIrp = IoGetTopLevelIrp();
        if (TopLevelIrp == NULL)
            IoSetTopLevelIrp(Irp);
            
        status = Ext2CreateIrpContext(DeviceObject, Irp, TopLevelIrp, &IrpContext);
        if (!NT_SUCCESS(status))
        {
            Irp->IoStatus.Status = status = STATUS_INSUFFICIENT_RESOURCES;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            __leave;
        }
        __try
        {
            status = Ext2Dispatch(IrpContext);
        }
        __except(Ext2ExceptionFilter(IrpContext, GetExceptionCode()))
        {
            status = Ext2ExceptionHandler(IrpContext, GetExceptionCode());
        }
    }
    __finally
    {
        IoSetTopLevelIrp(TopLevelIrp);
        FsRtlExitFileSystem();
    }

    BOOLEAN Again = Ext2IsStatusRemovable(IrpContext, status);
    while (Again)
        status = Ext2ProcessRemovableStatus(IrpContext, status, &Again);
    
    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS 
Ext2Dispatch(
             PIRP_CONTEXT IrpContext
             )
{
    NTSTATUS status;
    
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(IrpContext->Irp);
    
    switch (IrpStack->MajorFunction)
    {
    case IRP_MJ_READ:
        status = Ext2Read(IrpContext);
        break;

    case IRP_MJ_DIRECTORY_CONTROL:
        status = Ext2DirectoryControl(IrpContext);
        break;

    case IRP_MJ_CREATE:
        status = Ext2Create(IrpContext);
        break;

    case IRP_MJ_CLEANUP:
        status = Ext2Cleanup(IrpContext);
        break;

    case IRP_MJ_FILE_SYSTEM_CONTROL:
        status = Ext2FileSystemControl(IrpContext);
        break;
        
    case IRP_MJ_QUERY_VOLUME_INFORMATION:
        status = Ext2QueryVolumeInformation(IrpContext);
        break;

    case IRP_MJ_QUERY_INFORMATION:
        status = Ext2QueryInformation(IrpContext);
        break;

    case IRP_MJ_SET_INFORMATION:
        status = Ext2SetInformation(IrpContext);
        break;

    case IRP_MJ_CLOSE:
        status = Ext2Close(IrpContext);
        break;

    case IRP_MJ_DEVICE_CONTROL:
        status = Ext2DeviceControl(IrpContext);
        break;

    default:
        status = Ext2CompleteRequest(IrpContext, STATUS_INVALID_DEVICE_REQUEST);
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////

VOID
Ext2WorkerThreadRoutine(
                        PVOID   Context
                        )
{
    ASSERT(Context);

    PIRP_CONTEXT IrpContext = (PIRP_CONTEXT)Context;

    NTSTATUS status;

    __try
    {
        FsRtlEnterFileSystem();

        IoSetTopLevelIrp(IrpContext->TopLevelIrp ?
            IrpContext->TopLevelIrp : IrpContext->Irp);

        __try
        {
            status = Ext2Dispatch(IrpContext);
        }
        __except(Ext2ExceptionFilter(IrpContext, GetExceptionCode()))
        {
            status = Ext2ExceptionHandler(IrpContext, GetExceptionCode());
        }
    }
    __finally
    {
        IoSetTopLevelIrp(NULL); // doesn't matter (working thread)
        FsRtlExitFileSystem();
    }

    BOOLEAN Again = Ext2IsStatusRemovable(IrpContext, status);
    while (Again)
        status = Ext2ProcessRemovableStatus(IrpContext, status, &Again);

}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2QueueIrp(
             PIRP_CONTEXT IrpContext
             )
{
    ASSERT(IrpContext);

    IoMarkIrpPending(IrpContext->Irp);

    IrpContext->IsSync = TRUE;

    ExInitializeWorkItem(&IrpContext->WorkItem,
        Ext2WorkerThreadRoutine,
        IrpContext);

    ExQueueWorkItem(&IrpContext->WorkItem,
        CriticalWorkQueue);

    return STATUS_PENDING;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2Unload(
           PDRIVER_OBJECT DeviceObject
           )
{
    KdHeader(KD_LEV1, "Ext2Unload");
    
    ExDeleteNPagedLookasideList(&Ext2GlobalData.IrpContextList);
    ExDeleteResourceLite(&Ext2GlobalData.DataResource);
}

/////////////////////////////////////////////////////////////////////////////
