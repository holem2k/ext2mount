// LetterDlg.cpp: implementation of the LetterDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LetterDlg.h"

//////////////////////////////////////////////////////////////////////

CLetterDlg::CLetterDlg()
{
	ZeroMemory(&m_data, sizeof(m_data));
}

//////////////////////////////////////////////////////////////////////

CLetterDlg::~CLetterDlg()
{

}

//////////////////////////////////////////////////////////////////////

LRESULT CLetterDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	TCHAR szBuf[256];
	wsprintf(szBuf, _T("Смонтировать %s как"),
		m_data.szUnixName);

	SetDlgItemText(IDC_TEXT, szBuf);

	HWND hListbox = GetDlgItem(IDC_LISTBOX);
	wsprintf(szBuf, _T("A:"));
	for (UINT uMask = 1<<26; uMask; uMask >>= 1, szBuf[0]++)
	{
		if (m_data.uFreeLetters & uMask)
			::SendMessage(hListbox, CB_ADDSTRING, 0, (LPARAM)szBuf);
	}
	::SendMessage(hListbox, CB_SETCURSEL, 1, 0);
	::SetFocus(hListbox);


	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////

void CLetterDlg::SetDlgData(const LETTER_DLG_DATA *pData)
{
	m_data = *pData;
}

//////////////////////////////////////////////////////////////////////

void CLetterDlg::GetDlgData(LETTER_DLG_DATA *pData)
{
	*pData = m_data;
}

//////////////////////////////////////////////////////////////////////

LRESULT CLetterDlg::OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled)
{
	HWND hListbox = GetDlgItem(IDC_LISTBOX);

	TCHAR szBuf[256];
	UINT nSel = ::SendMessage(hListbox, CB_GETCURSEL, 0, 0);
	::SendMessage(hListbox, CB_GETLBTEXT, nSel, (LPARAM)szBuf);
	m_data.cLetterToMount = szBuf[0];

	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////