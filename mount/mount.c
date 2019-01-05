/*
	Module name:  mount.c
	Abstract: create symbolic links to ext2 partitions
*/

//#include <ntddk.h>
#include <ntifs.h>
#include "mount.h"
#include "ioctlcode.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, MountDeviceIoctl)
#pragma alloc_text (PAGE, MountClose)
#pragma alloc_text (PAGE, MountCreate)
#pragma alloc_text (PAGE, MountUnload)
#endif


const UINT FILE_SIZE1 = 0x2000;
const UINT FILE_SIZE2 = 0x2500;

PFILE_OBJECT pFile = NULL;
FCB header = {0};
ULONG c = 0;

PFILE_OBJECT pCopyFile = NULL;
FCB cheader = {0};


PFILE_OBJECT pFailFile = NULL;
FCB fheader = {0};


VOID *pPagedMem;
const ULONG cbPagedMem = 1024*1024*5;

void TryMakeReboot()
{
	ULONG i;

	DbgPrint("Rebooting\n");

	if (pPagedMem == NULL)
	{
		DbgPrint("Not enouth mem\n");
			return;
	}

	for (i = 0; i < cbPagedMem; i++)
		((BYTE *)pPagedMem)[i] = 'a';
	DbgPrint("Zeroed\n");
	
}

BOOLEAN
CdNoopAcquire(
    IN PVOID Fcb,
    IN BOOLEAN Wait
    )
{
    return TRUE;
}
void
CdNoopRelease(void *p
    )
{

}

