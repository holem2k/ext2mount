#include "stdafx.h"
#include "Options.h"

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CStorage::CStorage(const TCHAR *lpStorageName) : m_strName(lpStorageName)
{

}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CRegStorage::CRegStorage(const TCHAR *lpStorageName) : CStorage(lpStorageName)
{
    m_hKey = NULL;
}

////////////////////////////////////////////////////////////////////////

CRegStorage::~CRegStorage()
{
    Close();
}

////////////////////////////////////////////////////////////////////////

BOOL CRegStorage::Open()
{
    HKEY hKey;
    LONG Result = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, m_strName.c_str(),
        0, KEY_READ | KEY_WRITE, &hKey);

    if (Result == ERROR_SUCCESS)
    {
        m_hKey = hKey;
        return TRUE;
    }
    return FALSE;
}

////////////////////////////////////////////////////////////////////////

void CRegStorage::Close()
{
    if (m_hKey)
    {
        ::RegCloseKey(m_hKey);
        m_hKey = NULL;
    }
}

////////////////////////////////////////////////////////////////////////

BOOL CRegStorage::LoadNumValue(const TCHAR *lpValueName, DWORD *pdwValue)
{
    ATLASSERT(m_hKey);

    DWORD dwSize = sizeof(DWORD), dwType = REG_DWORD;
    LONG Result = ::RegQueryValueEx(m_hKey, lpValueName, 0, &dwType,
        (LPBYTE)pdwValue, &dwSize);

    return Result == ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////

BOOL CRegStorage::SaveNumValue(const TCHAR *lpValueName, DWORD dwValue)
{
    ATLASSERT(m_hKey);

    LONG Result = ::RegSetValueEx(m_hKey, lpValueName, 0, REG_DWORD,
        (LPBYTE)&dwValue, sizeof(DWORD));
    return Result == ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

COptionValue::COptionValue()
{
    m_uValue = 0;
    m_eType = NUM_TYPE;
}

////////////////////////////////////////////////////////////////////////

COptionValue::COptionValue(int uValue)
{
    m_uValue = uValue;
    m_eType = NUM_TYPE;
}

////////////////////////////////////////////////////////////////////////

COptionValue::COptionValue(const TCHAR *strValue)
{
    m_strValue = String(strValue);
    m_eType = STR_TYPE;
}

////////////////////////////////////////////////////////////////////////

COptionValue& COptionValue::operator=(const COptionValue& Value)
{
    m_eType = Value.m_eType;

    switch(m_eType)
    {
    case STR_TYPE:
        m_strValue = Value.m_strValue;
        break;
    case NUM_TYPE:
        m_uValue = Value.m_uValue;
        break;
    }
    return *this;
}

////////////////////////////////////////////////////////////////////////

COptionValue::operator unsigned int()
{
    if (m_eType == NUM_TYPE)
        return m_uValue;
    return 0;
}

////////////////////////////////////////////////////////////////////////

COptionValue::operator String()
{
    if (m_eType == STR_TYPE)
        return m_strValue;
    return _T("");
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

COptionValue& COptions::operator[](const TCHAR *strOption)
{
    return m_map[strOption];
}

////////////////////////////////////////////////////////////////////////

BOOL COptions::Save(CStorage *pStorage)
{
    BOOL bResult = FALSE;
    if (pStorage->Open())
    {
        for (_OptionMap::iterator i = m_map.begin(); i != m_map.end(); i++)
        {
            switch (i->second.Type())
            {
            case COptionValue::NUM_TYPE:
                bResult = pStorage->SaveNumValue(i->first.c_str(), i->second);
                break;
            case COptionValue::STR_TYPE:
                bResult = FALSE;
                break;
            }

            if (!bResult)
                break;
        }
        pStorage->Close();
    }
    return bResult;
}

////////////////////////////////////////////////////////////////////////

BOOL COptions::Load(CStorage *pStorage)
{
    BOOL bResult = FALSE;
    if (pStorage->Open())
    {
        for (_OptionMap::iterator i = m_map.begin(); i != m_map.end(); i++)
        {
            switch (i->second.Type())
            {
            case COptionValue::NUM_TYPE:
                {
                    DWORD dwValue;
                    BOOL bLoad = pStorage->LoadNumValue(i->first.c_str(), &dwValue);
                    if (bLoad)
                        i->second = dwValue;
                    else
                        bResult = FALSE;
                }
                break;

            case COptionValue::STR_TYPE:
                break;
            }
        }
        pStorage->Close();
    }
    return bResult;
}

////////////////////////////////////////////////////////////////////////
