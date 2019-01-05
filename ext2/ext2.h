#ifndef __EXT2_H__
#define __EXT2_H__

extern "C"
NTSTATUS
DriverEntry(
			IN PDRIVER_OBJECT  DriverObject,
			IN PUNICODE_STRING RegistryPath
			);

extern "C"
VOID 
Ext2InitFastIoDispatch(
                       PFAST_IO_DISPATCH FastDispatch
                       );

#endif //__EXT2_H__