NTSTATUS
DriverEntry(
			IN PDRIVER_OBJECT  DriverObject,
			IN PUNICODE_STRING RegistryPath
			)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT pDeviceObject = NULL;
	UNICODE_STRING uszLink, uszDevName;
	BOOLEAN bLink = FALSE;
	CACHE_MANAGER_CALLBACKS callbacks;
	CC_FILE_SIZES  sizes;
	
	DbgPrint("[mount.sys] -> DriverEntry()\n");
	
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MountDeviceIoctl;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = MountCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = MountClose;
	DriverObject->MajorFunction[IRP_MJ_READ] = MountRead;
	DriverObject->DriverUnload = MountUnload;
	
	__try
	{
		// create device
		RtlInitUnicodeString(&uszDevName, MountDevName);
		status = IoCreateDevice(DriverObject,
			0,
			&uszDevName,
			FILE_DEVICE_UNKNOWN,
			0,
			FALSE,
			&pDeviceObject);
//		pDeviceObject->Flags |= DO_BUFFERED_IO;
		
		if (!NT_SUCCESS(status))
		{
			DbgPrint("[mount.sys] - IoCreateDevice() failed.\n");
			__leave;
		}
		
		// create symbolic link to device
		RtlInitUnicodeString(&uszLink, MountLinkName);
		
		status = IoCreateSymbolicLink(&uszLink, &uszDevName);
		if (!NT_SUCCESS(status))
		{
			DbgPrint("[mount.sys] - IoCreateSymbolicLink() failed.\n");
			__leave;
		}
		else 
			bLink = TRUE;

		// 1st file		
		pFile = IoCreateStreamFileObject(NULL, pDeviceObject);
		if (pFile == NULL)
			__leave;
		pFile->ReadAccess = TRUE;
		pFile->WriteAccess = FALSE;
		pFile->DeleteAccess = FALSE;

		header.CommonFCBHeader.NodeTypeCode = 10;
		header.CommonFCBHeader.NodeByteSize = sizeof(header);
		header.CommonFCBHeader.IsFastIoPossible = 0;
		header.CommonFCBHeader.FileSize.QuadPart = FILE_SIZE1;
		header.CommonFCBHeader.ValidDataLength.QuadPart = FILE_SIZE1;
		header.CommonFCBHeader.AllocationSize.QuadPart = FILE_SIZE1;

		pFile->FsContext = &header;
		pFile->FsContext2 = NULL;                    
		pFile->SectionObjectPointer = &header.SectionObject;

		callbacks.AcquireForLazyWrite = CdNoopAcquire;
		callbacks.ReleaseFromLazyWrite = CdNoopRelease;
		callbacks.AcquireForReadAhead = CdNoopAcquire;
		callbacks.ReleaseFromReadAhead = CdNoopRelease;
	
		sizes.FileSize.QuadPart = FILE_SIZE1;
		sizes.ValidDataLength.QuadPart = FILE_SIZE1;
		sizes.AllocationSize.QuadPart = FILE_SIZE1;

		CcInitializeCacheMap(pFile, &sizes, TRUE, &callbacks, &header);
		CcSetAdditionalCacheAttributes(pFile, TRUE, TRUE);

		// 2nd file		
		pCopyFile = IoCreateStreamFileObject(NULL, pDeviceObject);
		if (pCopyFile == NULL)
			__leave;
		pCopyFile->ReadAccess = TRUE;
		pCopyFile->WriteAccess = FALSE;
		pCopyFile->DeleteAccess = FALSE;

		cheader.CommonFCBHeader.NodeTypeCode = 10;
		cheader.CommonFCBHeader.NodeByteSize = sizeof(header);
		cheader.CommonFCBHeader.IsFastIoPossible = 0;
		cheader.CommonFCBHeader.FileSize.QuadPart = FILE_SIZE2;
		cheader.CommonFCBHeader.ValidDataLength.QuadPart = FILE_SIZE2;
		cheader.CommonFCBHeader.AllocationSize.QuadPart = FILE_SIZE2;

		pCopyFile->FsContext = &header;
		pCopyFile->FsContext2 = NULL;                    
		pCopyFile->SectionObjectPointer = &cheader.SectionObject;

		callbacks.AcquireForLazyWrite = CdNoopAcquire;
		callbacks.ReleaseFromLazyWrite = CdNoopRelease;
		callbacks.AcquireForReadAhead = CdNoopAcquire;
		callbacks.ReleaseFromReadAhead = CdNoopRelease;
	
		sizes.FileSize.QuadPart = FILE_SIZE2;
		sizes.ValidDataLength.QuadPart = FILE_SIZE2;
		sizes.AllocationSize.QuadPart = FILE_SIZE2;

		CcInitializeCacheMap(pCopyFile, &sizes, FALSE, &callbacks, &cheader);
		CcSetAdditionalCacheAttributes(pCopyFile, TRUE, TRUE);

		// alloc memory
		pPagedMem = ExAllocatePool(PagedPool, cbPagedMem);

		
	}
	__finally
	{
		if (!NT_SUCCESS(status))
		{
			// delete symbolic link
			if (bLink)
				IoDeleteSymbolicLink(&uszLink);
			
			// delete device;
			if (pDeviceObject != NULL)
				IoDeleteDevice(pDeviceObject);
		}
	}
	return status;
}


NTSTATUS
MountDeviceIoctl(
				 IN  PDEVICE_OBJECT  DeviceObject,
				 IN  PIRP            Irp
				 )
{
	NTSTATUS status = STATUS_SUCCESS;
	PVOID Address = ExAllocatePool(NonPagedPool, 10000);
	PMDL Mdl = IoAllocateMdl(Address, 10000, FALSE, FALSE, NULL);
	MmProbeAndLockPages(Mdl, KernelMode, IoReadAccess);
	MmUnlockPages(Mdl);
	IoFreeMdl(Mdl);
	return status;

/*
	PIO_STACK_LOCATION pStack;
	ULONG cbDataIn;
	ULONG uParam, uDrive, uPartition, uLetter;
	UNICODE_STRING uszLink, uszDevice;
	BYTE *Buf;
	IO_STATUS_BLOCK sb;
	ULONG i;
	LARGE_INTEGER t;

	LARGE_INTEGER liOffset;
	VOID *pBCB, *pBuffer;

	PAGED_CODE();

	Buf = ExAllocatePool(PagedPool, 0x1000);
	ASSERT(Buf);
	
	DbgPrint("[mount.sys] -> MountDeviceIoctl()\n");
	pStack = IoGetCurrentIrpStackLocation(Irp);


	t.QuadPart = 10000000*40;
	status = KeDelayExecutionThread(KernelMode, FALSE, &t);

	DbgPrint("[mount.sys] -> After pause\n");

	liOffset.QuadPart = 0x100;
	if (CcCopyRead(pCopyFile, &liOffset, 0x200, TRUE, Buf, &sb))
	{
		DbgPrint("Data copied\n");
		for (i = 0; i < 0x200; i++)
		  DbgPrint("%03u = %c", i, ((BYTE *)Buf)[i]);
	}	
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
*/
}

