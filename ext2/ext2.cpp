#include "stdafx.h"
#include "ext2.h"
#include "struct.h"
#include "dispatch.h"
#include "fastio.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#endif


EXT2_GLOBAL_DATA Ext2GlobalData;
FAST_IO_DISPATCH Ext2FastIoDispatch;

extern "C"
NTSTATUS
DriverEntry(
            IN PDRIVER_OBJECT  DriverObject,
            IN PUNICODE_STRING RegistryPath
            )
{
    KdHeader(KD_LEV1, "DriverEntry");
    
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_OBJECT Device = NULL;
    
    __try
    {
        UNICODE_STRING DevName;
        RtlInitUnicodeString(&DevName, L"\\ext2r");
        status = IoCreateDevice(DriverObject,
            0,
            &DevName,
            FILE_DEVICE_DISK_FILE_SYSTEM,
            0,
            FALSE,
            &Device);
        
        if (!NT_SUCCESS(status))
            __leave;
        
        DriverObject->MajorFunction[IRP_MJ_CREATE] = 
            DriverObject->MajorFunction[IRP_MJ_CLEANUP] = 
            DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = 
            DriverObject->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION] = 
            DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] = 
            DriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL] =
            DriverObject->MajorFunction[IRP_MJ_CLOSE] =
            DriverObject->MajorFunction[IRP_MJ_READ] = 
            DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = Ext2CommonDispatch;
        DriverObject->DriverUnload = NULL;

        Ext2InitGlobalData(&Ext2GlobalData, Device, DriverObject, RegistryPath);
        Ext2InitFastIoDispatch(&Ext2FastIoDispatch);
        DriverObject->FastIoDispatch = &Ext2FastIoDispatch;
        
        IoRegisterFileSystem(Device);
    }
    __finally
    {
        if (NT_ERROR(status))	
        {
            KdPrint(("[ext2.sys] - failed to load driver.\n"));
            
            if (Device != NULL)
            {
                IoUnregisterFileSystem(Device);
                IoDeleteDevice(Device);
            }
        }
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID 
Ext2InitFastIoDispatch(
                       PFAST_IO_DISPATCH FastDispatch
                       )
{
    RtlZeroMemory(FastDispatch, sizeof(FAST_IO_DISPATCH));
    FastDispatch->SizeOfFastIoDispatch = sizeof(FAST_IO_DISPATCH);
    FastDispatch->FastIoCheckIfPossible = Ext2FastIoCheckIfPossible;
    FastDispatch->FastIoRead = FsRtlCopyRead;
    FastDispatch->AcquireFileForNtCreateSection = Ext2AcquireFileForNtCreateSection;
    FastDispatch->ReleaseFileForNtCreateSection = Ext2ReleaseFileFromNtCreateSection;  
    FastDispatch->FastIoQueryBasicInfo    = Ext2FastQueryBasicInfo;     
    FastDispatch->FastIoQueryStandardInfo = Ext2FastQueryStdInfo;       
    FastDispatch->FastIoLock              = Ext2FastLock;               
    FastDispatch->FastIoUnlockSingle      = Ext2FastUnlockSingle;       
    FastDispatch->FastIoUnlockAll         = Ext2FastUnlockAll;          
    FastDispatch->FastIoUnlockAllByKey    = Ext2FastUnlockAllByKey;     
    FastDispatch->FastIoQueryNetworkOpenInfo = Ext2FastQueryNetworkInfo;   
}

/////////////////////////////////////////////////////////////////////////////
