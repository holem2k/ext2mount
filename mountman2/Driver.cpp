// Driver.cpp: implementation of the CDriver class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Driver.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDriver::CDriver(LPCTSTR lpDriver, LPCTSTR lpDriverSys)
{
	m_bLoaded = FALSE;
	m_hDriver = INVALID_HANDLE_VALUE;
	lstrcpyn(m_szDriverName, lpDriver, SERVICE_MAX_NAME);
	lstrcpyn(m_szDriverSys, lpDriverSys, SERVICE_MAX_NAME);
}

//////////////////////////////////////////////////////////////////////

CDriver::~CDriver()
{

}

//////////////////////////////////////////////////////////////////////

BOOL CDriver::Load()
{
	m_er = E_OK;

	SC_HANDLE hSCManager;

	__try
	{
		hSCManager = ::OpenSCManager(NULL,
			NULL,
			SC_MANAGER_ALL_ACCESS);
		
		if (hSCManager == NULL)
			LEAVE_ERR(GetLastError() == ERROR_ACCESS_DENIED ? E_ACCESS_DENIED : E_SCM);

		if (!InstallDriver(hSCManager, m_szDriverName, m_szDriverSys))
			LEAVE_ERR(E_DRIVER_INSTALL);

		if (!StartDriver(hSCManager, m_szDriverName))
			LEAVE_ERR(E_DRIVER_START);

		m_bLoaded = TRUE;
	}
	__finally
	{
		if (m_er == E_DRIVER_START)
			UninstallDriver(hSCManager, m_szDriverName);

		if (hSCManager != NULL)
			::CloseServiceHandle(hSCManager);
	}

	RETURN_RESULT;
}

//////////////////////////////////////////////////////////////////////

BOOL CDriver::Unload()
{
	m_er = E_OK;

	SC_HANDLE hSCManager;

	__try
	{
		hSCManager = ::OpenSCManager(NULL,
			NULL,
			SC_MANAGER_ALL_ACCESS);

		if (hSCManager == NULL)
			LEAVE_ERR(GetLastError() == ERROR_ACCESS_DENIED ? E_ACCESS_DENIED : E_SCM);

		if (!StopDriver(hSCManager, m_szDriverName))
			LEAVE_ERR(E_DRIVER_STOP);

		if (!UninstallDriver(hSCManager, m_szDriverName))
			LEAVE_ERR(E_DRIVER_UNINSTALL);

		m_bLoaded = FALSE;
	}
	__finally
	{
		if (hSCManager != NULL)
			::CloseServiceHandle(hSCManager);
	}

	RETURN_RESULT;
}

//////////////////////////////////////////////////////////////////////

BOOL CDriver::Open()
{
	TCHAR szDriverName[SERVICE_MAX_NAME + 4];
	wsprintf(szDriverName, "\\\\.\\%s", m_szDriverName);

	m_hDriver = ::CreateFile(szDriverName, 
		0,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	return m_hDriver != INVALID_HANDLE_VALUE;
}

//////////////////////////////////////////////////////////////////////

BOOL CDriver::Close()
{
	return ::CloseHandle(m_hDriver);
}

//////////////////////////////////////////////////////////////////////

BOOL CDriver::DeviceControl(DWORD dwIoControlCode,       
		LPVOID lpInBuffer,           
		DWORD nInBufferSize,         
		LPVOID lpOutBuffer,          
		DWORD nOutBufferSize,        
		LPDWORD lpBytesReturned,     
		LPOVERLAPPED lpOverlapped)
{
	return DeviceIoControl(m_hDriver,
		dwIoControlCode,       
		lpInBuffer,           
		nInBufferSize,         
		lpOutBuffer,          
		nOutBufferSize,        
		lpBytesReturned,     
		lpOverlapped);
}

//////////////////////////////////////////////////////////////////////

CDriver::operator HANDLE()
{
	return m_hDriver;
}

//////////////////////////////////////////////////////////////////////

const TCHAR *CDriver::GetErrorString()
{
	const TCHAR *lpError;
	switch (m_er)
	{
	case E_SCM: lpError = _T("Ошибка работы с SCM."); break;
	case E_DRIVER_INSTALL: lpError = _T("Ошибка установки драйвера."); break;
	case E_DRIVER_START: lpError = _T("Ошибка загрузки драйвера."); break;
	case E_ACCESS_DENIED: lpError = _T("Недостаточно прав для выполнения операции."); break;
	default:
		return CObj::GetErrorString();
	}
	return lpError;
}

//////////////////////////////////////////////////////////////////////

BOOL CDriver::InstallDriver(SC_HANDLE hSCManager, LPCTSTR lpDriverName,
							LPCTSTR lpDriverSys)
{
	BOOL bResult = FALSE;
	
	SC_HANDLE hService = ::CreateService(hSCManager,
		lpDriverName,
		lpDriverName,
		SERVICE_ALL_ACCESS,		
		SERVICE_KERNEL_DRIVER,	
		SERVICE_DEMAND_START,	
		SERVICE_ERROR_NORMAL,	
		lpDriverSys,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);
	
	if (hService != NULL ||
		(hService == NULL && ::GetLastError() == ERROR_SERVICE_EXISTS))
	{
		bResult = TRUE;
		::CloseServiceHandle(hService);
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////

BOOL CDriver::UninstallDriver(SC_HANDLE hSCManager, LPCTSTR lpDriverName)
{
	BOOL bResult = FALSE;
	SC_HANDLE hService = ::OpenService(hSCManager, lpDriverName, SERVICE_ALL_ACCESS);
	if (hService != NULL)
	{
		bResult = ::DeleteService(hService);
		::CloseServiceHandle(hService);
	}
	return bResult;
}

//////////////////////////////////////////////////////////////////////

BOOL CDriver::StartDriver(SC_HANDLE hSCManager, LPCTSTR lpDriverName)
{
	BOOL bResult = FALSE;
	SC_HANDLE hService = ::OpenService(hSCManager, lpDriverName, SERVICE_ALL_ACCESS);
	if (hService != NULL)
	{
		bResult = ::StartService(hService, 0, NULL) ||
			GetLastError() == ERROR_SERVICE_ALREADY_RUNNING;

		::CloseServiceHandle(hService);
	}
	return bResult;
}

//////////////////////////////////////////////////////////////////////

BOOL CDriver::StopDriver(SC_HANDLE hSCManager, LPCTSTR lpDriverName)
{
	BOOL bResult = FALSE;
	SC_HANDLE hService = ::OpenService(hSCManager, lpDriverName, SERVICE_ALL_ACCESS);
	if (hService != NULL)
	{
		SERVICE_STATUS status;
		bResult = ::ControlService(hService, SERVICE_CONTROL_STOP, &status);
		::CloseServiceHandle(hService);
	}
	return bResult;
}

//////////////////////////////////////////////////////////////////////

