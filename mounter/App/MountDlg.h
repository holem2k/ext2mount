// MountDlg.h: interface for the CMountDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOUNTDLG_H__638E58CE_5740_480F_87F6_6CC880A68B01__INCLUDED_)
#define AFX_MOUNTDLG_H__638E58CE_5740_480F_87F6_6CC880A68B01__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "resource.h"
#include <vector>
#include "PartitionList.h"
#include "StatusBar.h"
#include "MountMan.h"
#include "Options.h"

using namespace std;

class CMountDlg : public CSimpleDialog<IDD_MAIN> 
{
public:
	typedef CSimpleDialog<IDD_MAIN> _BaseClass;

	CMountDlg(CCommonMountMan *pMountMan);
	virtual ~CMountDlg();

	LRESULT OnAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled);
	LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled);
	LRESULT OnInitMenuPopup(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnMount(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled);
	LRESULT OnUmount(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled);
    LRESULT OnCodepageCP1251(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled);
    LRESULT OnCodepageKOI8R(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL &bHandled);
	LRESULT OnRClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);		
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	BEGIN_MSG_MAP(CMountDlg)
		COMMAND_ID_HANDLER(ID_CP_CP1251, OnCodepageCP1251)
		COMMAND_ID_HANDLER(ID_CP_KOI8R, OnCodepageKOI8R)
		COMMAND_ID_HANDLER(ID_MAIN_ABOUT, OnAbout)
		COMMAND_ID_HANDLER(ID_MAIN_EXIT, OnExit)
		COMMAND_ID_HANDLER(ID_MAIN_MOUNT, OnMount)
		COMMAND_ID_HANDLER(ID_MAIN_UMOUNT, OnUmount)
		MESSAGE_HANDLER(WM_INITMENUPOPUP, OnInitMenuPopup);
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
        MESSAGE_HANDLER(WM_CLOSE, OnClose);
		NOTIFY_HANDLER(IDC_MAIN_PARTLIST, NM_RCLICK, OnRClick);
		CHAIN_MSG_MAP(_BaseClass)
	END_MSG_MAP()

protected:
    typedef vector<PARTITION_DATA> PARTITION_DATA_VEC; 

	CStatusBar m_status;
	CPartitionList m_ctrlPartitions;
	CCommonMountMan *m_pMountMan;
	PARTITION_DATA_VEC m_partitions;
    COptions m_options;
	
    void CMountDlg::GetMenuItemState(UINT nID, BOOL *pbEnabled, BOOL *pbChecked);
	void PrepareColumns();
    void LoadOptions();
    void SaveOptions();
	BOOL CanMount(UINT nNum);
	BOOL CanUmount(UINT nNum);
};

#endif // !defined(AFX_MOUNTDLG_H__638E58CE_5740_480F_87F6_6CC880A68B01__INCLUDED_)
