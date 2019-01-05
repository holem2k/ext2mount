#ifndef __CACHESUP_H__
#define __CACHESUP_H__

typedef struct CACHED_BLOCK
{
    PVOID Data;
    PVOID  Bcb;
} CACHED_BLOCK, *PCACHED_BLOCK;

typedef struct METASTREAM_DATA
{
    WORK_QUEUE_ITEM      WorkItem;
    PFILE_OBJECT         FileObject;
} METASTREAM_DATA, *PMETASTREAM_DATA;

extern "C"
VOID
Ext2DeleteStreamWorkerRoutine(
                              PVOID Context
                              );

extern "C"
PFILE_OBJECT
Ext2CreateMetaStream(
                     PVPB           Vpb,
                     ULONGLONG      Size
                     );

extern "C"
VOID
Ext2DeleteMetaStream(
                     PFILE_OBJECT   FileObject
                     );

extern "C"
NTSTATUS
Ext2GetCachedBlock(
                   IN PVCB              Vcb,
                   IN ULONG             Block,
                   OUT PCACHED_BLOCK    CachedBlock
                   );

extern "C"
VOID
Ext2ReleaseCachedBlock(
                       IN PCACHED_BLOCK  CachedBlock
                       );

#endif //__CACHESUP_H__