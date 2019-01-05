#ifndef __EXT2API_H__
#define __EXT2API_H__

typedef struct EXT2_DIR
{
    PVOID           DirData;
    ULONG           Count;
    PULONG          Offset;
    PUCHAR          RecodeBuffer;
    PCODE_PAGE_DATA CodePageData;
} EXT2_DIR, *PEXT2_DIR;

typedef struct EXT2_DIR_ENTRY
{
    ULONG           Inode;
    UNICODE_STRING  Name;
    WCHAR           Buffer[EXT2_NAME_LEN + 1];
} EXT2_DIR_ENTRY, *PEXT2_DIR_ENTRY;


extern "C"
NTSTATUS
Ext2ReadSuperBlock(
				   IN PDEVICE_OBJECT            DeviceObject,
				   IN OUT PEXT2_SUPERBLOCK      SuperBlock
				   );

extern "C"
NTSTATUS
Ext2CheckSuperBlock(
                    IN PEXT2_SUPERBLOCK      SuperBlock
                    );

extern "C"
NTSTATUS
Ext2AllocateAndReadGroupDesc(
                             IN PDEVICE_OBJECT      DeviceObject,
                             IN PEXT2_SUPERBLOCK    SuperBlock,
                             OUT PULONG             DescCount,
                             OUT PEXT2_GROUP_DESC   *GroupDesc
                             );

extern "C"
VOID
Ext2FreeGroupDesc(
                  IN PEXT2_GROUP_DESC GroupDesc
                  );

extern "C"
NTSTATUS
Ext2ReadInode(
              IN PVCB            Vcb,
              IN ULONG           InodeNum,
              IN OUT PEXT2_INODE Inode
              );

extern "C"
NTSTATUS
Ext2NameToInode(
                IN PVCB                 Vcb,
                IN PUNICODE_STRING      Name,
                IN OUT PULONG           Inode
                );


extern "C"
NTSTATUS
Ext2ReadFile(
             IN PIRP_CONTEXT    Context,
             IN PVCB            Vcb,
             IN PEXT2_INODE     Inode,
             IN ULONGLONG      Offset,
             IN ULONG           Length,
             IN OUT PVOID       Buffer
             );


extern "C"
VOID
Ext2InitializeDir(
                  IN OUT PEXT2_DIR  Dir       
                  );


extern "C"
VOID
Ext2UninitializeDir(
                    IN OUT PEXT2_DIR  Dir
                    );


extern "C"
NTSTATUS
Ext2ReadDir(
            IN PVCB             Vcb,
            IN PEXT2_INODE      Inode,
            IN BOOLEAN          IncludeDots,
            IN OUT PEXT2_DIR    Dir
            );

extern "C"
ULONG
Ext2GetEntryCount(
                  IN PEXT2_DIR      Dir
                  );

extern "C"
VOID
Ext2GetEntry(
             IN PEXT2_DIR           Dir,
             IN ULONG               Num,
             IN OUT PEXT2_DIR_ENTRY Entry
             );

extern "C"
ULONG
Ext2GetInodeAttributes(
                       PEXT2_INODE  Inode
                       );

extern "C"
NTSTATUS
Ext2ReadSymbolicLink(
                     IN PVCB        Vcb,
                     IN PEXT2_INODE Inode,
                     IN ULONG       Offset,
                     IN ULONG       Length,
                     IN OUT PVOID   Buffer
                     );

extern "C"
NTSTATUS
Ext2ReadVolume(
               IN PIRP_CONTEXT  IrpContext,
               IN PULONGLONG    Offset,
               IN ULONG         Length
               );

#endif //__EXT2API_H__