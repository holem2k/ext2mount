#ifndef __MOUNT_H__
#define __MOUNT_H__

const WCHAR MountLinkName[]	= L"\\DosDevices\\Mount";
const WCHAR MountDevName[]	= L"\\Device\\Mount";

NTSTATUS
DriverEntry(
			IN PDRIVER_OBJECT  DriverObject,
			IN PUNICODE_STRING RegistryPath
			);

NTSTATUS
MountDeviceIoctl(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
MountCreate(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
MountRead(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
MountClose(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

VOID
MountUnload(
    IN PDRIVER_OBJECT DriverObject
    );



typedef struct _FCB
{
	FSRTL_COMMON_FCB_HEADER         CommonFCBHeader;
	SECTION_OBJECT_POINTERS         SectionObject;
	ERESOURCE                       MainResource;
	ERESOURCE                       PagingIoResource;
} FCB;
 
#endif //__MOUNT_H__