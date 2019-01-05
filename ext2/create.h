#ifndef __CREATE_H__
#define __CREATE_H__

extern "C"
NTSTATUS
Ext2Create(
           PIRP_CONTEXT    IrpContext
           );


extern "C"
NTSTATUS
Ext2CreateFileSystem(
                     PIRP_CONTEXT    IrpContext
                     );

extern "C"
NTSTATUS
Ext2CreateVolume(
                 PIRP_CONTEXT    IrpContext
                 );


extern "C"
NTSTATUS
Ext2CreateFile(
               PIRP_CONTEXT    IrpContext
               );

extern "C"
NTSTATUS
Ext2AllocateName(
                 IN PFILE_OBJECT    FileObject,
                 OUT PWSTR          *FileName,
                 OUT PBOOLEAN       TrailSlash
                 );

#endif //__CREATE_H__