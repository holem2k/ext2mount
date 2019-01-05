#include "stdafx.h"
#include "ext2alg.h"
#include "cachesup.h"
#include "cmcallbacks.h"

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2CreateIrpContext(
                  PDEVICE_OBJECT    Device, 
                  PIRP              Irp,
                  PIRP              TopLevelIrp,
                  PIRP_CONTEXT      *IrpContextRes
                  )
{
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    if (IrpStack->DeviceObject == Ext2GlobalData.Device)
    {
        if (IrpStack->MajorFunction != IRP_MJ_CREATE &&
            IrpStack->MajorFunction != IRP_MJ_CLEANUP &&
            IrpStack->MajorFunction != IRP_MJ_CLOSE &&
            IrpStack->FileObject != NULL)
        {
            return STATUS_INVALID_DEVICE_REQUEST;
        }
    }

    PIRP_CONTEXT IrpContext;
    IrpContext = (PIRP_CONTEXT)ExAllocateFromNPagedLookasideList(&Ext2GlobalData.IrpContextList);

    if (IrpContext)
    {
        IrpContext->Device = Device;
        IrpContext->ExceptionInProgress = FALSE;
        IrpContext->PopupOnError = TRUE;
        IrpContext->Irp = Irp;

        IrpContext->MajorFunction = IrpStack->MajorFunction;
        IrpContext->MinorFunction = IrpStack->MinorFunction;
        IrpContext->FileObject = IrpStack->FileObject; 
        IrpContext->TopLevelIrp = TopLevelIrp;

        // obsolete
        switch (IrpStack->MajorFunction)
        {
        case IRP_MJ_FILE_SYSTEM_CONTROL:
            IrpContext->IsSync = TRUE;
            break;

        case IRP_MJ_CLOSE:
        case IRP_MJ_CLEANUP:
            IrpContext->IsSync = TRUE;
            break;

        default:
            IrpContext->IsSync = IoIsOperationSynchronous(Irp);
        }
            
    }
    else
        return STATUS_INSUFFICIENT_RESOURCES;

    *IrpContextRes = IrpContext;
    return STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2DestroyIrpContext(
                      PIRP_CONTEXT  IrpContext
                      )
{
    ExFreeToNPagedLookasideList(&Ext2GlobalData.IrpContextList, IrpContext);    
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2CompleteRequest(
                     PIRP_CONTEXT   IrpContext,
                     NTSTATUS       Status,
                     ULONG          Information,
                     CCHAR          PriorityBoost
                     )
{
    IrpContext->Irp->IoStatus.Status = Status;
    IrpContext->Irp->IoStatus.Information = Information;
    IoCompleteRequest(IrpContext->Irp, PriorityBoost);
    ExFreeToNPagedLookasideList(&Ext2GlobalData.IrpContextList, IrpContext);    
    return Status;
}


/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2ConvertSuperBlock(
                      PEXT2_SUPERBLOCK      MemSuperBlock,
                      PEXT2_SUPERBLOCK_REAL SuperBlock
                      )
{
    ASSERT(MemSuperBlock);
    ASSERT(SuperBlock);

    MemSuperBlock->BlocksCount = SuperBlock->s_blocks_count;
    MemSuperBlock->CheckInterval = SuperBlock->s_checkinterval;
    MemSuperBlock->Errors = SuperBlock->s_errors;
    MemSuperBlock->FreeBlocksCount = SuperBlock->s_free_blocks_count;
    MemSuperBlock->FreeInodesCount = SuperBlock->s_free_inodes_count;
    MemSuperBlock->InodesCount = SuperBlock->s_inodes_count;
    MemSuperBlock->InodesCount = SuperBlock->s_inodes_count;
    MemSuperBlock->InodesPerGroup = SuperBlock->s_inodes_per_group;
    MemSuperBlock->LastCheck = SuperBlock->s_lastcheck;
    MemSuperBlock->Magic = SuperBlock->s_magic;
    MemSuperBlock->MaxMntCount = SuperBlock->s_max_mnt_count;
    MemSuperBlock->MinorRevLevel = SuperBlock->s_minor_rev_level;
    MemSuperBlock->MntCount = SuperBlock->s_mnt_count;
    MemSuperBlock->MountTime = SuperBlock->s_mtime;
    MemSuperBlock->ResBlocksCount = SuperBlock->s_r_blocks_count;
    MemSuperBlock->RevLevel = SuperBlock->s_rev_level;
    MemSuperBlock->State = SuperBlock->s_state ;
    MemSuperBlock->InodeSize = SuperBlock->s_inode_size;
    MemSuperBlock->BlocksPerGroup = SuperBlock->s_blocks_per_group;
    MemSuperBlock->BlockSize = 1024 << SuperBlock->s_log_block_size;
    MemSuperBlock->BlockSizeShift = (UCHAR)(10 + SuperBlock->s_log_block_size);
    MemSuperBlock->FirstInode = SuperBlock->s_first_ino;
    MemSuperBlock->FirstDataBlock = SuperBlock->s_first_data_block;
    MemSuperBlock->FeatureCompat = SuperBlock->s_feature_compat;
    MemSuperBlock->FeatureIncompat = SuperBlock->s_feature_incompat;

    // correct if old revision
    if (MemSuperBlock->RevLevel == EXT2_GOOD_OLD_REV)
    {
        MemSuperBlock->InodeSize = EXT2_GOOD_OLD_INODE_SIZE;
        MemSuperBlock->FirstInode = EXT2_GOOD_OLD_FIRST_INO;
    }

}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2ConvertGroupDesc(
                     PEXT2_GROUP_DESC       GroupDesc,
                     PEXT2_GROUP_DESC_REAL  RealGroupDesc
                     )
{
    GroupDesc->BlockBitmap = RealGroupDesc->bg_block_bitmap;
    GroupDesc->FreeBlocksCount = RealGroupDesc->bg_free_blocks_count;
    GroupDesc->FreeInodesCount = RealGroupDesc->bg_free_inodes_count;
    GroupDesc->InodeBitmap = RealGroupDesc->bg_inode_bitmap;
    GroupDesc->InodeTable = RealGroupDesc->bg_inode_table;
    GroupDesc->UsedDirsCount = RealGroupDesc->bg_used_dirs_count;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2ConvertInode(
                 PEXT2_INODE       Inode,
                 PEXT2_INODE_REAL  RealInode
                 )
{
    Inode->Mode = RealInode->i_mode;
    Inode->UID = RealInode->i_uid;
    Inode->ATime = RealInode->i_atime;
    Inode->CTime = RealInode->i_ctime;
    Inode->MTime = RealInode->i_mtime;
    Inode->GID = RealInode->i_gid;
    Inode->LinksCount = RealInode->i_links_count;
    Inode->Flags = RealInode->i_flags;
    Inode->Blocks = RealInode->i_blocks;
    for (ULONG i = 0; i < EXT2_N_BLOCKS; i++)
        Inode->Block[i] = RealInode->i_block[i];

    Inode->Size = RealInode->i_size;

    if (S_ISREG(Inode->Mode))
        Inode->Size |= ((ULONGLONG)RealInode->i_size_high) << 32;

    if (!(S_ISREG(Inode->Mode) || S_ISLNK(Inode->Mode) || S_ISDIR(Inode->Mode)))
        Inode->Size = 0;
}

/////////////////////////////////////////////////////////////////////////////
// Param: Name - string allocated with ExAllocatePool

extern "C"
PFCB 
Ext2CreateFCB(
              IN ULONG      InodeNum,
              IN PCWSTR     Name
              )
{
    PAGED_CODE();

    PEXT2_INODE Inode = (PEXT2_INODE)ExAllocatePoolWithTag(PagedPool, sizeof(EXT2_INODE),
        EXT2_FCBINODE_TAG);

    if (Inode == NULL)
        return NULL;

    PFCB Fcb = (PFCB)ExAllocatePoolWithTag(NonPagedPool, sizeof(FCB), EXT2_FCB_TAG);
    if (Fcb)
    {
        RtlZeroMemory(Fcb, sizeof(FCB));
        Fcb->NodeTypeCode = FCB_TYPE;
        Fcb->NodeByteSize = sizeof(FCB);

        Fcb->InodeNum = InodeNum;
        Fcb->ReferenceCount = Fcb->OpenHandleCount = 0;

        ExInitializeResourceLite(&Fcb->MainResource);
        ExInitializeResourceLite(&Fcb->PagingResource);

        Fcb->SectionObject.DataSectionObject = NULL;
        Fcb->SectionObject.ImageSectionObject = NULL;
        Fcb->SectionObject.SharedCacheMap = NULL;

        Fcb->IsFastIoPossible = FastIoIsNotPossible;
        Fcb->Resource = &Fcb->MainResource;
        Fcb->PagingIoResource = &Fcb->PagingResource;

        Fcb->Inode = Inode;
        RtlInitUnicodeString(&Fcb->Name, Name);
    }
    else
        ExFreePool(Inode);

    return Fcb;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2FreeFCB(
            IN PFCB    Fcb
            )
{
    PAGED_CODE();

    ASSERT(Fcb);
    ASSERT(!Fcb->ReferenceCount && !Fcb->OpenHandleCount);

    if (Fcb->Name.Buffer)
        ExFreePool(Fcb->Name.Buffer);

    ExFreePool(Fcb->Inode);

    ExDeleteResourceLite(&Fcb->MainResource);
    ExDeleteResourceLite(&Fcb->PagingResource);

    ExFreePool(Fcb);
}

/////////////////////////////////////////////////////////////////////////////
// Ext2InitVCB()
// Expected IRQL: PASSIVE
// Section: paged.
// Cause page errors: YES
// Locks : Ext2GlobalData[EXCL]

extern "C"
BOOLEAN
Ext2InitVCB(
            PVCB                Vcb,
            PDEVICE_OBJECT      TargetDevice,
            PDEVICE_OBJECT      VolumeDevice,
            PVPB                Vpb,
            PEXT2_SUPERBLOCK    SuperBlock,
            PEXT2_GROUP_DESC    GroupDesc,
            ULONG               GroupCount,
            PCODE_PAGE_DATA     CodePageData,
            ULONG               ChangeCount
            )
{
    PAGED_CODE();

    ASSERT(Vcb);
    
    Vcb->NodeTypeCode = (SHORT)VCB_TYPE;
    Vcb->NodeByteSize = (SHORT)sizeof(VCB);
    Vcb->TargetDevice = TargetDevice;
    Vcb->VolumeDevice = VolumeDevice;
    Vcb->Vpb = Vpb;
    Vcb->FreeVpb = FALSE;
    Vcb->GroupDesc = GroupDesc;
    Vcb->GroupCount = GroupCount;
    Vcb->Flags = 0;
    Vcb->CodePageData = CodePageData;
    Vcb->VcbState = VCB_MOUNTED;

    ExInitializeResourceLite(&Vcb->MainResource);
    Vcb->ReferenceCount = MOUNT_REF;
    RtlCopyMemory(&Vcb->SuperBlock, SuperBlock, sizeof(EXT2_SUPERBLOCK));

    Ext2InitNameCache(&Vcb->NameCache);

    ObReferenceObject(TargetDevice);

    ExInitializePagedLookasideList(&Vcb->FileBlockPLAList,
        NULL, NULL, 0, SuperBlock->BlockSize, EXT2_BLOCK_TAG, 0);
    ExInitializePagedLookasideList(&Vcb->InodePLAList,
        NULL, NULL, 0, SuperBlock->BlockSize, EXT2_INODE_TAG, 0);

    InitializeListHead(&Vcb->FcbList);

    InitializeListHead(&Vcb->DirNotifyList);
    FsRtlNotifyInitializeSync(&Vcb->NotifySync);

    Vcb->VolumeLockFile = NULL;

    // init meta stream last, it will fire IRP_MJ_CLEANUP request - do nothing there
    Vcb->MetaStreamFile = Ext2CreateMetaStream(Vpb,
        ((ULONGLONG)SuperBlock->BlockSize) * SuperBlock->BlocksCount);
    if (Vcb->MetaStreamFile == NULL)
        return FALSE;
    Vcb->ReferenceCount += 1; // meta stream reference

    InsertTailList(&Ext2GlobalData.VcbList, &Vcb->ListEntry);

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2UninitVCB()
// Expected IRQL: PASSIVE
// Section: paged.
// Cause page errors: YES
// Locks : Ext2GlobalData[EXCL]

extern "C"
VOID
Ext2UninitVCB(
            PVCB    Vcb
            )
{
    PAGED_CODE();

    ASSERT(Vcb);

    Ext2UninitNameCache(&Vcb->NameCache);

    ExDeleteResourceLite(&Vcb->MainResource);

    ObDereferenceObject(Vcb->TargetDevice);

    ExDeletePagedLookasideList(&Vcb->FileBlockPLAList);

    ExDeletePagedLookasideList(&Vcb->InodePLAList);

    RemoveEntryList(&Vcb->ListEntry);

    FsRtlNotifyUninitializeSync(&Vcb->NotifySync);

    if (Vcb->FreeVpb && Vcb->Vpb->ReferenceCount == 0)
    {
        ExFreePool(Vcb->Vpb);
    }

    KdPrint(("Delete device %x\n", Vcb->VolumeDevice));
    IoDeleteDevice(Vcb->VolumeDevice);
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
PCCB
Ext2CreateCCB(
              )
{
    PAGED_CODE();

    PCCB Ccb = (PCCB)ExAllocatePoolWithTag(PagedPool, sizeof(CCB), EXT2_CCB_TAG);
    if (Ccb)
    {
        RtlZeroMemory(Ccb, sizeof(CCB));
        Ccb->NodeTypeCode = CCB_TYPE;
        Ccb->NodeByteSize = sizeof(CCB);
        Ccb->MatchAll = FALSE;
        // Ccb->ReallyWild = TRUE;
    }
    return Ccb;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2FreeCCB(
            PCCB    Ccb
            )
{
    PAGED_CODE();

    ASSERT(Ccb);

    if (Ccb->SearchPattern.Buffer)
        ExFreePool(Ccb->SearchPattern.Buffer);

    ExFreePool(Ccb);
}

/////////////////////////////////////////////////////////////////////////////
// Ext2RemoveFCB
// Lock: Vcb [SHARED]

extern "C"
PFCB
Ext2LookupFCB(
              PVCB      Vcb,
              ULONG     InodeNum
              )
{
    PFCB Fcb = NULL;
    PLIST_ENTRY Entry = Vcb->FcbList.Flink;
    while (Entry != &Vcb->FcbList)
    {
        PFCB ContFcb = CONTAINING_RECORD(Entry, FCB, ListEntry);
        if (ContFcb->InodeNum == InodeNum)
        {
            Fcb = ContFcb;
            break;
        }
        Entry = Entry->Flink;
    }
    return Fcb;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2AddFCB
// Lock: Vcb [EXCL]

extern "C"
VOID
Ext2AddFCB(
           PVCB     Vcb,
           PFCB     Fcb
           )
{
    InsertTailList(&Vcb->FcbList, &Fcb->ListEntry);
}

/////////////////////////////////////////////////////////////////////////////
// Ext2RemoveFCB
// Lock: Vcb [EXCL]

extern "C"
VOID
Ext2RemoveFCB(
           PVCB     Vcb,
           PFCB     Fcb
           )
{
    RemoveEntryList(&Fcb->ListEntry);
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2SetFcbFileSizes(
                    PFCB    Fcb,
                    ULONG   AllocationUnit
                    )
{
    Fcb->AllocationSize.QuadPart = ALIGN(Fcb->Inode->Size, AllocationUnit, ULONGLONG);
    Fcb->FileSize.QuadPart = Fcb->Inode->Size;
    Fcb->ValidDataLength.QuadPart = Fcb->Inode->Size;    
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
CSHORT
Ext2GetFcbType(
               PFILE_OBJECT FileObject
               )
{
    if (FileObject->FsContext == NULL)
        return UNK_TYPE;

    CSHORT Type = *(PCSHORT)FileObject->FsContext;
    switch (Type)
    {
    case FCB_TYPE:
    case VCB_TYPE:
    case MFCB_TYPE:
        break;

    default:
        Ext2BugCheck(BUGCHECK_STRUCT, Type, 0, 0);
    }
    return Type;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
inline
PVCB
Ext2GetVcb(
           PIRP_CONTEXT IrpContext
           )
{
    ASSERT(IrpContext->Device->DeviceExtension);
    return (PVCB)IrpContext->Device->DeviceExtension;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
inline
PCOMMON_FCB
Ext2GetCommonFcb(
                 PFILE_OBJECT   FileObject
                 )
{
    return (PCOMMON_FCB)FileObject->FsContext;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2GetFCBandVCB(
                 PIRP_CONTEXT   IrpContext,
                 PVCB           *Vcb,
                 PFCB           *Fcb
                 )
{
    ASSERT(Vcb);
    ASSERT(Fcb);

    *Vcb = (PVCB)IrpContext->Device->DeviceExtension;
    *Fcb = (PFCB)IrpContext->FileObject->FsContext;

    ASSERT(*Vcb);
    ASSERT((*Vcb)->NodeTypeCode == VCB_TYPE);
    ASSERT(*Fcb);
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
PCCB
Ext2GetCCB(
           PIRP_CONTEXT   IrpContext
           )
{
    return (PCCB)IrpContext->FileObject->FsContext2;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2SetCCBSearchPattern(...)
// IRQL: PASSIVE

extern "C"
NTSTATUS
Ext2SetCCBSearchPattern(
                        PCCB            Ccb,
                        PUNICODE_STRING NewSearchPattern
                        )
{
    PAGED_CODE();

    PUNICODE_STRING Pattern = &Ccb->SearchPattern;
    USHORT Length = NewSearchPattern->Length;

    Pattern->Buffer = (PWCHAR)ExAllocatePoolWithTag(PagedPool,
        Length, EXT2_PATTERN_TAG);
    if (Pattern->Buffer == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;

    RtlCopyMemory(Pattern->Buffer, NewSearchPattern->Buffer, Length);
    Pattern->Length = Length;
    Pattern->MaximumLength = Length;
    return STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2CheckForDismount
// LOCKS: Vcb[EXCL]
// REM: returns TRUE if Vcb needs to be unlocked,
// FALSE if Vcb was unlocked and destroyed.

extern "C"
inline
BOOLEAN
Ext2CheckForDismount(
                     IN PVCB Vcb
                     )
{
    KdHeader(KD_LEV1, "Ext2CheckForDismount");

    KdPrint(("Vcb->ReferenceCount = %X\n", Vcb->ReferenceCount));

    BOOLEAN Unlocked = FALSE;
    if (Vcb->ReferenceCount == 1 && Vcb->VcbState != VCB_DEAD) // Vcb is not mounted or dismounted
    {
        KdInfo("kill meta stream");
        Ext2DismountVcb(Vcb);
        KdInfo("after kill meta stream");
    }
    else if (Vcb->ReferenceCount == 0)
    {
        KdInfo("try kill VCB");
        
        if (Vcb->Vpb->ReferenceCount == 1)
        {
            KdInfo("kill VCB");
            
            ExReleaseResourceLite(&Vcb->MainResource);
            Unlocked = TRUE;
            Vcb->Vpb->ReferenceCount -= 1; // kill fsd ref
            // if Ext2CheckForDismount was called from Ext2Mount, 
            // then it's recursive acquire
            ExAcquireResourceExclusiveLite(&Ext2GlobalData.DataResource, TRUE);
            Ext2UninitVCB(Vcb); // Vcb really dead now
            ExReleaseResourceLite(&Ext2GlobalData.DataResource);
        }
        else
            KdInfo("can't kill VCB because VPB");
    }
    return Unlocked;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2DismountVcb(
                IN PVCB Vcb
                )
{
    Vcb->VcbState = VCB_DEAD;

    PVPB NewVpb = (PVPB)ExAllocatePool(NonPagedPoolMustSucceed, sizeof(VPB));
    RtlZeroMemory(NewVpb, sizeof(VPB));
    PVPB OldVpb = Vcb->Vpb;

    // work with Vpb
    BOOLEAN LastRef = OldVpb->ReferenceCount == 1;

    KIRQL SaveIrql;
    IoAcquireVpbSpinLock(&SaveIrql);

    if (OldVpb->RealDevice->Vpb == OldVpb)
    {
        if (!LastRef)
        {
            // swap VPB
            NewVpb->Type = IO_TYPE_VPB;
            NewVpb->Size = sizeof(VPB);
            NewVpb->RealDevice = OldVpb->RealDevice;
            NewVpb->RealDevice->Vpb = NewVpb;
            NewVpb->Flags = FlagOn(OldVpb->Flags, VPB_REMOVE_PENDING);

            NewVpb = NULL;
            Vcb->FreeVpb = TRUE;
        }
        else
        {
            // leave VPB for I/O system (Vcb->FreeVpb = FALSE;)
            OldVpb->DeviceObject = NULL;
            ClearFlag(OldVpb->Flags, VPB_MOUNTED);
        }
    }
    else
    {
        // somebody swapped VPB
        if (!LastRef)
        {
            ClearFlag(OldVpb->Flags, VPB_MOUNTED);
        }
        Vcb->FreeVpb = TRUE;
    }
    IoReleaseVpbSpinLock(SaveIrql);

    Ext2DeleteMetaStream(Vcb->MetaStreamFile);

    if (NewVpb)
        ExFreePool(NewVpb);
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOLEAN
Ext2EqualSuperBlock(
                    PEXT2_SUPERBLOCK    SuperBlock1,
                    PEXT2_SUPERBLOCK    SuperBlock2
                    )
{
    return SuperBlock1->BlockSize == SuperBlock2->BlockSize &&
        SuperBlock1->FreeInodesCount == SuperBlock2->FreeInodesCount &&
        SuperBlock1->MountTime == SuperBlock2->MountTime &&
        SuperBlock1->LastCheck == SuperBlock2->LastCheck &&
        SuperBlock1->MntCount == SuperBlock2->MntCount &&
        SuperBlock1->BlocksCount == SuperBlock2->BlocksCount;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
PMFCB
Ext2CreateMFCB(
               ULONGLONG Size
               )
{
    PMFCB Fcb = (PMFCB)ExAllocatePoolWithTag(NonPagedPool, 
        sizeof(MFCB), EXT2_MFCB_TAG);

    if (Fcb)
    {
        RtlZeroMemory(Fcb, sizeof(MFCB));
        Fcb->NodeTypeCode = MFCB_TYPE;
        Fcb->NodeByteSize = sizeof(MFCB);

        ExInitializeResourceLite(&Fcb->MainResource);
        ExInitializeResourceLite(&Fcb->PagingResource);

        Fcb->IsFastIoPossible = FastIoIsNotPossible;
        Fcb->Resource = &Fcb->MainResource;
        Fcb->PagingIoResource = &Fcb->PagingResource;

        Fcb->SectionObject.DataSectionObject = NULL;
        Fcb->SectionObject.ImageSectionObject = NULL;
        Fcb->SectionObject.SharedCacheMap = NULL;

        Fcb->AllocationSize.QuadPart = Size;
        Fcb->FileSize.QuadPart = Size;
        Fcb->ValidDataLength.QuadPart = Size;
    }
    return Fcb;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2FreeMFCB(
             PMFCB  Fcb
             )
{
    ASSERT(Fcb);

    ExDeleteResourceLite(&Fcb->MainResource);
    ExDeleteResourceLite(&Fcb->PagingResource);

    ExFreePool(Fcb);
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID 
Ext2InitGlobalData(
                  PEXT2_GLOBAL_DATA GlobalData,
                  PDEVICE_OBJECT    Device,
                  PDRIVER_OBJECT    Driver,
                  PUNICODE_STRING   RegPath
                  )
{
        GlobalData->Device = Device;
        GlobalData->Driver = Driver;
        ExInitializeResourceLite(&GlobalData->DataResource);
        ExInitializeNPagedLookasideList(&GlobalData->IrpContextList,
            NULL, NULL, 0, sizeof(IRP_CONTEXT), 0, 0);
        InitializeListHead(&Ext2GlobalData.VcbList);
        GlobalData->CacheMgrCallbacks.AcquireForLazyWrite = Ext2AcquireForLazyWrite;
        GlobalData->CacheMgrCallbacks.ReleaseFromLazyWrite = Ext2ReleaseFromLazyWrite;
        GlobalData->CacheMgrCallbacks.AcquireForReadAhead = Ext2AcquireForReadAhead;
        GlobalData->CacheMgrCallbacks.ReleaseFromReadAhead = Ext2ReleaseFromReadAhead;
        GlobalData->MetaCacheCallbacks.AcquireForLazyWrite = Ext2NoopAcquire;
        GlobalData->MetaCacheCallbacks.ReleaseFromLazyWrite = Ext2NoopRelease;
        GlobalData->MetaCacheCallbacks.AcquireForReadAhead = Ext2AcquireForCache;
        GlobalData->MetaCacheCallbacks.ReleaseFromReadAhead = Ext2ReleaseFromCache;

        Ext2LoadDefaultOptions(&GlobalData->Options);
        Ext2LoadOptions(&GlobalData->Options, RegPath);

        CODE_PAGE CodePage = GlobalData->Options.CodePage;
        if (!Ext2LoadCodePage(&GlobalData->CodePageData, CodePage))
            Ext2LoadCodePage(&GlobalData->CodePageData, CP_CP1251);
}

/////////////////////////////////////////////////////////////////////////////
