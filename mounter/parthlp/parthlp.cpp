#include "stdafx.h"
#include "parthlp.h"
#include "stdio.h"

#pragma comment(lib, "ntdll.lib")

BOOLEAN QuerySymbolicLink(PCHAR lpSymbolicLink, PCHAR lpBuffer, ULONG cbBuffer);

const ULONG MAX_LINK = 1024;

//////////////////////////////////////////////////////////////////////

int GetPartitionLetter(unsigned int nDrive, unsigned int nPartition,
					   CHAR &cLetter)
{
	int iResult = FALSE;

	CHAR szPartition[MAX_LINK];
	sprintf(szPartition, "\\Device\\Harddisk%u\\Partition%u", nDrive, nPartition);

	CHAR szTarget[MAX_LINK];
	if (QuerySymbolicLink(szPartition, szTarget, MAX_LINK))
	{
		cLetter = 0;
		CHAR cCurLetter = 'A';
		do
		{
			CHAR szWin32Name[MAX_LINK];  
			sprintf(szWin32Name, "\\??\\%c:", cCurLetter);

			CHAR szNTName[MAX_LINK];
			if (QuerySymbolicLink(szWin32Name, szNTName, MAX_LINK))
			{
				if (!strcmp(szTarget, szNTName))
				{
					cLetter = cCurLetter;
					break;
				}
			}
		}
		while (++cCurLetter <= 'Z');
		iResult = TRUE;
	}
	return iResult;
}

//////////////////////////////////////////////////////////////////////

BOOLEAN QuerySymbolicLink(PCHAR lpSymbolicLink, PCHAR lpBuffer, ULONG cbBuffer)
{
	if (lpBuffer == NULL || cbBuffer < 2)
		return FALSE;

	NTSTATUS status = STATUS_SUCCESS;

	UNICODE_STRING uszSymbolicLinkName;
	RtlInitUnicodeString(&uszSymbolicLinkName, NULL);

	HANDLE hSymbolicLink = NULL;

	UNICODE_STRING uszResult;

	__try
	{
		// create unicode string for result
		lpBuffer[cbBuffer - 1] = 0;
		RtlFillMemory(lpBuffer, cbBuffer - 1, ' ');
		ANSI_STRING aszResult;
		RtlInitAnsiString(&aszResult, lpBuffer);

		status = RtlAnsiStringToUnicodeString(&uszResult,
			&aszResult, TRUE);
		if (!NT_SUCCESS(status))
			__leave;

		// get symbolic link target
		ANSI_STRING aszSymbolicLinkName;
		RtlInitAnsiString(&aszSymbolicLinkName, lpSymbolicLink);

		status = RtlAnsiStringToUnicodeString(&uszSymbolicLinkName,
			&aszSymbolicLinkName, TRUE);
		if (!NT_SUCCESS(status))
			__leave;
		
		OBJECT_ATTRIBUTES ObjAttr;
		InitializeObjectAttributes(&ObjAttr,
			&uszSymbolicLinkName,
			OBJ_CASE_INSENSITIVE,
			NULL,
			NULL);

		status = NtOpenSymbolicLinkObject(&hSymbolicLink,
			SYMBOLIC_LINK_QUERY,
			&ObjAttr);
		if (!NT_SUCCESS(status))
			__leave;

		ULONG uBytesReturned;
		status = NtQuerySymbolicLinkObject(hSymbolicLink, &uszResult, &uBytesReturned);
		if (!NT_SUCCESS(status))
			__leave;

		if (cbBuffer < wcslen(uszResult.Buffer))
		{
			status = STATUS_BUFFER_TOO_SMALL;
			__leave;
		}
		else
			RtlUnicodeStringToAnsiString(&aszResult, &uszResult, FALSE);
		
	}
	__finally
	{
		if (uszSymbolicLinkName.Buffer != NULL)
			RtlFreeUnicodeString(&uszSymbolicLinkName);

		if (hSymbolicLink != NULL)
			NtClose(hSymbolicLink);

		if (uszResult.Buffer != NULL)
			RtlFreeUnicodeString(&uszResult);
	}

	return status == STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////