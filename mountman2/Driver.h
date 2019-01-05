// Driver.h: interface for the CDriver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRIVER_H__EDF058B2_5476_49E2_9AD3_F4E3ADAF04FE__INCLUDED_)
#define AFX_DRIVER_H__EDF058B2_5476_49E2_9AD3_F4E3ADAF04FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Obj.h"

const ERESULT E_SCM				 = 0x00000100;
const ERESULT E_DRIVER_INSTALL	 = 0x00000101;
const ERESULT E_DRIVER_UNINSTALL = 0x00000102;
const ERESULT E_DRIVER_START	 = 0x00000103;
const ERESULT E_DRIVER_STOP		 = 0x00000104;
const ERESULT E_ACCESS_DENIED	 = 0x00000105;

const UINT SERVICE_MAX_NAME = 257;

class CDriver : CObj
{
public:
	CDriver(LPCTSTR lpDriver, LPCTSTR lpDriverSys);
	virtual ~CDriver();

	BOOL Load();
	BOOL Unload();

	BOOL Open();
	BOOL Close();

	BOOL DeviceControl(DWORD dwIoControlCode,       
		LPVOID lpInBuffer,           
		DWORD nInBufferSize,         
		LPVOID lpOutBuffer,          
		DWORD nOutBufferSize,        
		LPDWORD lpBytesReturned,     
		LPOVERLAPPED lpOverlapped);

	operator HANDLE();
	const TCHAR *GetErrorString();
protected:
	BOOL InstallDriver(SC_HANDLE hSCManager, LPCTSTR lpDriverName, LPCTSTR lpDriverSys);
	BOOL UninstallDriver(SC_HANDLE hSCManager, LPCTSTR lpDriverName);
	BOOL StartDriver(SC_HANDLE hSCManager, LPCTSTR lpDriverName);
	BOOL StopDriver(SC_HANDLE hSCManager, LPCTSTR lpDriverName);

	BOOL m_bLoaded;
	HANDLE m_hDriver;
	TCHAR m_szDriverName[SERVICE_MAX_NAME];
	TCHAR m_szDriverSys[SERVICE_MAX_NAME];
};

#endif // !defined(AFX_DRIVER_H__EDF058B2_5476_49E2_9AD3_F4E3ADAF04FE__INCLUDED_)
