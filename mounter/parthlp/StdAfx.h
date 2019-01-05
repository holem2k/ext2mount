// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__5E7A247D_C55C_43AC_BFFF_0EDB269DD287__INCLUDED_)
#define AFX_STDAFX_H__5E7A247D_C55C_43AC_BFFF_0EDB269DD287__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _X86_

#ifdef __cplusplus
extern "C" // ntddk.h and undocnt.h are supposed to compile with C compiler
{
#endif

typedef long LONG_PTR, *PLONG_PTR;
typedef unsigned long ULONG_PTR, *PULONG_PTR;

#include <ntddk.h> // from Windows 2000 DDK
#include "UndocNT\undocnt.h" // corrected: commented  out some redefinitions (already defined in ntddk.h)

#ifdef __cplusplus
}
#endif

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__5E7A247D_C55C_43AC_BFFF_0EDB269DD287__INCLUDED_)
