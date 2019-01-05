#include "stdafx.h"
#include "cleanup.h"


/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2Cleanup(
            PIRP_CONTEXT    IrpContext
            )

{
    PAGED_CODE();

    ASSERT(IrpContext);

    KdHeader(KD_LEV1, "Ext2Cleanup");

    PVCB Vcb = Ext2GetVcb(IrpContext);
    if (Vcb == NULL)
    {
        // if device object is file system device object, just return
        return Ext2CompleteRequest(IrpContext, STATUS_SUCCESS);
    }

    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN VcbResAcquired = FALSE;
    BOOLEAN FcbResAcquired = FALSE;
    PFCB Fcb; 

    __try
    {
        ExAcquireResourceExclusiveLite(&Vcb->MainResource, TRUE);
        VcbResAcquired = TRUE;

        switch (Ext2GetFcbType(IrpContext->FileObject))
        {
        case FCB_TYPE:
            {
                SetFlag(IrpContext->FileObject->Flags, FO_CLEANUP_COMPLETE);

                Ext2GetFcb(IrpContext->FileObject, &Fcb);

                ExAcquireResourceExclusiveLite(&Fcb->MainResource, TRUE);
                FcbResAcquired = TRUE;
                
                Fcb->OpenHandleCount--;
                
                PFILE_OBJECT FileObject = IrpContext->FileObject;

                // TO DO
                // unlock byte range locks
                
                PCCB Ccb = Ext2GetCCB(IrpContext);
                FsRtlNotifyCleanup(Vcb->NotifySync,
                    &Vcb->DirNotifyList, Ccb);

                CcUninitializeCacheMap(FileObject, NULL, NULL);
            }
            break;

        case VCB_TYPE:
                // if file object locks volume, unlock it
                if (FlagOn(Vcb->Flags, VCB_LOCKED) && 
                    IrpContext->FileObject == Vcb->VolumeLockFile)
                {
                    UCHAR SavedIrql;
                    IoAcquireVpbSpinLock(&SavedIrql);
                    ClearFlag(Vcb->Vpb->Flags, VPB_LOCKED);
                    IoReleaseVpbSpinLock(SavedIrql);
                    ClearFlag(Vcb->Flags, VCB_LOCKED);
                    Vcb->VolumeLockFile = NULL;
                    FsRtlNotifyVolumeEvent(IrpContext->FileObject, FSRTL_VOLUME_UNLOCK);
                }
                SetFlag(IrpContext->FileObject->Flags, FO_CLEANUP_COMPLETE);

                // fall through
        case MFCB_TYPE:
        case UNK_TYPE:
            // do nothing, complete with success
            break;

        default:
            ASSERT(0);
        }

        
    }
    __finally
    {
        if (FcbResAcquired)
            ExReleaseResourceLite(&Fcb->MainResource);

        if (VcbResAcquired)
            ExReleaseResourceLite(&Vcb->MainResource);

        if (Ext2CanComplete(IrpContext, status))
            status = Ext2CompleteRequest(IrpContext, status);
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////