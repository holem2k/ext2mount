#ifndef __STRUCT_H__
#define __STRUCT_H__
#include "stdafx.h"
#include "helpers.h"
#include "options.h"

typedef struct EXT2_INODE
{
    USHORT      Mode;
    USHORT      UID;
    USHORT      GID;
    ULONGLONG   Size;
    ULONG       ATime;
    ULONG       CTime;
    ULONG       MTime;
    ULONG       LinksCount;
    ULONG       Flags;
    ULONG       Blocks;
    ULONG       Block[EXT2_N_BLOCKS];
} EXT2_INODE, *PEXT2_INODE;

typedef struct EXT2_SUPERBLOCK
{
    ULONG   InodesCount;
    ULONG   BlocksCount;		
    ULONG   ResBlocksCount;
    ULONG   FreeBlocksCount;
    ULONG   FreeInodesCount;
    ULONG   InodeSize;
    ULONG   BlocksPerGroup;
    ULONG   FirstInode;
    ULONG   FirstDataBlock;
    
    ULONG   InodesPerGroup;
    ULONG   MountTime;	
    USHORT  MntCount;	
    SHORT   MaxMntCount;
    USHORT  State;
    USHORT  Errors;
    ULONG   LastCheck;
    ULONG   CheckInterval;
    ULONG   FeatureCompat;
    ULONG   FeatureIncompat;
    
    USHORT  MinorRevLevel;
    ULONG   RevLevel;
    
    ULONG   BlockSize;
    UCHAR   BlockSizeShift;
 
    USHORT  Magic;
} EXT2_SUPERBLOCK, *PEXT2_SUPERBLOCK;


typedef struct EXT2_GROUP_DESC
{
    ULONG	BlockBitmap;
    ULONG	InodeBitmap;
    ULONG	InodeTable;
    USHORT	FreeBlocksCount;
    USHORT	FreeInodesCount;
    USHORT	UsedDirsCount;
} EXT2_GROUP_DESC, *PEXT2_GROUP_DESC;

const CSHORT UNK_TYPE = 'cU';
const CSHORT VCB_TYPE = 'cV';
const CSHORT FCB_TYPE = 'cF';
const CSHORT CCB_TYPE = 'cC';
const CSHORT MFCB_TYPE = 'cM';

typedef struct EXT2_GLOBAL_DATA
{
    ERESOURCE               DataResource;
    PDEVICE_OBJECT          Device; 
    PDRIVER_OBJECT  	    Driver;
    LIST_ENTRY              VcbList;
    NPAGED_LOOKASIDE_LIST   IrpContextList;
    CACHE_MANAGER_CALLBACKS CacheMgrCallbacks;
    CACHE_MANAGER_CALLBACKS MetaCacheCallbacks;
    CODE_PAGE_DATA          CodePageData;
    EXT2_OPTIONS            Options;
    UNICODE_STRING          RegPath;
} EXT2_GLOBAL_DATA, *PEXT2_GLOBAL_DATA;


// should be replaced with 'Generic table' later
const ULONG NAME_CACHE_SIZE = 128;

typedef struct NAME_CACHE
{
    PWCHAR  Name[NAME_CACHE_SIZE]; // NOT zero ending string
    ULONG   MemLength[NAME_CACHE_SIZE]; // Length of memory, not name
    ULONG   Inode[NAME_CACHE_SIZE];
    ULONG   Next;
} NAME_CACHE, *PNAME_CACHE;

// VCB flags
const ULONG VCB_LOCKED      = 0x00000001;

// VCB state
typedef enum VCB_STATE
{
    VCB_MOUNTED,
        VCB_NOT_MOUNTED,
        VCB_DISMOUNTED,
        VCB_DEAD
};

typedef struct VCB
{
    CSHORT NodeTypeCode;
    CSHORT NodeByteSize;
    
    // sync
    ERESOURCE               MainResource;
    
    PDEVICE_OBJECT          TargetDevice;
    PDEVICE_OBJECT          VolumeDevice;
    PVPB                    Vpb;
    BOOLEAN                 FreeVpb;
    
    // meta
    EXT2_SUPERBLOCK         SuperBlock;
    PEXT2_GROUP_DESC        GroupDesc;
    ULONG                   GroupCount;
    ULONG                   Flags;
    VCB_STATE               VcbState;
    
    //
    LIST_ENTRY              FcbList;
    
    ULONG                   ReferenceCount;
    
    NAME_CACHE              NameCache;
    PAGED_LOOKASIDE_LIST    FileBlockPLAList;
    PAGED_LOOKASIDE_LIST    InodePLAList; // temporary
    
    // notification support
    PNOTIFY_SYNC            NotifySync;
    LIST_ENTRY              DirNotifyList;
    
    PFILE_OBJECT            VolumeLockFile;
    PFILE_OBJECT            MetaStreamFile;

    PCODE_PAGE_DATA         CodePageData;
    
    LIST_ENTRY              ListEntry;
} VCB, *PVCB;

