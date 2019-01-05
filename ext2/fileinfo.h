#ifndef __FILEINFO_H__
#define __FILEINFO_H__

extern "C"
NTSTATUS
Ext2QueryInformation(
                     PIRP_CONTEXT   IrpContext
                     );

extern "C"
NTSTATUS
Ext2QueryBasicInformation(
                          PFCB          Fcb,
                          PFILE_OBJECT  FileObject,
                          PVOID         Buffer,
                          PULONG        Length
                          );

extern "C"
NTSTATUS
Ext2StandardInformation(
                        PFCB          Fcb,
                        PFILE_OBJECT  FileObject,
                        PVOID         Buffer,
                        PULONG        Length
                        );

extern "C"
NTSTATUS
Ext2QueryPositionInformation(
                            PFCB          Fcb,
                            PFILE_OBJECT  FileObject,
                            PVOID         Buffer,
                            PULONG        Length
                            );

extern "C"
NTSTATUS
Ext2QueryNameInformation(
                         PFCB          Fcb,
                         PFILE_OBJECT  FileObject,
                         PVOID         Buffer,
                         PULONG        Length
                         );

extern "C"
NTSTATUS
Ext2QueryEaInformation(
                       PFCB          Fcb,
                       PFILE_OBJECT  FileObject,
                       PVOID         Buffer,
                       PULONG        Length
                       );

extern "C"
NTSTATUS
Ext2SetInformation(
                   PIRP_CONTEXT   IrpContext
                   );


#endif //__FILEINFO_H__