// MountDlg.cpp: implementation of the CMountDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MountDlg.h"
#include "resource.h"
#include "herlpers.h"
#include "LetterDlg.h"
#include "..\..\ext2\common.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMountDlg::CMountDlg(CCommonMountMan *pMountMan) : m_pMountMan(pMountMan)
{
}

//////////////////////////////////////////////////////////////////////

CMountDlg::~CMountDlg()
{
}

//////////////////////////////////////////////////////////////////////

LRESULT CMountDlg::OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled)
{
	CSimpleDialog<IDD_ABOUT> dlg;
	dlg.DoModal();
	bHandled = TRUE;
	return 0; // processed
}

//////////////////////////////////////////////////////////////////////

LRESULT CMountDlg::OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    SaveOptions();

	::EndDialog(m_hWnd, 0);
	bHandled = TRUE;
	return 0; // processed
}

//////////////////////////////////////////////////////////////////////

LRESULT CMountDlg::OnInitMenuPopup(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HMENU hMenu = (HMENU)wParam;
	for (UINT i = 0; i < (UINT)::GetMenuItemCount(hMenu); i++)
	{
		MENUITEMINFO info;
		info.cbSize = sizeof(info);
		info.fMask = MIIM_STATE | MIIM_ID | MIIM_SUBMENU;
		BOOL a = ::GetMenuItemInfo(hMenu, i, TRUE, &info);
		if (info.hSubMenu == NULL)
		{
            BOOL bEnabled, bChecked;
            GetMenuItemState(info.wID, &bEnabled, &bChecked);
			info.fState = bEnabled ? MFS_ENABLED : MFS_DISABLED; 
            if (bChecked)
                info.fState |= MFS_CHECKED;
			::SetMenuItemInfo(hMenu, i, TRUE, &info);
		}
	}
	bHandled = TRUE;
	return 0; // processed
}

//////////////////////////////////////////////////////////////////////

void CMountDlg::GetMenuItemState(UINT nID, BOOL *pbEnabled, BOOL *pbChecked)
{
    BOOL bChecked = FALSE, bEnabled = FALSE;
	switch (nID)
	{
	case ID_MAIN_MOUNT:
		{
			UINT nSel;
			if (m_ctrlPartitions.GetSelection(&nSel))
				bEnabled = CanMount(nSel);
		}		
		break;

	case ID_MAIN_UMOUNT:
		{
			UINT nSel;
			if (m_ctrlPartitions.GetSelection(&nSel))
				bEnabled = CanUmount(nSel);
		}		
		break;

	case ID_MAIN_EXIT:
	case ID_MAIN_ABOUT:
    case SC_MOVE:
    case SC_CLOSE:
    case SC_MINIMIZE:
		bEnabled = TRUE;
		break;

    case ID_CP_CP1251:
        bEnabled = TRUE;
        bChecked = m_options[PARAMNAME_CODE_PAGE] == CP_CP1251;
        break;

    case ID_CP_KOI8R:
        bEnabled = TRUE;
        bChecked = m_options[PARAMNAME_CODE_PAGE] == CP_KOI8R;
        break;
	}

    *pbEnabled = bEnabled;
    *pbChecked = bChecked;
}

//////////////////////////////////////////////////////////////////////

LRESULT CMountDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	HICON hIcon = ::LoadIcon(::GetModuleHandle(NULL), 
		MAKEINTRESOURCE(IDI_MOUNTER));
	SetIcon(hIcon);
	SetIcon(hIcon, FALSE);

	m_ctrlPartitions.SubclassWindow(::GetDlgItem(m_hWnd, IDC_MAIN_PARTLIST));
	m_ctrlPartitions.InitColumns();

	m_status.Create(this);

    LoadOptions();

	PrepareColumns();
	bHandled = FALSE;
	return 0;
}

//////////////////////////////////////////////////////////////////////

void CMountDlg::PrepareColumns()
{
	UINT uSel;
	if (!m_ctrlPartitions.GetSelection(&uSel))
		uSel = 0xFFFFFFFF;

	m_status.SetSimpleText(_T(""));
	m_partitions.clear();
	m_ctrlPartitions.ClearLines();

	CUnderAdmin admin;
	if (!admin.Check())
	{
		m_status.SetSimpleText(_T("Для работы программы необходимы права администратора"));
		return;
	}

	if (m_pMountMan->EnumPartitions())
	{
		PARTITION partition;
		while (m_pMountMan->Next(&partition))
		{
			if (partition.Type == 0x83)
			{
				PARTITION_DATA data;
				data.nDrive = partition.nDrive;
				data.nPartition = partition.nPartition;
				data.cSize = partition.cSize;
				data.bMounted = partition.bMounted;
				data.cLetter = partition.cLetter;
                data.bExtended = partition.bExtended;
                lstrcpy(data.szUnixName, partition.szUnixName);
				m_partitions.push_back(data);
				m_ctrlPartitions.AddLine(&data);
			}
		}
		if (m_partitions.size() == 0)
			m_status.SetSimpleText(_T("Разделы файловой системы ext2 не найдены."));

		if (uSel != 0xFFFFFFFF)
			m_ctrlPartitions.SetSelection(uSel);
	}
	else
		m_status.SetSimpleText(_T("Ошибка определения конфигурации разделов - закройте остальные программы."));


}

