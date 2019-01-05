#ifndef __DIRCTRL_H__
#define __DIRCTRL_H__
#include "ext2api.h"

extern "C"
NTSTATUS
Ext2DirectoryControl(
                     PIRP_CONTEXT   IrpContext
                     );

extern "C"
NTSTATUS
Ext2QueryDirectory(
                   PIRP_CONTEXT   IrpContext
                   );

extern "C"
NTSTATUS
Ext2NotifyChangeDirectory(
                          PIRP_CONTEXT   IrpContext
                          );

// local support routines
// declared for using in #pragma...

extern "C"
NTSTATUS
Ext2ScanDirectory(
                  IN PVCB                   Vcb,
                  IN PEXT2_INODE            DirInode,
                  IN BOOLEAN                IsRootDir,
                  IN PUNICODE_STRING        SearchPattern,
                  IN OUT PULONG             Index,
                  IN BOOLEAN                FirstQuery,
                  IN BOOLEAN                SingleEntry,
                  IN BOOLEAN                MatchAll,
                  IN BOOLEAN                ReallyWild,
                  IN OUT PVOID              Buffer,
                  IN ULONG                  Length,
                  IN FILE_INFORMATION_CLASS FileInformationClass,
                  OUT PULONG                ResultLength
                  );

extern "C"
BOOLEAN
Ext2IsInformationClassSupported(
                                IN FILE_INFORMATION_CLASS FileInformationClass
                                );


extern "C"
BOOLEAN
Ext2CompareNameWithWildCard(
                            IN PUNICODE_STRING Name,
                            IN PUNICODE_STRING WildCard,
                            IN BOOLEAN ReallyWild,
                            IN BOOLEAN CaseInsensetive
                            );

extern "C"
BOOLEAN
Ext2IsInodeRequired(
                    FILE_INFORMATION_CLASS FileInformationClass
                    );


extern "C"
NTSTATUS
Ext2GetDirEntryInformation(
                           IN PEXT2_DIR_ENTRY        Entry,
                           IN PEXT2_INODE            Inode,
                           IN PVOID                  Buffer,
                           IN ULONG                  Offset,
                           IN ULONG                  Length,
                           IN FILE_INFORMATION_CLASS FileInformationClass,
                           IN ULONG                  AllocSize,             
                           IN OUT PULONG             EntryLength,
                           IN OUT PGENERATE_NAME_CONTEXT Context
                           );


#endif //__DIRCTRL_H__