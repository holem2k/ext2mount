#ifndef __DISPATCH_H__
#define __DISPATCH_H__


extern "C"
NTSTATUS
Ext2CommonDispatch(
			 PDEVICE_OBJECT DeviceObject,
			 PIRP Irp
			 );

extern "C"
NTSTATUS
Ext2Dispatch(
             PIRP_CONTEXT   IrpContext
			 );


extern "C"
VOID
Ext2Unload(
		   PDRIVER_OBJECT DeviceObject
           );

extern "C"
NTSTATUS
Ext2QueueIrp(
             PIRP_CONTEXT IrpContext
             );



#endif //__DISPATCH_H__