// MountMan.cpp: implementation of the CMountMan class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MountMan.h"

#ifdef USE_NATIVE_API
#include "..\parthlp\parthlp.h"
#endif

//////////////////////////////////////////////////////////////////////
/////////////////////////////////n/////////////////////////////////////

CCommonMountMan::CCommonMountMan()
{
	m_nCur = 0;
}

//////////////////////////////////////////////////////////////////////

CCommonMountMan::~CCommonMountMan()
{

}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CMountMan::CMountMan()
{

}

//////////////////////////////////////////////////////////////////////

CMountMan::~CMountMan()
{

}

//////////////////////////////////////////////////////////////////////

BOOL CMountMan::EnumPartitions()
{
	BOOL bResult = TRUE;

	m_partitions.clear();
	m_nCur = 0;
	m_partitions.reserve(0x10);

	for (UINT i = 0; i < MAX_HDD_NUM && bResult; i++)
	{
		TCHAR szDiskName[256];
		wsprintf(szDiskName, _T("\\\\.\\PhysicalDrive%u"), i);

		HANDLE hFile = ::CreateFile(szDiskName,
			GENERIC_READ,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			if (::GetLastError() == ERROR_FILE_NOT_FOUND)
				continue;
			else
			{
				bResult = FALSE;
				break;
			}
		}

		DWORD dwBytesReturned;
		BYTE lpBuffer[DRIVE_LAYOUT_BUFFER_SIZE];

		bResult = ::DeviceIoControl(hFile,
			IOCTL_DISK_GET_DRIVE_LAYOUT,
			NULL,
			0,
			lpBuffer,
			sizeof(lpBuffer),
			&dwBytesReturned,
			NULL);

		if (bResult)
		{
			DRIVE_LAYOUT_INFORMATION *pInfo = (DRIVE_LAYOUT_INFORMATION *)lpBuffer;
			for (UINT j = 0; j < pInfo->PartitionCount; j++)
			{
				if (pInfo->PartitionEntry[j].PartitionNumber)
				{
					PARTITION partition;
					partition.nDrive = i;
					partition.nPartition = pInfo->PartitionEntry[j].PartitionNumber;
					partition.Type = pInfo->PartitionEntry[j].PartitionType;
					partition.cSize = pInfo->PartitionEntry[j].PartitionLength.QuadPart;
					partition.bMounted = GetMountLetter(i, partition.nPartition, partition.cLetter);
                    partition.bExtended = j >= 4;
                    wsprintf(partition.szUnixName, _T("hd%c%u"), 'a' + i, j < 4 ? j + 1 : 4 + j / 4); 

					m_partitions.push_back(partition);
				}
			}
		}

		::CloseHandle(hFile);
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////

BOOL CMountMan::Next(PARTITION *pPartition)
{
	if (m_nCur < m_partitions.size())
	{
		*pPartition = m_partitions[m_nCur++];
		return TRUE;
	}
	else
		return FALSE;
}

//////////////////////////////////////////////////////////////////////

void CMountMan::Reset()
{
	m_nCur++;
}

//////////////////////////////////////////////////////////////////////

BOOL CMountMan::Mount(TCHAR cLetter, UINT nDrive, UINT nPartition)
{
	BOOL bResult = FALSE;

	TCHAR szMountPath[] = _T("Z:");
	szMountPath[0] = cLetter;

	TCHAR szPartition[MAX_PATH];
	wsprintf(szPartition, _T("\\Device\\Harddisk%u\\Partition%u"), nDrive, nPartition);

	if (::DefineDosDevice(DDD_RAW_TARGET_PATH, szMountPath, szPartition))
	{
		TCHAR szMountPoint[] = _T("Z:\\");
		szMountPoint[0] = cLetter;
		TCHAR szVolume[MAX_PATH];

		BOOL bVolume = ::GetVolumeNameForVolumeMountPoint(szMountPoint, szVolume, sizeof(szVolume));
		::DefineDosDevice(DDD_RAW_TARGET_PATH | DDD_REMOVE_DEFINITION,
			szMountPath, szPartition);

		if (bVolume)
			bResult = SetVolumeMountPoint(szMountPoint, szVolume);
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////

BOOL CMountMan::Umount(TCHAR cLetter, BOOL bBrute)
{
	BOOL bResult = FALSE;

	TCHAR szMountPoint[] = _T("Z:\\");
	szMountPoint[0] = cLetter;

    TCHAR szPartition[MAX_PATH];
    wsprintf(szPartition, _T("\\\\.\\%c:"), cLetter);
    
    HANDLE hPartition = ::CreateFile(szPartition,
        GENERIC_READ,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (hPartition != INVALID_HANDLE_VALUE)
    {
        bResult = TRUE;
        if (!bBrute)
        {
			DWORD dwBytesReturned;
            bResult = ::DeviceIoControl(hPartition,
                FSCTL_LOCK_VOLUME, 
                NULL,
                0,
                NULL,
                0,
                &dwBytesReturned,
                NULL);
        }

        if (bResult)
        {
			DWORD dwBytesReturned;
            bResult = ::DeviceIoControl(hPartition,
                FSCTL_DISMOUNT_VOLUME, 
                NULL,
                0,
                NULL,
                0,
                &dwBytesReturned,
                NULL);

            if (bResult)            
                bResult = DeleteVolumeMountPoint(szMountPoint);
        }

        ::CloseHandle(hPartition); // unlock if locked
    }

	return bResult;
}

//////////////////////////////////////////////////////////////////////

BOOL CMountMan::IsFree(TCHAR cLetter)
{
	TCHAR szNTDevice[MAX_PATH];

	TCHAR szDevice[] = TEXT("Z:");
	szDevice[0] = cLetter;
	
	return !::QueryDosDevice(szDevice, szNTDevice, sizeof(szNTDevice));
}

//////////////////////////////////////////////////////////////////////

BOOL CMountMan::GetMountLetter(UINT nDrive, UINT nPartition, TCHAR &cLetter)
{
#ifndef USE_NATIVE_API

	cLetter = 0;

	// find free letter
	TCHAR cFreeLetter = 'A';
	DWORD dwLogDrives = ::GetLogicalDrives();

	while (dwLogDrives & 0x01 && cFreeLetter <= 'Z')
		dwLogDrives >>= 1, cFreeLetter++;

	if (cFreeLetter == 'Z' + 1)
		return FALSE;

	// find partition mount point
	BOOL bResult = FALSE;

	TCHAR szMountPath[] = "Z:";
	szMountPath[0] = cFreeLetter;

	TCHAR szPartition[MAX_PATH];
	wsprintf(szPartition, "\\Device\\Harddisk%u\\Partition%u", nDrive, nPartition + 1);

	if (::DefineDosDevice(DDD_RAW_TARGET_PATH, szMountPath, szPartition))
	{
		TCHAR szMountPoint[] = "Z:\\";
		szMountPoint[0] = cFreeLetter;
		TCHAR szVolume[MAX_PATH];

		if (::GetVolumeNameForVolumeMountPoint(szMountPoint, szVolume, sizeof(szVolume)))
		{
			TCHAR cCurLetter = 'B'; // start from C:
			while (!bResult && ++cCurLetter <= 'Z')
			{
				if (cCurLetter == cFreeLetter)
					continue;

				TCHAR szCurMountPoint[] = _T("Z:\\");
				szCurMountPoint[0] = cCurLetter;
				TCHAR szCurVolume[MAX_PATH];
				if (GetVolumeNameForVolumeMountPoint(szCurMountPoint, szCurVolume, sizeof(szCurVolume)))
					bResult = !lstrcmp(szCurVolume, szVolume);
			}
			if (bResult)
				cLetter = cCurLetter;
		}
		::DefineDosDevice(DDD_RAW_TARGET_PATH | DDD_REMOVE_DEFINITION,
			szMountPath, szPartition);
	}
	return bResult;
#else
	BOOL bResult = FALSE;
	CHAR cLet;
	if (GetPartitionLetter(nDrive, nPartition, cLet) && cLet)
	{
		cLetter = cLet;
		bResult = TRUE;
	}
	return bResult;
#endif
}

//////////////////////////////////////////////////////////////////////