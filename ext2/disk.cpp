#include "stdafx.h"
#include "disk.h"

/////////////////////////////////////////////////////////////////////////////
// Ext2ReadDeviceCompletionRoutine()
// Expected IRQL: DISPATCH, APC, PASSIVE
// Section: NONpaged.
// Cause page errors: NO

extern "C"
NTSTATUS
Ext2ReadDeviceCompletionRoutine(
                                IN PDEVICE_OBJECT  DeviceObject,
                                IN PIRP            Irp,
                                IN PVOID           Context
                                )
{
    ASSERT(Context);

    *Irp->UserIosb = Irp->IoStatus; // A - A - a !!!

    KeSetEvent((PKEVENT)Context, IO_NO_INCREMENT, FALSE);
    
    // disks use direct I/O
    while (Irp->MdlAddress)
    {
        PMDL NextMdl = Irp->MdlAddress->Next;
        MmUnlockPages(Irp->MdlAddress);
        IoFreeMdl(Irp->MdlAddress);
        Irp->MdlAddress = NextMdl;
    }
    IoFreeIrp(Irp);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2ReadDevice(...)
// Expected IRQL: PASSIVE, APC
// Section: NONpaged.
// Cause page errors: YES_IF(Buffer)


extern "C"
NTSTATUS
Ext2ReadDevice(
               IN PDEVICE_OBJECT    DeviceObject,
               IN ULONGLONG         Offset,
               IN ULONG             Length,
               IN BOOLEAN           OverrideVerify,
               IN OUT PVOID         Buffer
               )
{
    ASSERT(!(Offset&0xFF));
    ASSERT(!(Length&0xFF));
    
    KEVENT Event;
    KeInitializeEvent(&Event,
        NotificationEvent,
        FALSE);
    
    IO_STATUS_BLOCK IoStatus;
    
    PIRP Irp = IoBuildAsynchronousFsdRequest(IRP_MJ_READ,
        DeviceObject,
        Buffer,
        Length,
        (PLARGE_INTEGER)&Offset,
        &IoStatus);
    
    if (Irp == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;
    
    if (OverrideVerify)
    {
        PIO_STACK_LOCATION IrpStack = IoGetNextIrpStackLocation(Irp);
        SetFlag(IrpStack->Flags, SL_OVERRIDE_VERIFY_VOLUME);
    }
    
    IoSetCompletionRoutine(Irp,
        Ext2ReadDeviceCompletionRoutine,
        &Event,
        TRUE,
        TRUE,
        TRUE);
    
    
    NTSTATUS status = IoCallDriver(DeviceObject, Irp);
    if (status == STATUS_PENDING)
    {
        status = KeWaitForSingleObject(&Event,
            Executive,
            KernelMode,
            FALSE,
            NULL);
        
        if (NT_SUCCESS(status))
            status = IoStatus.Status;

        /////////
        //KdPrint(("Dev = %x, Off = %x, Len = %x, status = %X\n", DeviceObject, (ULONG)Offset,  Length, status));
        /////////
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2ReadBlock(...)
// Expected IRQL: APC, PASSIVE; 
// Section: nonpaged.
// Cause page errors: YES_IF(SuperBlock, Buffer)

extern "C"
NTSTATUS
Ext2ReadBlock(
              IN PDEVICE_OBJECT     DeviceObject,
              IN PEXT2_SUPERBLOCK   SuperBlock,
              IN ULONG              Block,
              IN ULONG              Count,
              IN OUT PVOID          Buffer
              )
{
    return Ext2ReadDevice(DeviceObject,
        (ULONGLONG)Block << SuperBlock->BlockSizeShift,
        SuperBlock->BlockSize * Count,
        FALSE,
        Buffer);
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2MultiReadBlockCompletionRoutine(
                                    IN PDEVICE_OBJECT  DeviceObject,
                                    IN PIRP            Irp,
                                    IN PVOID           Context
                                    )
{
    PPARALLEL_IO_BLOCK IoBlock = (PPARALLEL_IO_BLOCK)Context;

    if (!NT_SUCCESS(Irp->IoStatus.Status))
        InterlockedExchange(&IoBlock->Status, Irp->IoStatus.Status);

    IoFreeMdl(Irp->MdlAddress);
    IoFreeIrp(Irp);

    if (InterlockedDecrement(&IoBlock->PendingIrpCount) == 0)
        KeSetEvent(&IoBlock->Event, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2MultiRead(...)
// Expected IRQL: APC, PASSIVE; 
// Section: nonpaged.
// Cause page errors: YES_IF()

extern "C"
NTSTATUS
Ext2MultiReadBlock(
                   PIRP_CONTEXT  IrpContext,
                   ULONG         Delta,
                   PULONG        Block,
                   ULONG         BlockCount
                   )
{
    ASSERT(BlockCount <= MAX_PARALLEL_READ);

    if (BlockCount == 0)
        return STATUS_SUCCESS;

    PVCB Vcb = Ext2GetVcb(IrpContext);
    PIRP Irp[MAX_PARALLEL_READ];
    PMDL Mdl[MAX_PARALLEL_READ];

    for (ULONG i = 0; i < BlockCount; i++)
        Irp[i] = NULL, Mdl[i] = NULL;

    // create Irps and Mdls
    BOOLEAN Ok = TRUE;
    PMDL SourceMdl = IrpContext->Irp->MdlAddress;
    ULONG BlockSize = Vcb->SuperBlock.BlockSize;
    ULONG BlockSizeShift = Vcb->SuperBlock.BlockSizeShift;

    for (i = 0; i < BlockCount; i++)
    {
        Irp[i] = IoMakeAssociatedIrp(IrpContext->Irp, Vcb->TargetDevice->StackSize + 1);
        if (Irp[i] == NULL)
        {
            Ok = FALSE;
            break;
        }

        PVOID BlockVirtualAddress = BUFFER(IrpContext->Irp->UserBuffer,
            Delta + (i << Vcb->SuperBlock.BlockSizeShift));

        Mdl[i] = IoAllocateMdl(BlockVirtualAddress,
            Vcb->SuperBlock.BlockSize,
            FALSE,
            FALSE,
            Irp[i]);

        if (Mdl[i] == NULL)
        {
            Ok = FALSE;
            break;
        }

        IoBuildPartialMdl(SourceMdl, Mdl[i], BlockVirtualAddress,
            BlockSize);

        IoSetNextIrpStackLocation(Irp[i]);
        // set current stack location
        PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp[i]);
        IrpStack->MajorFunction = IRP_MJ_READ; // Foo
        IrpStack->Parameters.Read.ByteOffset.QuadPart = (ULONGLONG)Block[i] << BlockSizeShift;
        IrpStack->Parameters.Read.Length = BlockSize;

        IoSetCompletionRoutine(Irp[i], 
            Ext2MultiReadBlockCompletionRoutine,
            &IrpContext->IoBlock,
            TRUE, TRUE, TRUE);
        IrpStack = IoGetNextIrpStackLocation(Irp[i]);
        IrpStack->MajorFunction = IRP_MJ_READ; 
        IrpStack->Parameters.Read.ByteOffset.QuadPart = (ULONGLONG)Block[i] << BlockSizeShift;
        IrpStack->Parameters.Read.Length = BlockSize;
    }

    if (!Ok)
    {
        for (i = 0; i < BlockCount; i++)
        {
            if (Mdl[i])
                IoFreeMdl(Mdl[i]);

            if (Irp[i])
                IoFreeIrp(Irp[i]);
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IrpContext->Irp->AssociatedIrp.IrpCount = 0;

    IrpContext->IoBlock.Status = STATUS_SUCCESS; // assume success
    IrpContext->IoBlock.PendingIrpCount = BlockCount;
    KeInitializeEvent(&IrpContext->IoBlock.Event,
        NotificationEvent, FALSE);

    for (i = 0; i < BlockCount; i++)
        IoCallDriver(Vcb->TargetDevice, Irp[i]);

    KeWaitForSingleObject(&IrpContext->IoBlock.Event, Executive,
        KernelMode, FALSE, NULL);

    return IrpContext->IoBlock.Status;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2PingDevice(...)
// Expected IRQL: PASSIVE; 
// Section: nonpaged.
// Cause page errors: YES_IF(MediaChangeCount)
// REM: MediaChangeCount is valid if STATUS_SUCCESS was returned;

extern "C"
NTSTATUS
Ext2PingDevice(
               IN PDEVICE_OBJECT    DeviceObject,
               IN BOOLEAN           OverrideVerify,
               OUT PULONG           MediaChangeCount
               )
{
    ULONG ChangeCount = 0;
    IO_STATUS_BLOCK IoStatus;

    KEVENT Event;
    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    PIRP Irp = IoBuildDeviceIoControlRequest(IOCTL_STORAGE_CHECK_VERIFY,
        DeviceObject,
        NULL,
        0,
        &ChangeCount,
        sizeof(ChangeCount),
        FALSE,
        &Event,
        &IoStatus);

    NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
    if (Irp)
    {
        if (OverrideVerify)
        {
            SetFlag(IoGetNextIrpStackLocation(Irp)->Flags, 
                SL_OVERRIDE_VERIFY_VOLUME);
        }

        status = IoCallDriver(DeviceObject, Irp);
        if (status == STATUS_PENDING)
        {
            KeWaitForSingleObject(&Event,
                Executive,
                KernelMode,
                FALSE,
                NULL);
            status = IoStatus.Status;
        }
        if (NT_SUCCESS(status))
        {
            *MediaChangeCount = IoStatus.Information == sizeof(ULONG) ? ChangeCount : 0;
        }
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2ContReadBlockCompletionRoutine(
                                   IN PDEVICE_OBJECT  DeviceObject,
                                   IN PIRP            Irp,
                                   IN PVOID           Context
                                   )
{
    ASSERT(Context);

    PKEVENT Event = (PKEVENT)Context;
    KeSetEvent(Event, 0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2ContReadBlock(...)
// Expected IRQL: PASSIVE, APC; 
// Section: nonpaged.

extern "C"
NTSTATUS
Ext2ContReadBlock(
                  IN PIRP_CONTEXT     IrpContext,
                  IN PULONGLONG       Offset,
                  IN ULONG            Length
                  )
{
    PIRP Irp = IrpContext->Irp;
    PIO_STACK_LOCATION IrpStack = IoGetNextIrpStackLocation(Irp);
    IrpStack->MajorFunction = IRP_MJ_READ;
    IrpStack->Parameters.Read.ByteOffset.QuadPart = *Offset;
    IrpStack->Parameters.Read.Length = Length;

    KEVENT Event;
    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(Irp, Ext2ContReadBlockCompletionRoutine,
        &Event, TRUE, TRUE, TRUE);

    PVCB Vcb = Ext2GetVcb(IrpContext);
    NTSTATUS status = IoCallDriver(Vcb->TargetDevice, Irp);
    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(&Event,
            Executive,
            KernelMode,
            FALSE,
            NULL);

        status = Irp->IoStatus.Status;
    }
    
    return status;
}

/////////////////////////////////////////////////////////////////////////////