typedef struct FCB : FSRTL_COMMON_FCB_HEADER // NT required header
{
    SECTION_OBJECT_POINTERS SectionObject;
    ERESOURCE               MainResource;
    ERESOURCE               PagingResource;
    // end of NT required header
    
    // meta
    PEXT2_INODE             Inode;
    ULONG                   InodeNum;
    UNICODE_STRING          Name;
    
    ULONG                   OpenHandleCount;
    ULONG                   ReferenceCount;
    //SHARE_ACCESS            ShareAccess;
    LIST_ENTRY              ListEntry;
} FCB, *PFCB;

typedef struct MFCB : FSRTL_COMMON_FCB_HEADER // NT required header
{
    SECTION_OBJECT_POINTERS SectionObject;
    ERESOURCE               MainResource;
    ERESOURCE               PagingResource;
} MFCB, *PMFCB;


typedef struct CCB
{
    CSHORT          NodeTypeCode;
    CSHORT          NodeByteSize;
    UNICODE_STRING  SearchPattern;
    BOOLEAN         ReallyWild;
    BOOLEAN         MatchAll;
    ULONG           Index;
} CCB, *PCCB;

typedef struct PARALLEL_IO_BLOCK
{
    KEVENT      Event;
    NTSTATUS    Status;
    LONG       PendingIrpCount;
} PARALLEL_IO_BLOCK, *PPARALLEL_IO_BLOCK;

typedef struct IRP_CONTEXT
{
    PIRP                Irp;
    PDEVICE_OBJECT      Device;
    BOOLEAN             ExceptionInProgress;
    BOOLEAN             IsSync;
    BOOLEAN             PopupOnError;
    WORK_QUEUE_ITEM     WorkItem;
    ULONG               MajorFunction;
    ULONG               MinorFunction;
    PFILE_OBJECT        FileObject;
    PIRP                TopLevelIrp;
    PARALLEL_IO_BLOCK   IoBlock;
} IRP_CONTEXT, *PIRP_CONTEXT;


#define SetFlag(x, f) (x) |= (f)
#define ClearFlag(x, f) (x) &= ~(f)
#define IsFlagOn(x, f) (x & f ? TRUE : FALSE)

#define EXT2_SB_TAG             '1txe'
#define EXT2_CACHE_TAG          '2txe'
#define EXT2_INODE_TAG          '3txe'
#define EXT2_BLOCK_TAG          '4txe'
#define EXT2_DIR_TAG            '5txe'
#define EXT2_OFFS_TAG           '6txe'
#define EXT2_ENTRY_TAG          '7txe'
#define EXT2_CCB_TAG            '8txe'
#define EXT2_FCB_TAG            '9txe'
#define EXT2_FILENAME_TAG       'Atxe'
#define EXT2_PATTERN_TAG        'Btxe'
#define EXT2_MFCB_TAG           'Ctxe'
#define EXT2_FCBINODE_TAG       'Dtxe'

#define BUFFER(Buffer, Offset) ((PCHAR)Buffer + (Offset))
#define SET_ENTRY_OFFSET(Buffer, EntryOffset, Offset) \
*(PULONG)BUFFER(Buffer, EntryOffset) = Offset;
#define ALIGN(A, C, Atype) ((A) + ((C) - 1) & ~(((Atype)C) - 1))

#define Ext2CanComplete(IrpContext, Status) \
(!IrpContext->ExceptionInProgress && (Status == STATUS_SUCCESS || !IoIsErrorUserInduced(Status)))

const ULONG MOUNT_REF = 1;

const ULONG SECTOR_SIZE = 512;

typedef PFSRTL_COMMON_FCB_HEADER PCOMMON_FCB;

extern "C"
NTSTATUS
Ext2CreateIrpContext(
                     PDEVICE_OBJECT    Device, 
                     PIRP              Irp,
                     PIRP              TopLevelIrp,
                     PIRP_CONTEXT      *IrpContext
                     );


