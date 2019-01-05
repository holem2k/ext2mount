// PartitionList.h: interface for the CPartitionList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARTITIONLIST_H__D20C38A2_73DC_4769_8D91_0737F3771C74__INCLUDED_)
#define AFX_PARTITIONLIST_H__D20C38A2_73DC_4769_8D91_0737F3771C74__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MountMan.h"

struct PARTITION_DATA
{
	UINT nDrive;
	UINT nPartition;
	BYTE Type;
	__int64 cSize;
	BOOL bMounted;
	TCHAR cLetter;
    BOOL bExtended;
	UINT uLocale; // reserved;
    TCHAR szUnixName[32];
};

class CPartitionList : public CWindowImpl<CPartitionList>
{
public:
	CPartitionList();
	virtual ~CPartitionList();

	BOOL InitColumns();
	BOOL AddLine(const PARTITION_DATA *pPartition);
	BOOL ClearLines();
	BOOL GetSelection(UINT *puSelection);
	void SetSelection(UINT uSelection);

	BEGIN_MSG_MAP(CPartitionList)
	END_MSG_MAP();
protected:
	HIMAGELIST m_hImageList; 
};


#endif // !defined(AFX_PARTITIONLIST_H__D20C38A2_73DC_4769_8D91_0737F3771C74__INCLUDED_)
