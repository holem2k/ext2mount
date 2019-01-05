#ifndef __DISK_H__
#define __DISK_H__

const ULONG MAX_PARALLEL_READ = 5;

extern "C"
NTSTATUS
Ext2ReadDevice(
			   IN PDEVICE_OBJECT    DeviceObject,
			   IN ULONGLONG         Offset,
			   IN ULONG             Length,
			   IN BOOLEAN           OverrideVerify,
			   IN OUT PVOID         Buffer
			   );


extern "C"
NTSTATUS
Ext2ReadBlock(
			  IN PDEVICE_OBJECT     DeviceObject,
			  IN PEXT2_SUPERBLOCK   SuperBlock,
			  IN ULONG              Block,
              IN ULONG              Count,
			  IN OUT PVOID          Buffer
			  );

extern "C"
NTSTATUS
Ext2MultiReadBlock(
                   PIRP_CONTEXT  IrpContext,
                   ULONG         Delta,
                   PULONG        Block,
                   ULONG         BlockCount
                   );

extern "C"
NTSTATUS
Ext2PingDevice(
               IN PDEVICE_OBJECT    DeviceObject,
               IN BOOLEAN           OverrideVerify,
               OUT PULONG           MediaChangeCount
               );

extern "C"
NTSTATUS
Ext2ContReadBlock(
                  IN PIRP_CONTEXT     IrpContext,
                  IN PULONGLONG       Offset,
                  IN ULONG            Length
                  );

#endif //__DISK_H__