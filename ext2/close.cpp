#include <stdafx.h>
#include "close.h"

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2Close(
          PIRP_CONTEXT    IrpContext
          )
{
    PAGED_CODE();

    ASSERT(IrpContext);

    KdHeader(KD_LEV1, "Ext2Close");

    PVCB Vcb = Ext2GetVcb(IrpContext);
    if (Vcb == NULL)
    {
        // file system device object close
        return Ext2CompleteRequest(IrpContext, STATUS_SUCCESS);
    }

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
                Ext2GetFcb(IrpContext->FileObject, &Fcb);

                ExAcquireResourceExclusiveLite(&Fcb->MainResource, TRUE);
                FcbResAcquired = TRUE;

                KdPrint(("Close: Inode = %x, Handle = %x, Ref = %x\n", Fcb->InodeNum, Fcb->OpenHandleCount, Fcb->ReferenceCount));
#ifdef _DEBUG
                KdPrint(("Name = %ws\n", Fcb->Name.Buffer));
                KdPrint(("FName = %ws", IrpContext->FileObject->FileName.Buffer));
                if (IrpContext->FileObject->RelatedFileObject)
                {
                    //PFCB RFcb;
                    //Ext2GetFcb(IrpContext->FileObject->RelatedFileObject, &RFcb);
                    KdPrint(("*\n"));
                }
                else
                    KdPrint(("\n"));
#endif

                Fcb->ReferenceCount--;
                Vcb->ReferenceCount--;
                PCCB Ccb = Ext2GetCCB(IrpContext);
                if (Ccb != NULL)
                    Ext2FreeCCB(Ccb);
                if (Fcb->ReferenceCount == 0)
                {
                    KdPrint(("Kill Fcb %x\n", Fcb->InodeNum));
                    ExReleaseResourceLite(&Fcb->MainResource);
                    FcbResAcquired = FALSE;
                    Ext2RemoveFCB(Vcb, Fcb);
                    Ext2FreeFCB(Fcb);
                }
            }
            break;
            
        case VCB_TYPE:
            {
                Vcb->ReferenceCount--;
            }
            break;

        case MFCB_TYPE:
            {
                KdInfo("close meta stream");

                PMFCB Fcb;
                Ext2GetFcb(IrpContext->FileObject, &Fcb);
                Ext2FreeMFCB(Fcb);
                Vcb->ReferenceCount--;
            }
            break;

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

        VcbResAcquired = !Ext2CheckForDismount(Vcb);

        if (VcbResAcquired)
            ExReleaseResourceLite(&Vcb->MainResource);

        if (!IrpContext->ExceptionInProgress)
            Ext2CompleteRequest(IrpContext, STATUS_SUCCESS);
    }
    return STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
