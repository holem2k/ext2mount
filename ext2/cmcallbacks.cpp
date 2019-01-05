#include "stdafx.h"
#include "cmcallbacks.h"

/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOLEAN 
Ext2AcquireForLazyWrite(
                        IN  PVOID   Context,
                        IN  BOOLEAN Wait
                        )
{
    PAGED_CODE();

    KdHeader(KD_LEV1, "acquire for write");

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2ReleaseFromLazyWrite(
                         IN PVOID Context
                         )
{
    PAGED_CODE();

    KdHeader(KD_LEV1, "release from write");
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOLEAN 
Ext2AcquireForReadAhead(
                        IN  PVOID   Context,
                        IN  BOOLEAN Wait
                        )
{
    PAGED_CODE();

    PFCB Fcb = (PFCB)Context;
    ASSERT(Fcb);
    ASSERT(Fcb->NodeTypeCode == FCB_TYPE);

    KdHeader(KD_LEV1, "acquire for read-ahead");

    return ExAcquireResourceSharedLite(&Fcb->MainResource, Wait);
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2ReleaseFromReadAhead(
                         IN PVOID Context
                         )
{
    PAGED_CODE();

    PFCB Fcb = (PFCB)Context;
    ASSERT(Fcb);
    ASSERT(Fcb->NodeTypeCode == FCB_TYPE);

    KdHeader(KD_LEV1, "release from read-ahead");

    ExReleaseResourceLite(&Fcb->MainResource);
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOLEAN 
Ext2NoopAcquire(
                IN  PVOID   Context,
                IN  BOOLEAN Wait
                )
{
    PAGED_CODE();

    KdHeader(KD_LEV1, "noop acquire for write");

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2NoopRelease(
                IN PVOID Context
                )
{
    PAGED_CODE();

    KdHeader(KD_LEV1, "noop release from write");
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOLEAN 
Ext2AcquireForCache(
                    IN  PVOID   Context,
                    IN  BOOLEAN Wait
                    )
{
    PAGED_CODE();

    KdHeader(KD_LEV1, "acquire for cache");

    PMFCB Fcb = (PMFCB)Context;
    ASSERT(Fcb->NodeTypeCode == MFCB_TYPE);

    return ExAcquireResourceSharedLite(&Fcb->MainResource, Wait);
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2ReleaseFromCache(
                     IN PVOID Context
                     )
{
    PAGED_CODE();

    KdHeader(KD_LEV1, "release from read-ahead");

    PMFCB Fcb = (PMFCB)Context;
    ASSERT(Fcb->NodeTypeCode == MFCB_TYPE);

    ExReleaseResourceLite(&Fcb->MainResource);
}

/////////////////////////////////////////////////////////////////////////////
