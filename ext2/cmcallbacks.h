
extern "C"
BOOLEAN 
Ext2AcquireForLazyWrite(
                        IN  PVOID   Context,
                        IN  BOOLEAN Wait
                        );

extern "C"
VOID
Ext2ReleaseFromLazyWrite(
                         IN PVOID Context
                         );

extern "C"
BOOLEAN 
Ext2AcquireForReadAhead(
                        IN  PVOID   Context,
                        IN  BOOLEAN Wait
                        );


extern "C"
VOID
Ext2ReleaseFromReadAhead(
                         IN PVOID Context
                         );

extern "C"
BOOLEAN 
Ext2NoopAcquire(
                IN  PVOID   Context,
                IN  BOOLEAN Wait
                );

extern "C"
VOID
Ext2NoopRelease(
                IN PVOID Context
                );

extern "C"
BOOLEAN 
Ext2AcquireForCache(
                    IN  PVOID   Context,
                    IN  BOOLEAN Wait
                    );

extern "C"
VOID
Ext2ReleaseFromCache(
                     IN PVOID Context
                     );
