// PartitionList.cpp: implementation of the CPartitionList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PartitionList.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPartitionList::CPartitionList()
{
	m_hImageList = NULL;
}

//////////////////////////////////////////////////////////////////////

CPartitionList::~CPartitionList()
{
//	if (m_hImageList)
//		ImageList_Destroy(m_hImageList);
}

//////////////////////////////////////////////////////////////////////

BOOL CPartitionList::InitColumns()
{
	BOOL bResult = TRUE;
	const LVCOLUMN rgColumns[] = 
	{
		// 1st column
		{
			LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM, // mask
				LVCFMT_LEFT, // fmt
				100, // cx
				TEXT("Раздел"),
				0,
				0, // subitem
				-1, // image
				-1, // order
		},
		// 2nd column
		{
				LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM, // mask
					LVCFMT_RIGHT, // fmt
					100, // cx
					TEXT("Размер"),
					0,
					1, // subitem
					-1, // image
					-1, // order
			},
			// 3d column
			{
					LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM, // mask
						LVCFMT_LEFT, // fmt
						240, // cx
						TEXT("Буква"),
						0,
						2, // subitem
						-1, // image
						-1, // order
				}
				
	};

	for (UINT i = 0; i < sizeof(rgColumns)/sizeof(rgColumns[0]); i++)
	{
		if (SendMessage(LVM_INSERTCOLUMN, i, (LPARAM)(rgColumns + i)) == -1)
		{
			bResult = FALSE;
			break;
		}
	}

	if (!m_hImageList)
	{
		m_hImageList = ImageList_Create(16, 16, ILC_COLOR8 | ILC_MASK, 1, 1);
		HICON hIcon = LoadIcon(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_PARTITION_16)); 
		ImageList_AddIcon(m_hImageList, hIcon); 
		SendMessage(LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)m_hImageList);
	}

	SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, 
		LVS_EX_FULLROWSELECT);

	return bResult;
}

//////////////////////////////////////////////////////////////////////

BOOL CPartitionList::AddLine(const PARTITION_DATA *pPartition)
{
	TCHAR szBuffer[256];

	// 'partition' column
	//wsprintf(szBuffer, _T("hd%c%u"), 'a' + pPartition->nDrive, 
      //  pPartition->bExtended ? pPartition->nPartition + 4 : pPartition->nPartition + 1);

	LVITEM item;
	item.mask =  LVIF_TEXT | LVIF_IMAGE;
	item.iItem = SendMessage(LVM_GETITEMCOUNT, 0);
	item.iSubItem = 0;
	item.pszText = (TCHAR *)pPartition->szUnixName; 
	item.iImage = 0;
	SendMessage(LVM_INSERTITEM, 0, (LPARAM)&item);

	// 'size' column
	if (pPartition->cSize < 1024*1024) // < 1 МБ
		wsprintf(szBuffer, _T("%u КБ"), pPartition->cSize/1024);
	else 
		wsprintf(szBuffer, _T("%u МБ"), pPartition->cSize/1024/1024);

	item.mask = LVIF_TEXT;
	item.iSubItem = 1;
	item.pszText = szBuffer;
	SendMessage(LVM_SETITEM, 0, (LPARAM)&item);

	// 'letter' column
	if (pPartition->bMounted)
		wsprintf(szBuffer, _T("%C:"), pPartition->cLetter);
	else
		wsprintf(szBuffer, _T("Нет"));
	item.mask = LVIF_TEXT;
	item.iSubItem = 2;
	item.pszText = szBuffer;
	SendMessage(LVM_SETITEM, 0, (LPARAM)&item);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////

BOOL CPartitionList::ClearLines()
{
	return SendMessage(LVM_DELETEALLITEMS, 0, 0);
}

//////////////////////////////////////////////////////////////////////

BOOL CPartitionList::GetSelection(UINT *puSelection)
{
	*puSelection = SendMessage(LVM_GETSELECTIONMARK, 0, 0);
	return *puSelection != -1;
}

//////////////////////////////////////////////////////////////////////

void CPartitionList::SetSelection(UINT uSelection)
{
	LVITEM item;
	item.state = LVIS_SELECTED | LVIS_FOCUSED;
	item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
	SendMessage(LVM_SETITEMSTATE, uSelection, (LPARAM)&item);
}

//////////////////////////////////////////////////////////////////////

