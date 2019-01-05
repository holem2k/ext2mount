// LetterDlg.h: interface for the LetterDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LETTERDLG_H__C242D399_83C9_4B94_B485_705CD900A097__INCLUDED_)
#define AFX_LETTERDLG_H__C242D399_83C9_4B94_B485_705CD900A097__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "resource.h"

struct LETTER_DLG_DATA
{
	UINT uFreeLetters;
	TCHAR cLetterToMount;
    TCHAR szUnixName[32];
};


class CLetterDlg : public CSimpleDialog<IDD_LETTER>
{
public:
	typedef CSimpleDialog<IDD_LETTER> _BaseClass;
	CLetterDlg();
	virtual ~CLetterDlg();
	void SetDlgData(const LETTER_DLG_DATA *pData);
	void GetDlgData(LETTER_DLG_DATA *pData);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled);

	BEGIN_MSG_MAP(CMountDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
		COMMAND_ID_HANDLER(IDOK, OnOk);
		CHAIN_MSG_MAP(_BaseClass)
	END_MSG_MAP()
protected:
	LETTER_DLG_DATA m_data;
};

#endif // !defined(AFX_LETTERDLG_H__C242D399_83C9_4B94_B485_705CD900A097__INCLUDED_)
