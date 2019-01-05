#include "stdafx.h"
#include "ext2alg.h"
#include "disk.h"
#include "cachesup.h"

/////////////////////////////////////////////////////////////////////////////
// Ext2FileBlock(...)
// Expected IRQL: PASSIVE, APC
// Section: paged.
// Cause page errors: YES_IF(Inode, DevBlock), internal

extern "C"
NTSTATUS
Ext2FileBlock(
              IN PVCB           Vcb,
              IN PEXT2_INODE    Inode,
              IN ULONG          Block,
              IN OUT PULONG     DevBlock
              )
{
    KdHeader(KD_LEV2, "Ext2FileBlock");

    NTSTATUS status = STATUS_UNSUCCESSFUL;
    if (Block >= ALIGN(Inode->Size, Vcb->SuperBlock.BlockSize, ULONGLONG) >> Vcb->SuperBlock.BlockSizeShift)
    {
        return status;
    }

    if (Block < EXT2_NDIR_BLOCKS)
    {
        *DevBlock = Inode->Block[Block];
        status = STATUS_SUCCESS;
    }
    else
    {
        PEXT2_SUPERBLOCK SuperBlock = &Vcb->SuperBlock;
        ULONG PointerPerBlock = SuperBlock->BlockSize / sizeof(Inode->Block[0]);
        ULONG RelativeBlock = Block - EXT2_NDIR_BLOCKS;
        
        ULONG Ind = 0;
        ULONG PSize = PointerPerBlock;
        while (RelativeBlock >= PSize)
        {
            RelativeBlock -= PSize;
            Ind++;
            PSize *= PointerPerBlock;
        }
        
        ASSERT(Ind >= 0 && Ind <= 2);
        
        ULONG PointerNum = Inode->Block[EXT2_NDIR_BLOCKS + Ind];
        
        do
        {
            CACHED_BLOCK CachedBlock;
            status = Ext2GetCachedBlock(Vcb, PointerNum, &CachedBlock);
            
            if (!NT_SUCCESS(status))
                break;
            
            if (Ind > 0)
            {
                PSize /= PointerPerBlock;
                ULONG N = RelativeBlock / PSize;
                PointerNum = ((ULONG *)CachedBlock.Data)[N];
                RelativeBlock = RelativeBlock % PSize;
            }
            else
            {
                *DevBlock = ((ULONG *)CachedBlock.Data)[RelativeBlock];
                status = STATUS_SUCCESS; // ?
            }
            
            Ext2ReleaseCachedBlock(&CachedBlock);
        }
        while (Ind-- > 0);
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2InitNameCache()
// Expected IRQL: PASSIVE, APC
// Section: paged.
// Cause page errors: YES_IF(Cache)

extern "C"
VOID
Ext2InitNameCache(
                  IN PNAME_CACHE Cache
                  )
{
    KdHeader(KD_LEV2, "Ext2InitNameCache");

    for (ULONG i = 0; i < NAME_CACHE_SIZE; i++)
        Cache->Name[i] = NULL;
    Cache->Next = 0;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2UninitNameCache()
// Expected IRQL: PASSIVE, APC
// Section: paged.
// Cause page errors: YES_IF(Cache)

extern "C"
VOID
Ext2UninitNameCache(
                    IN PNAME_CACHE Cache
                    )
{
    KdHeader(KD_LEV2, "Ext2UninitNameCache");

    for (ULONG i = 0; i < NAME_CACHE_SIZE; i++)
    {
        if (Cache->Name[i])
            ExFreePool(Cache->Name[i]);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Ext2LookupNameCache()
// Expected IRQL: PASSIVE, APC
// Section: paged.
// Cause page errors: YES_IF(Cache, Name)

extern "C"
ULONG
Ext2LookupNameCache(
                    IN PNAME_CACHE      Cache,
                    IN PCWSTR           Name,
                    IN ULONG            Length
                    )
{
    ASSERT(Name);

    KdHeader(KD_LEV2, "Ext2LookupNameCache");

    ULONG Inode = 0; // not found

    for (ULONG i = 0; i < NAME_CACHE_SIZE; i++)
    {
        ULONG MemLength = Cache->MemLength[i];
        if (Cache->Name[i] && Length * sizeof(WCHAR) == MemLength)
        {
            if (RtlCompareMemory(Name, Cache->Name[i], MemLength) == MemLength)
            {
                Inode = Cache->Inode[i];
                break;
            }
        }
    }
    return Inode;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2PutInNameCache()
// Expected IRQL: PASSIVE, APC
// Section: paged.
// Cause page errors: YES_IF(Cache, Name)

extern "C"
VOID
Ext2PutInNameCache(
                   IN PNAME_CACHE      Cache,
                   IN PCWSTR           Name,
                   IN ULONG            Length,
                   IN ULONG            Inode
                   )
{
    ASSERT(Name);

    KdHeader(KD_LEV2, "Ext2PutInNameCache");

    ULONG Next = Cache->Next;

    if (Cache->Name[Next])
        ExFreePool(Cache->Name[Next]);

    Length *= sizeof(WCHAR);

    Cache->Name[Next] = (PWSTR)ExAllocatePoolWithTag(PagedPool,
        Length, EXT2_CACHE_TAG);

    if (Cache->Name[Next])
    {
        RtlCopyMemory(Cache->Name[Next], Name, Length);
        Cache->Inode[Next] = Inode;
        Cache->MemLength[Next] = Length;
        Cache->Next = (Next + 1) % NAME_CACHE_SIZE;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Ext2ParseDirAndAllocateOffset()
// Expected IRQL: PASSIVE, APC
// Section: paged.
// Cause page errors: YES_IF(DirData, Count, Offset), internal

extern "C"
NTSTATUS
Ext2ParseDirAndAllocateOffset(
                              IN PVOID              DirData,
                              IN ULONG              Size,
                              IN PEXT2_SUPERBLOCK   SuperBlock,
                              IN BOOLEAN            IncludeDots,
                              IN OUT PULONG         Count,
                              IN OUT PULONG         *Offset
                              )
{
    PAGED_CODE();

    ASSERT(DirData);
    ASSERT(Offset);

    KdHeader(KD_LEV2, "Ext2ParseDirAndAllocateOffset");

    // count inodes
    ULONG InodeTotalCount = 0;
    ULONG EntryOffset = 0;
    while (EntryOffset < Size)
    {
        InodeTotalCount++;
        EntryOffset += GET_ENTRY(DirData, EntryOffset)->rec_len;
    }

    // allocate memory for offsets
    PULONG Offs = (PULONG)ExAllocatePoolWithTag(PagedPool,
        sizeof(ULONG) * InodeTotalCount, EXT2_OFFS_TAG);

    if (Offs == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;

    NTSTATUS status = STATUS_SUCCESS;

    // validate dir entries and calc offsets
    PEXT2_DIR_ENTRY_REAL Entry = GET_ENTRY(DirData, 0);
    ULONG InodesCount = SuperBlock->InodesCount;
    ULONG Mask = ~(SuperBlock->BlockSize - 1);
    ULONG InodeCount = 0;
    EntryOffset = 0;

    for (ULONG i = 0; i < InodeTotalCount; i++)
    {
        // validate entry
        if ((EntryOffset & Mask) != ((EntryOffset + Entry->rec_len - 1) & Mask) ||
            Entry->rec_len & 3 ||
            Entry->rec_len < EXT2_DIR_REC_LEN(1) || 
            Entry->rec_len < EXT2_DIR_REC_LEN(Entry->name_len) || 
            Entry->inode > InodesCount ||
            Entry->name_len > EXT2_NAME_LEN
            )
        {
            KdPrint(("[ext2.sys] - invalid directory entry\n"));
            status = STATUS_NOT_A_DIRECTORY;
            break;
        }
        
        if ((i >= 2 || IncludeDots) && Entry->inode)
            Offs[InodeCount++] = EntryOffset;

        EntryOffset += Entry->rec_len;
        Entry = GET_ENTRY(DirData, EntryOffset);
    }

    if (NT_SUCCESS(status))
    {
        *Offset = Offs;
        *Count = InodeCount;
    }
    else
        ExFreePool(Offs);

    return status;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2FreeOffset()
// Expected IRQL: PASSIVE, APC
// Section: paged.
// Cause page errors: NO

extern "C"
VOID
Ext2FreeOffset(
               PULONG   Offset
               )
{
    PAGED_CODE();

    ASSERT(Offset);

    KdHeader(KD_LEV2, "Ext2FreeOffset");

    ExFreePool(Offset);
}

/////////////////////////////////////////////////////////////////////////////
// Ext2ReadDirCached(...)
// Expected IRQL: PASSIVE
// Section: paged.
// Cause page errors: YES_IF(Vcb, Inode, Buffer), internal 

extern "C"
NTSTATUS
Ext2ReadDirCached(
                   IN PVCB            Vcb,
                   IN PEXT2_INODE     Inode,
                   IN ULONG           Size,
                   IN OUT PVOID       Buffer
                   )
{
    PAGED_CODE();

    ASSERT(Inode);
    ASSERT(Buffer);
    ASSERT((Size & (Vcb->SuperBlock.BlockSize - 1)) == 0);

    KdHeader(KD_LEV1, "Ext2ReadDirCached");

    NTSTATUS status;
    PEXT2_SUPERBLOCK SuperBlock = &Vcb->SuperBlock;
    ULONG BlockSizeShift = SuperBlock->BlockSizeShift;
    ULONG BlockSize = SuperBlock->BlockSize;
    ULONG LastBlock = (Size - 1) >> BlockSizeShift; 

    for (ULONG i = 0; i <= LastBlock; i++)
    {
        ULONG DeviceBlock;
        status = Ext2FileBlock(Vcb, Inode, i, &DeviceBlock);
        if (!NT_SUCCESS(status))
            break;

        CACHED_BLOCK CachedBlock;
        status = Ext2GetCachedBlock(Vcb, DeviceBlock, &CachedBlock);
        if (!NT_SUCCESS(status))
            break;

        // BAD, but enough fast
        RtlCopyMemory(BUFFER(Buffer, i << BlockSizeShift), 
            CachedBlock.Data, BlockSize);

        Ext2ReleaseCachedBlock(&CachedBlock);
    }

    return status;
}

/////////////////////////////////////////////////////////////////////////////
