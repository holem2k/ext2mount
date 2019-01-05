#ifndef __EXT2ALG_H__
#define __EXT2ALG_H__


#define GET_ENTRY(Dir, Offset) \
    ((PEXT2_DIR_ENTRY_REAL) ((PCHAR)Dir + Offset))

extern "C"
NTSTATUS
Ext2FileBlock(
              IN PVCB           Vcb,
              IN PEXT2_INODE    Inode,
              IN ULONG          Block,
              IN OUT PULONG     DevBlock
              );

extern "C"
VOID
Ext2InitNameCache(
                  IN PNAME_CACHE Cache
                  );

extern "C"
VOID
Ext2UninitNameCache(
                    IN PNAME_CACHE Cache
                    );


extern "C"
ULONG
Ext2LookupNameCache(
                    IN PNAME_CACHE      Cache,
                    IN PCWSTR           Name,
                    IN ULONG            Length
                    );

extern "C"
VOID
Ext2PutInNameCache(
                   IN PNAME_CACHE      Cache,
                   IN PCWSTR           Name,
                   IN ULONG            Length,
                   IN ULONG            Inode
                   );

extern "C"
NTSTATUS
Ext2ParseDirAndAllocateOffset(
                              IN PVOID              DirData,
                              IN ULONG              Size,
                              IN PEXT2_SUPERBLOCK   SuperBlock,
                              IN BOOLEAN            IncludeDots,
                              IN OUT PULONG         Count,
                              IN OUT PULONG         *Offset
                              );

extern "C"
VOID
Ext2FreeOffset(
               PULONG   Offset
               );

extern "C"
NTSTATUS
Ext2ReadDirCached(
                   IN PVCB            Vcb,
                   IN PEXT2_INODE     Inode,
                   IN ULONG           Size,
                   IN OUT PVOID       Buffer
                   );

#endif //__EXT2ALG_H__