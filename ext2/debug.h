#ifndef __DEBUG_H__
#define __DEBUG_H__

//#define LOG_TO_FILE

#define KD_LEV1     0x01
#define KD_LEV2     0x02
#define KD_CUR_LEV  KD_LEV1

#ifdef _DEBUG
#define KdHeader(l, h) l <= KD_CUR_LEV ? KdPrint(("[ext2r.sys] - "h"\n")) : 0;
#define KdInfo(i) KdPrint(("[info] - "i"\n"));
#else
#define KdHeader(l, h)
#define KdInfo(i)
#endif

#define EXT2_BUGCHECK   0x00000AAA
#define BUGCHECK_STRUCT 0x00100000

#define Ext2BugCheck(Code, Par1, Par2, Par3) \
    KeBugCheckEx(EXT2_BUGCHECK, Code + __LINE__, Par1, Par2, Par3);

#ifdef _DEBUG

VOID
Ext2LogToFile(
              IN PCHAR Format,
              ...
              );

#ifdef LOG_TO_FILE
#undef KdPrint
#define KdPrint(A) Ext2LogToFile A
#endif

#endif // #ifdef _DEBUG

#endif //__DEBUG_H__