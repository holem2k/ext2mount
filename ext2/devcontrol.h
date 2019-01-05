#ifndef __DEVCONTROL_H__
#define __DEVCONTROL_H__

extern "C"
NTSTATUS
Ext2DeviceControl(
                  PIRP_CONTEXT IrpContext
                  );

#endif //__DEVCONTROL_H__