#include "stdafx.h"
#include "cachesup.h"

/////////////////////////////////////////////////////////////////////////////

extern "C"
PFILE_OBJECT
Ext2CreateMetaStream(
                     PVPB           Vpb,
                     ULONGLONG      Size
                     )
{
    PMFCB Fcb = Ext2CreateMFCB(Size);
    if (Fcb == NULL)
        return NULL;

    PFILE_OBJECT FileObject = IoCreateStreamFileObject(NULL, Vpb->RealDevice);
    if (FileObject == NULL)
    {
        Ext2FreeMFCB(Fcb);
        return NULL;
    }
    FileObject->FsContext = Fcb;
    FileObject->ReadAccess = TRUE;
    FileObject->WriteAccess = FALSE;
    FileObject->DeleteAccess = FALSE;
    FileObject->Vpb = Vpb;
    FileObject->SectionObjectPointer = &Fcb->SectionObject;

    KdPrint(("MetaFile %X", FileObject));

#ifdef _DEBUG
#if 0
    // !!!!!!!! it will crush system on volume dismount !!!!!!!
    RtlInitUnicodeString(&FileObject->FileName, L"Ext2Stream");
#endif
#endif

    CcInitializeCacheMap(FileObject, (PCC_FILE_SIZES)&Fcb->AllocationSize, TRUE,
        &Ext2GlobalData.MetaCacheCallbacks, Fcb);

    return FileObject;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2DeleteMetaStream(
                     PFILE_OBJECT   FileObject
                     )
{
    PMETASTREAM_DATA Data = (PMETASTREAM_DATA)ExAllocatePool(NonPagedPool,
        sizeof(METASTREAM_DATA));
    if (Data)
    {
        Data->FileObject = FileObject;
        ExInitializeWorkItem(&Data->WorkItem,
            Ext2DeleteStreamWorkerRoutine, Data);
        ExQueueWorkItem(&Data->WorkItem, CriticalWorkQueue);
    }
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2DeleteStreamWorkerRoutine(
                              PVOID Context
                              )
{
    ASSERT(Context);

    KdHeader(KD_LEV1, "Ext2DeleteStreamWorkerRoutine");

    PMETASTREAM_DATA Data = (PMETASTREAM_DATA)Context;

    __try
    {
        PFILE_OBJECT FileObject = Data->FileObject;

        PMFCB Fcb;
        Ext2GetFcb(FileObject, &Fcb);

        ExAcquireResourceExclusive(&Fcb->MainResource, TRUE);
        CcPurgeCacheSection(FileObject->SectionObjectPointer,
            NULL, 0, FALSE);
        CcUninitializeCacheMap(FileObject, NULL, NULL);
        ExReleaseResource(&Fcb->MainResource);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    ObDereferenceObject(Data->FileObject);

    ExFreePool(Data);

    KdHeader(KD_LEV1, "Ext2DeleteStreamWorkerRoutine end");
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2GetCachedBlock(
                   IN PVCB              Vcb,
                   IN ULONG             Block,
                   OUT PCACHED_BLOCK    CachedBlock
                   )
{
    ULONGLONG Offset = (ULONGLONG)Block << Vcb->SuperBlock.BlockSizeShift;
    NTSTATUS status = STATUS_SUCCESS;

    __try
    {
        CcMapData(Vcb->MetaStreamFile,
            (PLARGE_INTEGER)&Offset,
            Vcb->SuperBlock.BlockSize,
            TRUE,
            &CachedBlock->Bcb,
            &CachedBlock->Data);
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
Ext2ReleaseCachedBlock(
                       IN PCACHED_BLOCK  CachedBlock
                       )
{
    __try
    {
        CcUnpinData(CachedBlock->Bcb);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        
    }
}

/////////////////////////////////////////////////////////////////////////////
// Ext2PurgeVolume

extern "C"
NTSTATUS
Ext2PurgeVolume(
                IN PVCB Vcb
                )
{
    // TO DO
    return STATUS_UNSUCCESSFUL;
}

/////////////////////////////////////////////////////////////////////////////