extern "C"
VOID
Ext2DestroyIrpContext(
                      PIRP_CONTEXT  IrpContext
                      );


extern "C"
NTSTATUS
Ext2CompleteRequest(
                    PIRP_CONTEXT   IrpContext,
                    NTSTATUS       Status,
                    ULONG          Information = 0,
                    CCHAR          PriorityBoost = IO_DISK_INCREMENT
                    );

extern "C"
VOID
Ext2ConvertSuperBlock(
                      PEXT2_SUPERBLOCK      MemSuperBlock,
                      PEXT2_SUPERBLOCK_REAL SuperBlock      
                      );

extern "C"
VOID
Ext2ConvertGroupDesc(
                     PEXT2_GROUP_DESC       GroupDesc,
                     PEXT2_GROUP_DESC_REAL  RealGroupDesc
                     );


extern "C"
VOID
Ext2ConvertInode(
                 PEXT2_INODE       Inode,
                 PEXT2_INODE_REAL  RealInode
                 );

extern "C"
PFCB 
Ext2CreateFCB(
              IN ULONG      InodeNum,
              IN PCWSTR     Name
              );

extern "C"
VOID
Ext2FreeFCB(
            IN PFCB    Fcb
            );

extern "C"
PMFCB
Ext2CreateMFCB(
               ULONGLONG Size
               );

extern "C"
VOID
Ext2FreeMFCB(
             PMFCB  Fcb
             );

extern "C"
BOOLEAN
Ext2InitVCB(
            PVCB                Vcb,
            PDEVICE_OBJECT      TargetDevice,
            PDEVICE_OBJECT      VolumeDevice,
            PVPB                Vpb,
            PEXT2_SUPERBLOCK    Superblock,
            PEXT2_GROUP_DESC    GroupDesc,
            ULONG               GroupCount,
            PCODE_PAGE_DATA     CodePageData,
            ULONG               ChangeCount
            );

extern "C"
VOID
Ext2UninitVCB(
              PVCB    Vcb
              );

extern "C"
PCCB
Ext2CreateCCB(
              );

extern "C"
VOID
Ext2FreeCCB(
            PCCB    Ccb
            );


extern "C"
PFCB
Ext2LookupFCB(
              PVCB      Vcb,
              ULONG     InodeNum
              );

extern "C"
VOID
Ext2AddFCB(
           PVCB     Vcb,
           PFCB     Fcb
           );


extern "C"
VOID
Ext2RemoveFCB(
              PVCB     Vcb,
              PFCB     Fcb
              );

extern "C"
VOID
Ext2SetFcbFileSizes(
                    PFCB    Fcb,
                    ULONG   AllocationUnit
                    );

extern "C"
CSHORT
Ext2GetFcbType(
               PFILE_OBJECT FileObject
               );

extern "C"
inline
PVCB
Ext2GetVcb(
           PIRP_CONTEXT Context
           );

extern "C"
inline
PCOMMON_FCB
Ext2GetCommonFcb(
                 PFILE_OBJECT   FileObject
                 );

template<class T>
inline
VOID
Ext2GetFcb(
           PFILE_OBJECT FileObject,
           T            **Fcb
           )
{
    *Fcb = (T *)FileObject->FsContext;
}

extern "C" 
VOID
Ext2GetFCBandVCB(
                 PIRP_CONTEXT   IrpContext,
                 PVCB           *Vcb,
                 PFCB           *Fcb
                 ); // obsolete

extern "C"
PCCB
Ext2GetCCB(
           PIRP_CONTEXT   IrpContext
           );

extern "C"
NTSTATUS
Ext2SetCCBSearchPattern(
                        PCCB            Ccb,
                        PUNICODE_STRING NewSearchPattern
                        );

extern "C"
inline
BOOLEAN
Ext2CheckForDismount(
                     IN PVCB Vcb
                     );


extern "C"
VOID
Ext2DismountVcb(
                IN PVCB Vcb
                );

extern "C"
BOOLEAN
Ext2EqualSuperBlock(
                    PEXT2_SUPERBLOCK    SuperBlock1,
                    PEXT2_SUPERBLOCK    SuperBlock2
                    );

extern "C"
VOID 
Ext2InitGlobalData(
                   PEXT2_GLOBAL_DATA GlobalData,
                   PDEVICE_OBJECT    Device,
                   PDRIVER_OBJECT    Driver,
                   PUNICODE_STRING   RegPath
                   );

#endif //__STRUCT_H__