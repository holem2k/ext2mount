#ifndef __READ_H__
#define __READ_H__

extern "C"
NTSTATUS
Ext2Read(
         PIRP_CONTEXT    IrpContext
         );

// 

extern "C"
NTSTATUS
Ext2MdlRead(
            IN PFILE_OBJECT FileObject,
            IN PULONGLONG   Offset,
            IN ULONG        Length,
            IN OUT PIRP     Irp,
            OUT PULONG      BytesLocked
            );

extern "C"
VOID
Ext2MdlReadComplete(
                    IN PFILE_OBJECT    FileObject,
                    IN PIRP            Irp
                    );

extern "C"
NTSTATUS
Ext2NonCachedRead(
                  IN PIRP_CONTEXT  IrpContext,
                  IN PVCB          Vcb,
                  IN PFILE_OBJECT  FileObject,
                  IN CSHORT        Type,
                  IN PULONGLONG    Offset,
                  IN ULONG         Length,
                  IN PVOID         Buffer
                  );

#endif //__READ_H__