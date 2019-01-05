// Obj.h: interface for the CObj class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OBJ_H__5AF3A7A3_E2B8_43F3_BD3B_4F09FD9857F0__INCLUDED_)
#define AFX_OBJ_H__5AF3A7A3_E2B8_43F3_BD3B_4F09FD9857F0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef unsigned int ERESULT;

const ERESULT E_OK			 = 0x00000000;
const ERESULT E_INVALID_ARG  = 0x00000001;

class CObj  
{
public:
	CObj();
	virtual ~CObj();
	const TCHAR *GetErrorString();
	void ClearError();
protected:
	ERESULT m_er;
};

#define RETURN_ERR(er)  { m_er = er; return FALSE; };
#define LEAVE_ERR(er)  { m_er = er; __leave; };
#define RETURN_RESULT { return m_er == E_OK; };

#endif // !defined(AFX_OBJ_H__5AF3A7A3_E2B8_43F3_BD3B_4F09FD9857F0__INCLUDED_)
