#ifndef __EXCEPT_H__
#define __EXCEPT_H__

#include "stdafx.h"

NTSTATUS
Ext2ExceptionHandler(
                     PIRP_CONTEXT   IrpContext,
                     NTSTATUS       Status
                     );

int
Ext2ExceptionFilter(
                    PIRP_CONTEXT    IrpContext,
                    NTSTATUS        Status
                    );


#endif //__EXCEPT_H__