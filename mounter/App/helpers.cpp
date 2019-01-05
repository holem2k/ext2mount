#include "stdafx.h"
#include "herlpers.h"

//////////////////////////////////////////////////////////////////////
// CUnderAdmin
//////////////////////////////////////////////////////////////////////

BOOL CUnderAdmin::Check()
{
	BOOL bResult = FALSE;
	HANDLE hAccessToken = INVALID_HANDLE_VALUE;
	PSID psidAdministrators = NULL;

	__try 
	{
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hAccessToken))
			__leave;

		TOKEN_GROUPS *ptg;
		DWORD dwInfoLen;

		DWORD cbTokenGroups;
		if (GetTokenInformation(hAccessToken, TokenGroups, NULL, 0, &cbTokenGroups))
			__leave;
		else
		{
			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
				__leave;
		}

		ptg = (TOKEN_GROUPS *)_alloca(cbTokenGroups);
		if (!ptg)
			__leave;

		if (!GetTokenInformation(hAccessToken, TokenGroups, ptg, cbTokenGroups, &dwInfoLen))
			__leave;

		SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
		if (!AllocateAndInitializeSid(&siaNtAuthority, 2,
					SECURITY_BUILTIN_DOMAIN_RID,
					DOMAIN_ALIAS_RID_ADMINS,
					0, 0, 0, 0, 0, 0,
					&psidAdministrators))
		{
			__leave;
		}

		for(UINT i = 0; i < ptg->GroupCount; i ++)
		{
			if (EqualSid(psidAdministrators, ptg->Groups[i].Sid))
			{
				bResult = TRUE;
				break;
			}
		}
	}
	__finally
	{
		if (psidAdministrators)
			FreeSid(psidAdministrators);
		if (hAccessToken != INVALID_HANDLE_VALUE)
			CloseHandle(hAccessToken);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////
// CRunOnce
//////////////////////////////////////////////////////////////////////

CRunOnce::CRunOnce(LPCTSTR lpAppSign)
{
	m_hMutex = ::CreateMutex(NULL, TRUE, lpAppSign);
	m_bRunning = ::GetLastError() == ERROR_ALREADY_EXISTS;
	if (m_bRunning)
	{
		::CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
}

//////////////////////////////////////////////////////////////////////

CRunOnce::~CRunOnce()
{
	if (m_hMutex)
		::CloseHandle(m_hMutex);
}

//////////////////////////////////////////////////////////////////////

BOOL CRunOnce::IsRunning()
{
	return m_bRunning;
}

//////////////////////////////////////////////////////////////////////

void CRunOnce::Show(LPCTSTR lpWindowCaption)
{

}

//////////////////////////////////////////////////////////////////////