//////////////////////////////////////////////////////////////////////

void CMountDlg::LoadOptions()
{
    m_options[PARAMNAME_CODE_PAGE] = CP_KOI8R;

    String strRegPath(_T("System\\CurrentControlSet\\Services\\"));
    strRegPath = strRegPath + DRIVER_NAME + _T("\\") + PARAM_SUBKEY_NAME;
    CRegStorage storage(strRegPath.c_str());
    m_options.Load(&storage);

    if (m_options[PARAMNAME_CODE_PAGE] >= CP_INVALID)
        m_options[PARAMNAME_CODE_PAGE] = CP_KOI8R;
}

//////////////////////////////////////////////////////////////////////

void CMountDlg::SaveOptions()
{
    String strRegPath(_T("System\\CurrentControlSet\\Services\\"));
    strRegPath = strRegPath + DRIVER_NAME + _T("\\") + PARAM_SUBKEY_NAME;
    CRegStorage storage(strRegPath.c_str());
    m_options.Save(&storage);
}

//////////////////////////////////////////////////////////////////////

BOOL CMountDlg::CanMount(UINT nNum)
{
	return !m_partitions[nNum].bMounted;
}

//////////////////////////////////////////////////////////////////////

BOOL CMountDlg::CanUmount(UINT nNum)
{
	return  m_partitions[nNum].bMounted;
}

//////////////////////////////////////////////////////////////////////

LRESULT CMountDlg::OnMount(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	UINT nSel;
	if (m_ctrlPartitions.GetSelection(&nSel))
	{
		LETTER_DLG_DATA data;
		data.uFreeLetters = 0;
        wcscpy(data.szUnixName, m_partitions[nSel].szUnixName);

		for (TCHAR cLetter = 'A'; cLetter <= 'Z'; cLetter++, data.uFreeLetters <<= 1)
			data.uFreeLetters |= 1 & m_pMountMan->IsFree(cLetter);

		CLetterDlg dlg;
		dlg.SetDlgData(&data);
		if (dlg.DoModal() == IDOK)
		{
			dlg.GetDlgData(&data);
			m_status.SetSimpleText(_T("Монтирование..."));
			if (!m_pMountMan->Mount(data.cLetterToMount, 
                m_partitions[nSel].nDrive, m_partitions[nSel].nPartition))
			{
				MessageBox(_T("Ошибка при монтировании раздела"),
					_T("Ошибка"), MB_OK | MB_ICONEXCLAMATION);
			}
			m_status.SetSimpleText(_T(""));
			PrepareColumns();
		}		
	}
	bHandled = TRUE;
	return 0;
}

//////////////////////////////////////////////////////////////////////

LRESULT CMountDlg::OnUmount(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	UINT nSel;
	if (m_ctrlPartitions.GetSelection(&nSel))
	{
		ATLASSERT(m_partitions[nSel].bMounted);
		if (!m_pMountMan->Umount(m_partitions[nSel].cLetter, FALSE))
		{
			int iRes = MessageBox(_T("Размонтирование раздела может привести к потере данных. Продолжить ?"),
				_T("Предупреждение"), MB_YESNO | MB_ICONEXCLAMATION);

			if (iRes == IDYES)
			{
				if (!m_pMountMan->Umount(m_partitions[nSel].cLetter, TRUE))
				{
					MessageBox(_T("Ошибка при размонтировании раздела"),
						_T("Ошибка"), MB_OK | MB_ICONEXCLAMATION);
				}
			}

		}
		PrepareColumns();
	}
	bHandled = TRUE;
	return 0;
}

//////////////////////////////////////////////////////////////////////

LRESULT CMountDlg::OnRClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)pnmh;
	HMENU hMenu = ::LoadMenu(::GetModuleHandle(NULL),
		MAKEINTRESOURCE(IDR_POPUP_MENU));
	if (hMenu != NULL)
	{
		int x = lpnmitem->ptAction.x, y = lpnmitem->ptAction.y;

		RECT rect;
		::GetWindowRect(GetDlgItem(IDC_MAIN_PARTLIST), &rect);
		x += rect.left; y += rect.top;

		HMENU hSubMenu = ::GetSubMenu(hMenu, 0);
		ATLASSERT(hSubMenu);
		::TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, 
			x, y, 0, m_hWnd, NULL);
		::DestroyMenu(hMenu);
	}
	bHandled = FALSE;
	return 0; // allow default processing
}

//////////////////////////////////////////////////////////////////////

LRESULT CMountDlg::OnCodepageCP1251(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled)
{
    m_options[PARAMNAME_CODE_PAGE] = CP_CP1251;
	bHandled = TRUE;
	return 0; // allow default processing
}

//////////////////////////////////////////////////////////////////////

LRESULT CMountDlg::OnCodepageKOI8R(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled)
{
    m_options[PARAMNAME_CODE_PAGE] = CP_KOI8R;
	bHandled = TRUE;
	return 0; // allow default processing
}

//////////////////////////////////////////////////////////////////////

LRESULT CMountDlg::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    SaveOptions();

    bHandled = FALSE;
    return 0; // processed
}

//////////////////////////////////////////////////////////////////////