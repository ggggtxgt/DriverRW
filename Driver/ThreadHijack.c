#include "ThreadJijack.h"
#include "GetModuleUitl.h"

// 获取 PsSuspendThread 函数地址（通过特征码）
_PsSuspendThread GetPsSuspendThread() {
    ULONG64 uaddr = RwGetAddrByCode("特征码", 84);
    if (uaddr == 0) return NULL;
    return (_PsSuspendThread)(uaddr - 42);
}

// 挂起指定线程
NTSTATUS SuspendThreadById(HANDLE ThreadId) {
    _PsSuspendThread pfnSuspend = GetPsSuspendThread();
    if (pfnSuspend == NULL) return STATUS_NOT_FOUND;

    PETHREAD Thread = NULL;
    NTSTATUS status = PsLookupThreadByThreadId(ThreadId, &Thread);
    if (!NT_SUCCESS(status)) return status;

    ULONG prevCount = 0;
    status = pfnSuspend(Thread, &prevCount);  // 获取之前挂起计数
    ObDereferenceObject(Thread);
    return status;
}
