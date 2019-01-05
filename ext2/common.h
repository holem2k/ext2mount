#ifndef __COMMON_H__
#define __COMMON_H__

enum CODE_PAGE {CP_CP1251, CP_KOI8R, CP_INVALID};

const WCHAR DRIVER_NAME[]             = L"ext2r";
const WCHAR PARAM_SUBKEY_NAME[]       = L"Parameters";
const WCHAR PARAMNAME_CODE_PAGE[]     = L"CodePage";
const WCHAR PARAMNAME_RESOLVE_LINKS[] = L"ResolveLinks";

#endif //__COMMON_H__