// StatusBar.h: interface for the CStatusBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STATUSBAR_H__91615E98_16D1_4CC6_BCA6_8C2B7A8A7CCD__INCLUDED_)
#define AFX_STATUSBAR_H__91615E98_16D1_4CC6_BCA6_8C2B7A8A7CCD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CStatusBar : public CWindow
{
public:
	CStatusBar();
	virtual ~CStatusBar();
	BOOL Create(CWindow *pParent);
	void SetSimpleText(LPCTSTR lpText);
};

#endif // !defined(AFX_STATUSBAR_H__91615E98_16D1_4CC6_BCA6_8C2B7A8A7CCD__INCLUDED_)
