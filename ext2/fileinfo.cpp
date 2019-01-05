#include "stdafx.h"
#include "fileinfo.h"
#include "versup.h"
#include "helpers.h"
#include "ext2api.h"

/////////////////////////////////////////////////////////////////////////////
// Ext2QueryInformation(...)
// Expected IRQL: PASSIVE;
// Section: paged.
// Cause page errors: YES(internal)

extern "C"
NTSTATUS
Ext2QueryInformation(
                     PIRP_CONTEXT   IrpContext
                     )
{
    PAGED_CODE();

    ASSERT(IrpContext);
    
    KdHeader(KD_LEV1, "Ext2QueryInformation");

    NTSTATUS status = STATUS_SUCCESS;
    ULONG Length = 0;
    CSHORT Type = Ext2GetFcbType(IrpContext->FileObject);
    PFCB Fcb;
    BOOLEAN FcbResAcquired = FALSE;

    __try
    {
        if (Type == FCB_TYPE)
        {
            PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(IrpContext->Irp);
            Length = IrpStack->Parameters.QueryFile.Length;
            FILE_INFORMATION_CLASS FileInformationClass = IrpStack->Parameters.QueryFile.FileInformationClass;
            PVOID Buffer = IrpContext->Irp->AssociatedIrp.SystemBuffer;
            PFILE_OBJECT FileObject = IrpContext->FileObject;

            Ext2GetFcb(FileObject, &Fcb);
            
            ExAcquireResourceSharedLite(&Fcb->MainResource, TRUE);
            FcbResAcquired = TRUE;
            
            status = Ext2VerifyFcbOperation(IrpContext);
            if (!NT_SUCCESS(status))
                __leave;
            
            switch(FileInformationClass)
            {
            case FileAllInformation:
                {
                    ULONG FakeLength;
                    PFILE_ALL_INFORMATION Info = (PFILE_ALL_INFORMATION)Buffer;
                    
                    Ext2QueryBasicInformation(Fcb, FileObject,
                        &Info->BasicInformation, &FakeLength);
                    
                    Ext2StandardInformation(Fcb, FileObject,
                        &Info->StandardInformation, &FakeLength);
                    
                    Ext2QueryPositionInformation(Fcb, FileObject,
                        &Info->PositionInformation, &FakeLength);
                    
                    Ext2QueryEaInformation(Fcb, FileObject,
                        &Info->EaInformation, &FakeLength);
                    
                    Length -= FIELD_OFFSET(FILE_ALL_INFORMATION, NameInformation);
                    status = Ext2QueryNameInformation(Fcb, FileObject,
                        &Info->NameInformation, &Length);
                    Length += FIELD_OFFSET(FILE_ALL_INFORMATION, NameInformation);
                }
                break;
                
            case FileBasicInformation:
                status = Ext2QueryBasicInformation(Fcb, FileObject, 
                    Buffer, &Length);
                break;
                
            case FileStandardInformation:
                status = Ext2StandardInformation(Fcb, FileObject,
                    Buffer, &Length);
                break;
                
            case FilePositionInformation:
                status = Ext2QueryPositionInformation(Fcb, FileObject, 
                    Buffer, &Length);
                break;
                
            case FileNameInformation:
                status = Ext2QueryNameInformation(Fcb, FileObject, 
                    Buffer, &Length);
                break;
                
            case FileEaInformation:
                status = Ext2QueryEaInformation(Fcb, FileObject,
                    Buffer, &Length);
                break;
                
            default:
                status = STATUS_INVALID_PARAMETER;
            }
        }
        else
            status = STATUS_INVALID_DEVICE_REQUEST;
    }
    __finally
    {
        if (FcbResAcquired)
            ExReleaseResourceLite(&Fcb->MainResource);
        
        if (Ext2CanComplete(IrpContext, status))
        {
            status = Ext2CompleteRequest(IrpContext, status,
                !NT_ERROR(status) ? Length : 0);
        }
    }
    
    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2QueryBasicInformation(
                          PFCB          Fcb,
                          PFILE_OBJECT  FileObject,
                          PVOID         Buffer,
                          PULONG        Length
                          )
{
    PFILE_BASIC_INFORMATION Info = (PFILE_BASIC_INFORMATION)Buffer;
    Info->CreationTime.QuadPart = Ext2TimeUnixToWin(Fcb->Inode->CTime);
    Info->LastAccessTime.QuadPart = Ext2TimeUnixToWin(Fcb->Inode->ATime);
    Info->ChangeTime.QuadPart = Ext2TimeUnixToWin(Fcb->Inode->MTime);
    Info->LastWriteTime.QuadPart = Ext2TimeUnixToWin(Fcb->Inode->MTime);
    Info->FileAttributes = Ext2GetInodeAttributes(Fcb->Inode);
    *Length = sizeof(FILE_BASIC_INFORMATION);
    return STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2StandardInformation(
                        PFCB          Fcb,
                        PFILE_OBJECT  FileObject,
                        PVOID         Buffer,
                        PULONG        Length
                        )
{
    PFILE_STANDARD_INFORMATION Info = (PFILE_STANDARD_INFORMATION)Buffer;
    Info->AllocationSize = Fcb->AllocationSize;
    Info->EndOfFile = Fcb->FileSize;
    Info->NumberOfLinks = Fcb->Inode->LinksCount;
    Info->DeletePending = FALSE;
    Info->Directory = S_ISDIR(Fcb->Inode->Mode);
    *Length = sizeof(FILE_STANDARD_INFORMATION);
    return STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2QueryPositionInformation(
                             PFCB          Fcb,
                             PFILE_OBJECT  FileObject,
                             PVOID         Buffer,
                             PULONG        Length
                             )
{
    PFILE_POSITION_INFORMATION Info = (PFILE_POSITION_INFORMATION)Buffer;
    Info->CurrentByteOffset = FileObject->CurrentByteOffset;
    *Length = sizeof(FILE_POSITION_INFORMATION);
    return STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2QueryNameInformation(
                         PFCB          Fcb,
                         PFILE_OBJECT  FileObject,
                         PVOID         Buffer,
                         PULONG        Length
                         )
{
    PFILE_NAME_INFORMATION Info = (PFILE_NAME_INFORMATION)Buffer;

    NTSTATUS status = STATUS_SUCCESS;
    Info->FileNameLength = Fcb->Name.Length; 

    ULONG BytesToCopy = *Length - FIELD_OFFSET(FILE_NAME_INFORMATION, FileName);
    if (BytesToCopy < Fcb->Name.Length)
    {
        status = STATUS_BUFFER_OVERFLOW;
    }
    else
        BytesToCopy = Fcb->Name.Length;

    RtlCopyMemory(Info->FileName, Fcb->Name.Buffer, BytesToCopy);

    *Length = BytesToCopy + FIELD_OFFSET(FILE_NAME_INFORMATION, FileName);

    return status;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
NTSTATUS
Ext2QueryEaInformation(
                       PFCB          Fcb,
                       PFILE_OBJECT  FileObject,
                       PVOID         Buffer,
                       PULONG        Length
                       )
{
    PFILE_EA_INFORMATION Info = (PFILE_EA_INFORMATION)Buffer;
    Info->EaSize = 0;
    *Length = sizeof(FILE_EA_INFORMATION);
    return STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
// Ext2SetInformation(...)
// Expected IRQL: PASSIVE;
// Section: paged.
// Cause page errors: YES(internal)

extern "C"
NTSTATUS
Ext2SetInformation(
                   PIRP_CONTEXT   IrpContext
                   )
{
    PAGED_CODE();
    
    ASSERT(IrpContext);
    
    KdHeader(KD_LEV1, "Ext2QueryInformation");

    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(IrpContext->Irp);
    FILE_INFORMATION_CLASS FileInformationClass = IrpStack->Parameters.SetFile.FileInformationClass;
    if (FileInformationClass != FilePositionInformation)
        return Ext2CompleteRequest(IrpContext, STATUS_INVALID_DEVICE_REQUEST);

    NTSTATUS status = STATUS_SUCCESS;
    CSHORT Type = Ext2GetFcbType(IrpContext->FileObject);
    if (Type != FCB_TYPE)
        return Ext2CompleteRequest(IrpContext, STATUS_INVALID_DEVICE_REQUEST);

    PFILE_POSITION_INFORMATION Info = (PFILE_POSITION_INFORMATION)IrpContext->Irp->AssociatedIrp.SystemBuffer;
    PFILE_OBJECT FileObject = IrpContext->FileObject;

    if (FlagOn(FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING))
    {
        PDEVICE_OBJECT DeviceObject = IrpStack->DeviceObject;
        if (Info->CurrentByteOffset.LowPart & DeviceObject->AlignmentRequirement)
            return Ext2CompleteRequest(IrpContext, STATUS_INVALID_PARAMETER);
    }

    FileObject->CurrentByteOffset.QuadPart = Info->CurrentByteOffset.QuadPart;

    return Ext2CompleteRequest(IrpContext, STATUS_SUCCESS);
}

/////////////////////////////////////////////////////////////////////////////
