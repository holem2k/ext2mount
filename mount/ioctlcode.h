#ifndef __IOCTLCODE_H__
#define __IOCTLCODE_H__
#include <windef.h>

#ifndef CTL_CODE
#pragme message ("CTL_CODE undefined. Include 'winioctl.h'")
#endif

#define LINK_PARAM(drive, partition, letter) \
               MAKELONG((WORD)letter, MAKEWORD((BYTE)drive, (BYTE)partition))

#define LINK_LETTER(param) LOWORD(param)
#define LINK_DRIVE(param)  LOBYTE(HIWORD((LONG)param))
#define LINK_PARTITION(param) HIBYTE(HIWORD((LONG)param))

#define IOCTL_CREATE_SYMBOLIC_LINK CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DELETE_SYMBOLIC_LINK CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif
