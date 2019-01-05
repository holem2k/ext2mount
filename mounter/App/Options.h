#ifndef __OPTIONS_H__
#define __OPTIONS_H__
#include <string>
#include <map>
using namespace std;

#ifdef _UNICODE
typedef wstring String;
#else
typedef string String;
#endif

////////////////////////////////////////////////////////////////////////

class CStorage
{
public:
    CStorage(const TCHAR *lpStorageName);
    virtual BOOL Open() = 0;
    virtual void Close() = 0;
    virtual BOOL LoadNumValue(const TCHAR *lpValueName, DWORD *dwValue) = 0;
    virtual BOOL SaveNumValue(const TCHAR *lpValueName, DWORD dwValue) = 0;
protected:
    String m_strName;
};

////////////////////////////////////////////////////////////////////////

class CRegStorage : public CStorage
{
public:
    CRegStorage(const TCHAR *lpStorageName);
    ~CRegStorage();
    virtual BOOL Open();
    virtual void Close();
    virtual BOOL LoadNumValue(const TCHAR *lpValueName, DWORD *dwValue);
    virtual BOOL SaveNumValue(const TCHAR *lpValueName, DWORD dwValue);
protected:
    HKEY m_hKey;
};

////////////////////////////////////////////////////////////////////////

class COptionValue
{
public:
    COptionValue();
    COptionValue(int uValue);
    COptionValue(const TCHAR *strValue);
    COptionValue& operator=(const COptionValue& Value);
    operator unsigned int();
    operator String();
    typedef enum {NUM_TYPE, STR_TYPE} _OptionType;
    _OptionType Type() { return m_eType; }
protected:
    int m_uValue;
    String m_strValue;
    _OptionType m_eType;
};

////////////////////////////////////////////////////////////////////////

class COptions
{
public:
    COptions() {};
    COptionValue& operator[](const TCHAR *strOption);
    BOOL Save(CStorage *pStorage);
    BOOL Load(CStorage *pStorage);
protected:
    typedef map<String, COptionValue> _OptionMap;
    _OptionMap m_map;
};

////////////////////////////////////////////////////////////////////////

#endif //__OPTIONS_H__