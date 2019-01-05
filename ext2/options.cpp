#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOLEAN
Ext2GetRegDwordValue(
                     IN HANDLE  Key,
                     IN PCWSTR  KeyName,
                     OUT PULONG Value
                     )
{
    UNICODE_STRING ParamName;
    RtlInitUnicodeString(&ParamName, KeyName);

    const ULONG KeyInfoLength = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG) - sizeof(UCHAR);
    UCHAR Buffer[KeyInfoLength];
    PKEY_VALUE_PARTIAL_INFORMATION Info = (PKEY_VALUE_PARTIAL_INFORMATION)Buffer;

    ULONG ResultLength;
    NTSTATUS status = ZwQueryValueKey(Key, &ParamName,
        KeyValuePartialInformation, Info, KeyInfoLength, &ResultLength);

    if (NT_SUCCESS(status) && Info->DataLength == sizeof(ULONG))
    {
        *Value = *((PULONG)Info->Data);
        return TRUE;
    }

    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOLEAN
Ext2LoadOptions(
                PEXT2_OPTIONS   Options,
                PUNICODE_STRING RegPath
                )
{
    // get key name
    UNICODE_STRING KeyName;
    USHORT KeyNameLength = RegPath->Length + sizeof(WCHAR) + sizeof(PARAM_SUBKEY_NAME);
    KeyName.Buffer = (PWCHAR)ExAllocatePool(PagedPool, KeyNameLength);
    if (KeyName.Buffer == NULL)
        return FALSE;
    KeyName.Length = 0;
    KeyName.MaximumLength = KeyNameLength;
    RtlAppendUnicodeStringToString(&KeyName, RegPath);
    RtlAppendUnicodeToString(&KeyName, L"\\");
    RtlAppendUnicodeToString(&KeyName, PARAM_SUBKEY_NAME);

    OBJECT_ATTRIBUTES ObjectAttributes;
    InitializeObjectAttributes(&ObjectAttributes, &KeyName, 0, NULL, NULL);

    BOOLEAN AllLoaded = TRUE;

    HANDLE Key;
    NTSTATUS status = ZwOpenKey(&Key, KEY_QUERY_VALUE, &ObjectAttributes);
    if (NT_SUCCESS(status))
    {
        ULONG Value;
        BOOLEAN Result;

        // get 'CodePage' option
        Result = Ext2GetRegDwordValue(Key, PARAMNAME_CODE_PAGE, &Value);
        if (Result)
            Options->CodePage = static_cast<CODE_PAGE>(Value);
        else
            AllLoaded = FALSE;

        // get 'ResolveLinks' option
        Result = Ext2GetRegDwordValue(Key, PARAMNAME_RESOLVE_LINKS, &Value);
        if (Result)
            Options->ResolveLinks = static_cast<BOOLEAN>(Value);
        else
            AllLoaded = FALSE;

        ZwClose(Key);
    }

    ExFreePool(KeyName.Buffer);

    return AllLoaded;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2LoadDefaultOptions(
                       PEXT2_OPTIONS Options
                       )
{
    Options->CodePage = CP_CP1251;
    Options->ResolveLinks = FALSE; // not supported in this version
}

/////////////////////////////////////////////////////////////////////////////