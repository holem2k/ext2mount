// MountMan.h: interface for the CMountMan class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOUNTMAN_H__58EBEE9C_1D13_48DC_B7DA_6693BB8C8F3E__INCLUDED_)
#define AFX_MOUNTMAN_H__58EBEE9C_1D13_48DC_B7DA_6693BB8C8F3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
using namespace std;

typedef struct PARTITION
{
	UINT nDrive;		// 0..3
	UINT nPartition;	// 0..(N - 1)
	BYTE Type;
	__int64 cSize;
	BOOL bMounted;
	TCHAR cLetter;
    BOOL bExtended;
    TCHAR szUnixName[32];
} PARTITION, *PPARTITION;

typedef vector<PARTITION> PARTITION_VEC;


class CCommonMountMan  
{
public:
	CCommonMountMan();
	virtual ~CCommonMountMan();

	virtual BOOL EnumPartitions() = 0;
	virtual BOOL Next(PARTITION *pPartition) = 0;
	virtual void Reset() = 0;

	virtual BOOL Mount(TCHAR cLetter, UINT nDrive, UINT nPartition) = 0;
	virtual BOOL Umount(TCHAR cLetter, BOOL bBroute) = 0;

	virtual BOOL IsFree(TCHAR cLetter) = 0;
protected:
	PARTITION_VEC m_partitions;
	UINT m_nCur;
};


const UINT MAX_HDD_NUM = 4;
const UINT DRIVE_LAYOUT_BUFFER_SIZE = sizeof(DRIVE_LAYOUT_INFORMATION) + sizeof(PARTITION_INFORMATION) * 255;

class CMountMan : public CCommonMountMan
{
public:
	CMountMan();
	virtual ~CMountMan();

	BOOL EnumPartitions();
	BOOL Next(PARTITION *pPartition);
	void Reset();

	BOOL Mount(TCHAR cLetter, UINT nDrive, UINT nPartition);
	BOOL Umount(TCHAR cLetter, BOOL bBrute);

	BOOL IsFree(TCHAR cLetter);
protected:
	BOOL GetMountLetter(UINT nDrive, UINT nPartition, TCHAR &cLetter);
};

#endif // !defined(AFX_MOUNTMAN_H__58EBEE9C_1D13_48DC_B7DA_6693BB8C8F3E__INCLUDED_)
