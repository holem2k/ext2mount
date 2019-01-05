#ifndef __VERSUP_H__
#define __VERSUP_H__


extern "C"
inline
BOOLEAN 
Ext2IsStatusRemovable(
                      PIRP_CONTEXT  IrpContext,
                      NTSTATUS      Status
                      );

extern "C"
NTSTATUS
Ext2ProcessRemovableStatus(
                           IN PIRP_CONTEXT IrpContext,
                           IN NTSTATUS     Status,
                           IN OUT PBOOLEAN Again
                           );

extern "C"
NTSTATUS
Ext2VerifyFcbOperation(
                       PIRP_CONTEXT  IrpContext
                       );
                       

extern "C"
NTSTATUS
Ext2VerifyVcb(
              PIRP_CONTEXT  IrpContext,
              PVCB          Vcb
              );

#endif //__VERSUP_H__