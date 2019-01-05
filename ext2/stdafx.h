#ifndef __STDAFX_H__
#define __STDAFX_H__

extern "C"
{
	#include <ntifs.h>
    #include <ntddstor.h>
}

#include "ext2fs.h"
#include "struct.h"
#include "helpers.h"
#include "debug.h"
#include "common.h"

extern EXT2_GLOBAL_DATA Ext2GlobalData;


#endif //__STDAFX_H__