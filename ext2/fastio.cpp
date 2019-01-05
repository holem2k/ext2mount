#include "stdafx.h"
#include "fastio.h"

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2AcquireFileForNtCreateSection(
                                  IN PFILE_OBJECT    FileObject
                                  )
{
    PAGED_CODE();

    KdHeader(KD_LEV1, "acquire for NtCreateSection");

    PCOMMON_FCB Fcb = Ext2GetCommonFcb(FileObject);
    ASSERT(Fcb->NodeTypeCode == FCB_TYPE);
    ExAcquireResourceExclusiveLite(Fcb->Resource, TRUE);
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
VOID
Ext2ReleaseFileFromNtCreateSection(
                                  IN PFILE_OBJECT    FileObject
                                  )
{
    PAGED_CODE();

    KdHeader(KD_LEV1, "release from NtCreateSection");

    PCOMMON_FCB Fcb = Ext2GetCommonFcb(FileObject);
    ASSERT(Fcb->NodeTypeCode == FCB_TYPE);
    ExReleaseResourceLite(Fcb->Resource);
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOLEAN
Ext2FastIoCheckIfPossible(
                        IN PFILE_OBJECT         FileObject,
                        IN PLARGE_INTEGER       FileOffset,
                        IN ULONG                Length,
                        IN BOOLEAN              Wait,
                        IN ULONG                LockKey,
                        IN BOOLEAN              CheckForReadOperation,
                        OUT PIO_STATUS_BLOCK    IoStatus,
                        IN PDEVICE_OBJECT       DeviceObject
                        )
{
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOLEAN
Ext2FastQueryBasicInfo (
                      IN PFILE_OBJECT                   FileObject,
                      IN BOOLEAN                        Wait,
                      IN OUT PFILE_BASIC_INFORMATION    Buffer,
                      OUT PIO_STATUS_BLOCK              IoStatus,
                      IN PDEVICE_OBJECT                 DeviceObject
                      )
{
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOLEAN
Ext2FastQueryStdInfo (
                      IN PFILE_OBJECT                   FileObject,
                      IN BOOLEAN                        Wait,
                      IN OUT PFILE_STANDARD_INFORMATION Buffer,
                      OUT PIO_STATUS_BLOCK              IoStatus,
                      IN PDEVICE_OBJECT                 DeviceObject
                      )
{
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOLEAN
Ext2FastLock(
             IN PFILE_OBJECT     FileObject,
             IN PLARGE_INTEGER   FileOffset,
             IN PLARGE_INTEGER   Length,
             PEPROCESS           ProcessId,
             ULONG               Key,
             BOOLEAN             FailImmediately,
             BOOLEAN             ExclusiveLock,
             OUT PIO_STATUS_BLOCK IoStatus,
             IN PDEVICE_OBJECT   DeviceObject
             )
{
    return FALSE;
}

            
/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOLEAN
Ext2FastUnlockSingle(
                     IN PFILE_OBJECT   FileObject,
                     IN PLARGE_INTEGER FileOffset,
                     IN PLARGE_INTEGER Length,
                     PEPROCESS         ProcessId,
                     ULONG             Key,
                     OUT PIO_STATUS_BLOCK IoStatus,
                     IN PDEVICE_OBJECT DeviceObject
                     )
{
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOLEAN
Ext2FastUnlockAll(
                  IN PFILE_OBJECT       FileObject,
                  PEPROCESS             ProcessId,
                  OUT PIO_STATUS_BLOCK  IoStatus,
                  IN PDEVICE_OBJECT     DeviceObject
                  )
{
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOLEAN
Ext2FastUnlockAllByKey(
                       IN PFILE_OBJECT  FileObject,
                       PVOID            ProcessId,
                       ULONG            Key,
                       OUT PIO_STATUS_BLOCK IoStatus,
                       IN PDEVICE_OBJECT    DeviceObject
                       )
{
    return FALSE;
}
                       
/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOLEAN
Ext2FastQueryNetworkInfo(
                         IN PFILE_OBJECT        FileObject,
                         IN BOOLEAN             Wait,
                         OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
                         OUT PIO_STATUS_BLOCK   IoStatus,
                         IN PDEVICE_OBJECT      DeviceObject
                         )
{
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
