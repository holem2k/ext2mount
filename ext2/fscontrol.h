#ifndef __FSCONTROL_H__
#define __FSCONTROL_H__

extern "C"
NTSTATUS
Ext2FileSystemControl(
                      PIRP_CONTEXT  IrpContext
                      );


extern "C"
NTSTATUS
Ext2Mount(
          PIRP_CONTEXT  IrpContext
          );


extern "C"
NTSTATUS
Ext2VerifyVolume(
                 PIRP_CONTEXT  IrpContext
                 );

extern "C"
NTSTATUS
Ext2UserFsRequest(
                  PIRP_CONTEXT  IrpContext
                  );

extern "C"
NTSTATUS
Ext2IsVolumeMounted(
                    PIRP_CONTEXT  IrpContext
                    );

extern "C"
NTSTATUS
Ext2LockVolume(
               PIRP_CONTEXT  IrpContext
               );

extern "C"
NTSTATUS
Ext2UnlockVolume(
                 PIRP_CONTEXT  IrpContext
                 );

extern "C"
NTSTATUS
Ext2DismountVolume(
                   PIRP_CONTEXT IrpContext
                   );

extern "C"
PVCB
Ext2TryRemount(
               IN PEXT2_SUPERBLOCK  SuperBlock,
               IN PDEVICE_OBJECT    RealDevice
               );

extern "C"
VOID
Ext2ScanForDismount(
                    );

#endif //__FSCONTROL_H__