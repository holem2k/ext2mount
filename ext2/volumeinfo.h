#ifndef __VOLUMEINFO_H__
#define __VOLUMEINFO_H__

extern "C"
NTSTATUS
Ext2QueryVolumeInformation(
                           PIRP_CONTEXT
                           );


extern "C"
NTSTATUS
Ext2FsVolumeInformation(
                        IN PVCB         Vcb,
                        PVOID           Buffer,
                        IN OUT PULONG   Length
                        );

extern "C"
NTSTATUS
Ext2FsSizeInformation(
                      IN PVCB         Vcb,
                      PVOID           Buffer,
                      IN OUT PULONG   Length
                      );


extern "C"
NTSTATUS
Ext2FsAttributeInformation(
                           IN PVCB         Vcb,
                           PVOID           Buffer,
                           IN OUT PULONG   Length
                           );


extern "C"
NTSTATUS
Ext2FsDeviceInformation(
                        IN PVCB         Vcb,
                        PVOID           Buffer,
                        IN OUT PULONG   Length
                        );

#endif //__VOLUMEINFO_H__