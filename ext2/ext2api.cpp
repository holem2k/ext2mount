#include "stdafx.h"
#include "ext2api.h"
#include "disk.h"
#include "ext2alg.h"
#include "cachesup.h"
#include "helpers.h"

/////////////////////////////////////////////////////////////////////////////
// Ext2ReadSuperBlock(...)
// Expected IRQL: PASSIVE;
// Section: paged.
// Cause page errors: YES_IF(SuperBlock), internal paged allocation

extern "C"
NTSTATUS
Ext2ReadSuperBlock(
				   IN PDEVICE_OBJECT            DeviceObject,
				   IN OUT PEXT2_SUPERBLOCK      SuperBlock
				   )
{
    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(SuperBlock);

    KdHeader(KD_LEV1, "Ext2ReadSuperBlock");

    NTSTATUS status = STATUS_SUCCESS;
    PEXT2_SUPERBLOCK_REAL RealSuperBlock = NULL;

    __try
    {
        RealSuperBlock = (PEXT2_SUPERBLOCK_REAL)ExAllocatePool(PagedPool,
            sizeof(EXT2_SUPERBLOCK_REAL));

        if (RealSuperBlock == NULL)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        status = Ext2ReadDevice(DeviceObject,
            SUPERBLOCK_OFFSET,
            sizeof(EXT2_SUPERBLOCK_REAL),
            TRUE,
            RealSuperBlock);

        if (!NT_SUCCESS(status))
            __leave;

        Ext2ConvertSuperBlock(SuperBlock, RealSuperBlock);
    }
    __finally
    {
        if (RealSuperBlock)
            ExFreePool(RealSuperBlock);
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2CheckSuperBlock(...)
// Expected IRQL: PASSIVE
// Section: paged.
// Cause page errors: YES_IF(SuperBlock)

extern "C"
NTSTATUS
Ext2CheckSuperBlock(
                    IN PEXT2_SUPERBLOCK      SuperBlock
                    )
{
    ASSERT(SuperBlock);

    PAGED_CODE();

    NTSTATUS status = STATUS_UNRECOGNIZED_VOLUME;;

    if (SuperBlock->Magic != EXT2_SUPER_MAGIC ||
        SuperBlock->InodeSize != EXT2_GOOD_OLD_INODE_SIZE ||
        SuperBlock->BlocksPerGroup > SuperBlock->BlockSize*8 ||
		SuperBlock->InodesPerGroup > SuperBlock->BlockSize*8
        )
    {
        return status;
    }

    if (SuperBlock->FeatureIncompat & ~EXT2_FEATURE_INCOMPAT_SUPP)
    {
        KdInfo("Unsupported filesystem features");
        return status;
    }

    // TO DO ?
    // check file system state

    return STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2AllocateAndReadGroupDesc(...)
// Expected IRQL: PASSIVE
// Section: paged.
// Cause page errors: YES_IF(SuperBlock), internal

extern "C"
NTSTATUS
Ext2AllocateAndReadGroupDesc(
                             IN PDEVICE_OBJECT      DeviceObject,
                             IN PEXT2_SUPERBLOCK    SuperBlock,
                             OUT PULONG             DescCountRes,
                             OUT PEXT2_GROUP_DESC   *GroupDescRes
                             )
{
    PAGED_CODE();

    ASSERT(DescCountRes);
    ASSERT(GroupDescRes);

    NTSTATUS status = STATUS_SUCCESS;
    PEXT2_GROUP_DESC GroupDesc = NULL;
    PEXT2_GROUP_DESC_REAL RealGroupDesc = NULL;

    __try
    {
        ULONG DescCount = (SuperBlock->BlocksCount - SuperBlock->FirstDataBlock +
            SuperBlock->BlocksPerGroup - 1)/SuperBlock->BlocksPerGroup;

        ULONG RealDescPerBlock = SuperBlock->BlockSize/sizeof(EXT2_GROUP_DESC_REAL);
        ULONG RealBlockCount = (DescCount - 1)/RealDescPerBlock + 1;

        RealGroupDesc = (PEXT2_GROUP_DESC_REAL)ExAllocatePool(PagedPool,
            RealBlockCount * SuperBlock->BlockSize);

        if (RealGroupDesc ==  NULL)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        ULONG StartBlock = 1 + SuperBlock->FirstDataBlock;

        status = Ext2ReadBlock(DeviceObject, SuperBlock,
            StartBlock, RealBlockCount, RealGroupDesc);

        if (!NT_SUCCESS(status))
            __leave;

        GroupDesc = (PEXT2_GROUP_DESC)ExAllocatePool(NonPagedPool,
            sizeof(EXT2_GROUP_DESC) * DescCount);

        if (GroupDesc == NULL)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        for (ULONG i = 0; i < DescCount; i++)
            Ext2ConvertGroupDesc(GroupDesc + i, RealGroupDesc + i);

        *DescCountRes = DescCount;
        *GroupDescRes = GroupDesc;
    }
    __finally
    {
        if (RealGroupDesc)
            ExFreePool(RealGroupDesc);

        if (NT_ERROR(status) && GroupDesc)
            ExFreePool(GroupDesc);
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2FreeGroupDesc(...)
// Expected IRQL: PASSIVE
// Section: paged.
// Cause page errors: NO

extern "C"
VOID
Ext2FreeGroupDesc(
                  IN PEXT2_GROUP_DESC GroupDesc
                  )
{
    PAGED_CODE();

    ExFreePool(GroupDesc);
}

/////////////////////////////////////////////////////////////////////////////
// Ext2ReadInode(...)
// Expected IRQL: PASSIVE, APC
// Section: paged.
// Cause page errors: YES_IF(Inode), internal

extern "C"
NTSTATUS
Ext2ReadInode(
              IN PVCB            Vcb,
              IN ULONG           InodeNum,
              IN OUT PEXT2_INODE Inode
              )
{
    PAGED_CODE();

    ASSERT(Inode);

    KdHeader(KD_LEV1, "Ext2ReadInode");

    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PEXT2_SUPERBLOCK SuperBlock = &Vcb->SuperBlock;

    if ((InodeNum > SuperBlock->InodesCount ||
        InodeNum < SuperBlock->FirstInode) && 
        InodeNum != EXT2_ROOT_INO)
    {
        KdPrint(("[ext2.sys] - invalid inode number\n"));
        return status;
    }
    
    ULONG Group = (InodeNum - 1)/SuperBlock->InodesPerGroup;
    if (Group >= Vcb->GroupCount)
    {
        KdPrint(("[ext2.sys] - invalid inode group number\n"));
        return status;
    }

    ULONG InodeInGroup = (InodeNum - 1)%SuperBlock->InodesPerGroup;
    ULONG InodePerBlock = SuperBlock->BlockSize/SuperBlock->InodeSize;
    ULONG InodeBlock = Vcb->GroupDesc[Group].InodeTable +
        InodeInGroup/InodePerBlock;

    CACHED_BLOCK CachedBlock;
    status = Ext2GetCachedBlock(Vcb, InodeBlock, &CachedBlock);
    if (NT_SUCCESS(status))
    {
            ULONG InodeInBlock = InodeInGroup % InodePerBlock;
            Ext2ConvertInode(Inode, (PEXT2_INODE_REAL)CachedBlock.Data + InodeInBlock);
            Ext2ReleaseCachedBlock(&CachedBlock);
            status = STATUS_SUCCESS; // ?
    }

    return status;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2ReadFile(...)
// Expected IRQL: PASSIVE, APC;
// Section: paged.
// Cause page errors: YES_IF(Vcb, Inode, Buffer), internal 

extern "C"
NTSTATUS
Ext2ReadFile(
             IN PIRP_CONTEXT    IrpContext,
             IN PVCB            Vcb,
             IN PEXT2_INODE     Inode,
             IN ULONGLONG       Offset,
             IN ULONG           Length,
             IN OUT PVOID       Buffer
             )
{
    PAGED_CODE();

    ASSERT(Inode);
    ASSERT(Buffer);

    KdHeader(KD_LEV1, "Ext2ReadFile");

    PEXT2_SUPERBLOCK SuperBlock = &Vcb->SuperBlock;

    PVOID Block = ExAllocateFromPagedLookasideList(&Vcb->FileBlockPLAList);

    if (Block == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;

    NTSTATUS status;
    ULONG BlockSizeShift = SuperBlock->BlockSizeShift;
    ULONG BlockSize = SuperBlock->BlockSize;
    ULONG StartBlock = (ULONG)(Offset >> BlockSizeShift);
    ULONG EndBlock = (ULONG)((Offset + Length - 1) >> BlockSizeShift);
    ULONG Delta = (ULONG)(BlockSize - Offset % BlockSize);
    ULONG BlockGroup[MAX_PARALLEL_READ];
    ULONG BlockGroupCount = 0;
    ULONG BlockGroupOffs;
    BOOLEAN ForceReadParallel = FALSE;

    for (ULONG i = StartBlock; i <= EndBlock; i++)
    {
        ULONG DeviceBlock;
        status = Ext2FileBlock(Vcb, Inode, i, &DeviceBlock);
        if (!NT_SUCCESS(status))
            break;

        if (i == StartBlock || i == EndBlock)
        {
            if (DeviceBlock != 0)
            {
                status = Ext2ReadBlock(Vcb->TargetDevice, SuperBlock, 
                    DeviceBlock, 1, Block);
                if (!NT_SUCCESS(status))
                    break;
            }

            ULONG BlockOffset, BlockLength, BufferOffset;
            if (i == StartBlock)
            {
                BufferOffset = 0;
                BlockOffset = BlockSize - Delta;
                BlockLength = Length > Delta ? Delta : Length;
            }
            else
            {
                BlockOffset = 0;
                BufferOffset = Delta + ((EndBlock - StartBlock - 1) << BlockSizeShift);
                BlockLength = (ULONG)((Offset + Length) - ((ULONGLONG)EndBlock << BlockSizeShift));
                ForceReadParallel = TRUE;
            }

            if (DeviceBlock != 0) 
            {
                RtlCopyMemory((PCHAR)Buffer + BufferOffset,
                    (PCHAR)Block + BlockOffset, BlockLength);
            }
            else
            {
                RtlZeroMemory((PCHAR)Buffer + BufferOffset, BlockLength);
            }
        }
        else if (DeviceBlock == 0)
        {
            RtlZeroMemory((PCHAR)Buffer +
                Delta + ((i - StartBlock - 1) << BlockSizeShift), BlockSize);
            ForceReadParallel = TRUE;
        }
        else
        {
            if (BlockGroupCount == 0)
                BlockGroupOffs = Delta + ((i - StartBlock - 1) << BlockSizeShift);
            BlockGroup[BlockGroupCount++] = DeviceBlock;
        }

        if (BlockGroupCount == MAX_PARALLEL_READ || ForceReadParallel)
        {
            status = Ext2MultiReadBlock(IrpContext, BlockGroupOffs, 
                BlockGroup, BlockGroupCount);
            if (!NT_SUCCESS(status))
                break;
            BlockGroupCount = 0;
            ForceReadParallel = FALSE;
        }
    }

    ExFreeToPagedLookasideList(&Vcb->FileBlockPLAList, Block);

    return status;
}


/////////////////////////////////////////////////////////////////////////////
// Ext2InitializeDir(...)
// Expected IRQL: PASSIVE, APC;
// Section: paged.
// Cause page errors: YES_IF(Vcb, Dir)

extern "C"
VOID
Ext2InitializeDir(
                  IN OUT PEXT2_DIR  Dir       
                  )
{
    PAGED_CODE();

    ASSERT(Dir);

    KdHeader(KD_LEV2, "Ext2InitializeDir");

    Dir->RecodeBuffer = NULL;
    Dir->DirData = NULL;
    Dir->Offset = NULL;
    Dir->Count = 0;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2UninitializeDir(...)
// Expected IRQL: PASSIVE, APC;
// Section: paged.
// Cause page errors: YES_IF(Vcb, Dir)

extern "C"
VOID
Ext2UninitializeDir(
                    IN OUT PEXT2_DIR  Dir
                    )
{
    PAGED_CODE();

    ASSERT(Dir);

    KdHeader(KD_LEV2, "Ext2UninitializeDir");

    if (Dir->RecodeBuffer)
        ExFreePool(Dir->RecodeBuffer);

    if (Dir->DirData)
        ExFreePool(Dir->DirData);

    if (Dir->Offset)
        Ext2FreeOffset(Dir->Offset);
}

/////////////////////////////////////////////////////////////////////////////
// Ext2GetEntryCount(...)
// Expected IRQL: PASSIVE, APC;
// Section: paged.
// Cause page errors: YES_IF(Dir, InodeCount)

extern "C"
ULONG
Ext2GetEntryCount(
                  IN PEXT2_DIR      Dir
                  )
{
    ASSERT(Dir);

    PAGED_CODE();

    return Dir->Count;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2ReadDir(...)
// Expected IRQL: PASSIVE, APC;
// Section: paged.
// Cause page errors: YES_IF(Vcb, Inode)

extern "C"
NTSTATUS
Ext2ReadDir(
            IN PVCB             Vcb,
            IN PEXT2_INODE      Inode,
            IN BOOLEAN          IncludeDots,
            IN OUT PEXT2_DIR    Dir
            )
{
    PAGED_CODE();

    ASSERT(Vcb);
    ASSERT(Dir);
    ASSERT(Inode);
    ASSERT(S_ISDIR(Inode->Mode));

    KdHeader(KD_LEV1, "Ext2ReadDir");

    NTSTATUS status = STATUS_SUCCESS;
    if (Inode->Size == 0)
        return status;

    ULONG DirSize = (ULONG)Inode->Size; 

    PVOID DirData = NULL;
    PULONG Offset = NULL;
    PUCHAR RecodeBuffer = NULL;  
    ULONG Count;

    __try
    {
        if (Inode->Size % Vcb->SuperBlock.BlockSize)
        {
            KdPrint(("[ext.sys] - invalid directory\n"));
            status =  STATUS_NOT_A_DIRECTORY;
            __leave;
        }

        RecodeBuffer = (PUCHAR)ExAllocatePool(PagedPool, EXT2_NAME_LEN + 1);
        if (RecodeBuffer == NULL)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        DirData = ExAllocatePoolWithTag(PagedPool, DirSize, EXT2_DIR_TAG);
        if (DirData == NULL)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        status = Ext2ReadDirCached(Vcb, Inode, DirSize, DirData);
        if (!NT_SUCCESS(status))
            __leave;

        status = Ext2ParseDirAndAllocateOffset(DirData, DirSize,
            &Vcb->SuperBlock, IncludeDots, &Count, &Offset);
    }
    __finally
    {
        if (NT_SUCCESS(status))
        {
            Dir->CodePageData = Vcb->CodePageData;
            Dir->RecodeBuffer = RecodeBuffer;
            Dir->DirData = DirData;
            Dir->Offset = Offset;
            Dir->Count = Count;
        }
        else
        {
            if (DirData)
                ExFreePool(DirData);
            if (Offset)
                Ext2FreeOffset(Offset);
        }
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2GetEntry(...)
// Expected IRQL: PASSIVE;
// Section: paged.
// Cause page errors: YES_IF(Dir, Entry)

extern "C"
VOID
Ext2GetEntry(
             IN PEXT2_DIR           Dir,
             IN ULONG               Num,
             IN OUT PEXT2_DIR_ENTRY Entry
             )
{
    PAGED_CODE();

    ASSERT(Dir);
    ASSERT(Entry);

    KdHeader(KD_LEV2, "Ext2GetEntry");

    PEXT2_DIR_ENTRY_REAL RealEntry = PEXT2_DIR_ENTRY_REAL((PCHAR)Dir->DirData + Dir->Offset[Num]);

    // name
    ULONG NameLen = RealEntry->name_len;
    Ext2RecodeBytes(Dir->CodePageData,
        (PUCHAR)RealEntry + 8, Dir->RecodeBuffer, NameLen);
    Dir->RecodeBuffer[NameLen] = '\0';
    ANSI_STRING AnsiName;
    RtlInitAnsiString(&AnsiName, (PCHAR)(Dir->RecodeBuffer));
    Entry->Name.Buffer = Entry->Buffer;
    Entry->Name.MaximumLength = sizeof(Entry->Buffer);
    Entry->Name.Length = 0;
    RtlAnsiStringToUnicodeString(&Entry->Name, &AnsiName, FALSE);

    // inode
    Entry->Inode = RealEntry->inode;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2NameToInode(...)
// Expected IRQL: PASSIVE, APC;
// Section: paged.
// Cause page errors: YES_IF(Vcb, Name, Inode)

extern "C"
NTSTATUS
Ext2NameToInode(
                IN PVCB                 Vcb,
                IN PUNICODE_STRING      Name,
                IN OUT PULONG           Inode
                )
{
    PAGED_CODE();

    ASSERT(Name);
    ASSERT(Name->Buffer);
    ASSERT(Inode);

    KdHeader(KD_LEV1, "Ext2NameToInode");

    if (Name->Length == 0)
        return STATUS_NO_SUCH_FILE;

    if (Name->Buffer[0] != '\\')
        return STATUS_NO_SUCH_FILE;

    // check if root 
    if (Name->Length == 2)
    {
        *Inode = EXT2_ROOT_INO;
        return STATUS_SUCCESS;
    }

    PWCHAR Buffer = Name->Buffer;
    ULONG Length = Name->Length >> 1;

    // first, lookup name cache
    ULONG CurNameOff = Length;
    ULONG CurInode = 0;
    while (CurNameOff > 0 && !CurInode)
    {
        CurInode = Ext2LookupNameCache(&Vcb->NameCache, Buffer, CurNameOff--);
        if (CurInode == 0)
        {
            while (Buffer[CurNameOff] != '\\')
                CurNameOff--;
        }
        else
            CurNameOff++;
    }

    if (CurInode == 0)
        CurInode = EXT2_ROOT_INO; // CurNameOff = 0 too

    // resolve name
    PEXT2_DIR_ENTRY Entry = NULL;
    if (CurNameOff < Length)
    {
        Entry = (PEXT2_DIR_ENTRY)ExAllocatePoolWithTag(PagedPool,
            sizeof(EXT2_DIR_ENTRY), EXT2_ENTRY_TAG); // to slow

        if (Entry == NULL)
            return STATUS_INSUFFICIENT_RESOURCES;
    }

    NTSTATUS status = STATUS_SUCCESS;
    while (CurNameOff < Length && NT_SUCCESS(status))
    {
        // get name component
        ULONG CompOff = CurNameOff + 1;
        while (Buffer[CompOff] != '\\' && Buffer[CompOff] != '\0')
            CompOff++;

        ULONG CompLength = (CompOff - CurNameOff - 1) << 1;
        if (CompLength == 0)
        {
            status = STATUS_NO_SUCH_FILE;
            break;
        }

        EXT2_INODE DirInode;
        status = Ext2ReadInode(Vcb, CurInode, &DirInode);
        if (!NT_SUCCESS(status))
            break;

        if (!S_ISDIR(DirInode.Mode))
        {
            status = STATUS_NO_SUCH_FILE;
            break;
        }

        EXT2_DIR Dir;
        Ext2InitializeDir(&Dir);

        status = Ext2ReadDir(Vcb, &DirInode, FALSE, &Dir);
        if (!NT_SUCCESS(status))
            break;

        status = STATUS_NO_SUCH_FILE;
        ULONG EntryCount = Ext2GetEntryCount(&Dir);
        for (ULONG i = 0; i < EntryCount; i++)
        {
            Ext2GetEntry(&Dir, i, Entry);
           
            BOOLEAN Find = CompLength == Entry->Name.Length && 
                RtlCompareMemory(Buffer + CurNameOff + 1, 
                &Entry->Buffer, CompLength) == CompLength;

            if (Find)
            {
                CurInode = Entry->Inode;
                Ext2PutInNameCache(&Vcb->NameCache, Buffer, CompOff, CurInode);
                status = STATUS_SUCCESS;
                break;
            }
        }

        Ext2UninitializeDir(&Dir);

        CurNameOff = CompOff;
    }

    if (Entry)
        ExFreePool(Entry);

    if (NT_SUCCESS(status))
        *Inode = CurInode;

    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
ULONG
Ext2GetInodeAttributes(
                       PEXT2_INODE  Inode
                       )
{
    PAGED_CODE();

    KdHeader(KD_LEV1, "Ext2GetInodeAttributes");

    ULONG Attr = FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY;
    if (S_ISDIR(Inode->Mode))
        SetFlag(Attr, FILE_ATTRIBUTE_DIRECTORY);
    return Attr;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2ReadSymbolicLink(...)
// Expected IRQL: PASSIVE, APC
// Section: paged.
// Cause page errors: YES_IF(Vcb, Inode, Buffer), internal paged allocation

extern "C"
NTSTATUS
Ext2ReadSymbolicLink(
                     IN PVCB        Vcb,
                     IN PEXT2_INODE Inode,
                     IN ULONG       Offset,
                     IN ULONG       Length,
                     IN OUT PVOID   Buffer
                     )
{
    PAGED_CODE();

    ASSERT(Vcb);
    ASSERT(S_ISLNK(Inode->Mode));

    KdHeader(KD_LEV1, "Ext2ReadSymbolicLink");

    PVOID Block = ExAllocateFromPagedLookasideList(&Vcb->FileBlockPLAList);
    if (Block == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;

    NTSTATUS status = STATUS_SUCCESS;
    ULONG SymSize = (ULONG)Inode->Size;

    if (Offset + Length <= SymSize)
    {
        if (!Inode->Blocks)
        {
            RtlCopyMemory(Block, &Inode->Block[0], SymSize);
        }
        else
        {
            status = Ext2ReadBlock(Vcb->TargetDevice, &Vcb->SuperBlock,
                Inode->Block[0], 1, Block);
        }
        
        if (NT_SUCCESS(status))
            RtlCopyMemory(Buffer, BUFFER(Block, Offset), Length);
    }
    else
        status = STATUS_UNSUCCESSFUL;

    ExFreeToPagedLookasideList(&Vcb->FileBlockPLAList, Block);
    return status;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2ReadVolume(...)
// Expected IRQL: PASSIVE, APC
// Section: NONpaged. (handle double nested page fault);

extern "C"
NTSTATUS
Ext2ReadVolume(
               IN PIRP_CONTEXT  IrpContext,
               IN PULONGLONG    Offset,
               IN ULONG         Length
               )
{
    return Ext2ContReadBlock(IrpContext, Offset, Length);
}

/////////////////////////////////////////////////////////////////////////////