NTSTATUS
MountCreate(
			IN  PDEVICE_OBJECT  DeviceObject,
			IN  PIRP            Irp
			)
{
	NTSTATUS status = STATUS_SUCCESS;
	PAGED_CODE();
	DbgPrint("[mount.sys] -> MountCreate()\n");
	
	// simply complete request
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS
MountRead(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pStack;
	BOOLEAN bPageIO;
	void *pBuffer;
	ULONG Length, Offset;
	LARGE_INTEGER liOffset;
	VOID *pBCB, *pBuffer2;
	ULONG i;
	LARGE_INTEGER t;

	DbgPrint("[mount.sys] -> MountRead()\n");

	InterlockedIncrement(&c);

	pStack = IoGetCurrentIrpStackLocation(Irp);
	pBuffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);

	bPageIO = Irp->Flags & IRP_PAGING_IO ?  TRUE : FALSE;
	Length = pStack->Parameters.Read.Length;
	Offset = pStack->Parameters.Read.ByteOffset.LowPart;

	DbgPrint("pBuffer = %u, Length = %u, Offset = %u\n", (ULONG)pBuffer, Length, pStack->Parameters.Read.ByteOffset.LowPart);

	if (bPageIO)
	{
		if (pStack->FileObject == pCopyFile)
		{
			TryMakeReboot();
			liOffset.QuadPart = 0x2400;
			if (CcMapData(pFile, &liOffset, 0x200, TRUE,  &pBCB, &pBuffer2))
			{
				DbgPrint("Data mapped\n");			
				RtlFillMemory(pBuffer, 0x300, '2');

				for (i = 0; i < 0x200; i++)
				  ((BYTE *)pBuffer)[i] = ((BYTE *)pBuffer)[i] + ((BYTE *)pBuffer2)[i];

				CcUnpinData(pBCB);
			}
			Irp->IoStatus.Status = STATUS_SUCCESS;
			Irp->IoStatus.Information = 0x1000;
			IoCompleteRequest(Irp, IO_NO_INCREMENT);
		}
		else
		{
			TryMakeReboot();
			// 2nd page fault handler
			if (Offset >= FILE_SIZE2)
			{
				Irp->IoStatus.Status = STATUS_END_OF_FILE;
				Irp->IoStatus.Information = 0x0;
				IoCompleteRequest(Irp, IO_NO_INCREMENT);
			}
			else
			{
				RtlFillMemory(pBuffer, 0x600, '1');
				Irp->IoStatus.Status = STATUS_SUCCESS;
				Irp->IoStatus.Information = 0x600;
				IoCompleteRequest(Irp, IO_NO_INCREMENT);
			}
		}

	}
	InterlockedDecrement(&c);
	return status;
}

NTSTATUS
MountClose(
		   IN  PDEVICE_OBJECT  DeviceObject,
		   IN  PIRP            Irp
		   )
{
	NTSTATUS status = STATUS_SUCCESS;
	PAGED_CODE();
	DbgPrint("[mount.sys] -> MountClose()\n");
	
	// simply complete request
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}


VOID
MountUnload(
			IN PDRIVER_OBJECT DriverObject
 			)
{
	UNICODE_STRING link;
	PAGED_CODE();
	DbgPrint("[mount.sys] -> MountUnload()\n");
	
	// delete symbolic link
	RtlInitUnicodeString(&link, MountLinkName);
	IoDeleteSymbolicLink(&link);
	
	// delete device
	IoDeleteDevice(DriverObject->DeviceObject);
}



