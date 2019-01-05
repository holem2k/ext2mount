// MountGUI.h: interface for the CMountGUI class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOUNTGUI_H__91EFF10A_9072_4D2C_95F6_B8CFFD01DE37__INCLUDED_)
#define AFX_MOUNTGUI_H__91EFF10A_9072_4D2C_95F6_B8CFFD01DE37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MountMan.h"

class CMountGUI  
{
public:
	CMountGUI(CCommonMountMan *pMountMan);
	virtual ~CMountGUI();
	void Run();
protected:
	CCommonMountMan *m_pMountMan;
};

#endif // !defined(AFX_MOUNTGUI_H__91EFF10A_9072_4D2C_95F6_B8CFFD01DE37__INCLUDED_)
