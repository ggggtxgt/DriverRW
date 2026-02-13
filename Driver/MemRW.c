/********************************************************************************************************************
 * @brief   MemRW.c
 * @details 主要实现进程内存的读写操作；
********************************************************************************************************************/

#include "MemRW.h"

// 检查读取内存时所需参数是否正确
NTSTATUS CheckParams(PVOID start, ULONG64 size, PVOID dest) {
    if ((ULONG64)start > (ULONG64)MmHighestUserAddress ||
        (ULONG64)start + size > (ULONG64)MmHighestUserAddress ||
        (ULONG64)start + size < (ULONG64)start) {
        return STATUS_ACCESS_VIOLATION;     // 无效地址
    }
    if (0 == size) {
        return STATUS_INVALID_PARAMETER_3;  // 大小无效
    }
    if (NULL == dest) {
        return STATUS_INVALID_PARAMETER_4;  // 目标缓冲区无效
    }
    return STATUS_SUCCESS;
}

// 根据PID获取PEPROCESS并验证进程存活
NTSTATUS GetTargetProcess(HANDLE pid, PEPROCESS* process) {
    NTSTATUS status = PsLookupProcessByProcessId(pid, process);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    // 检查进程是否仍在运行（0x103 = STATUS_PENDING，表示未退出）
    if (PsGetProcessExitStatus(*process) != 0x103) {
        ObDereferenceObject(*process);
        *process = NULL;
        return STATUS_INVALID_PARAMETER_1;  // 进程已终止或状态异常
    }
    return STATUS_SUCCESS;
}

// 分配并清零非分页内核缓冲区
PVOID AllocateAndZeroBuffer(ULONG64 size, NTSTATUS* status) {
    PVOID buffer = ExAllocatePool(NonPagedPool, size);
    if (NULL == buffer) {
        *status = STATUS_UNSUCCESSFUL;
        return NULL;
    }
    RtlZeroMemory(buffer, size);
    *status = STATUS_SUCCESS;
    return buffer;
}

// 释放内核缓冲区
VOID FreeKernelBuffer(PVOID buffer) {
    if (buffer != NULL) {
        ExFreePool(buffer);
    }
}

// 以进程挂靠读取指定进程的应用层内存
NTSTATUS ReadR3Memory(HANDLE pid, PVOID start, ULONG64 size, PVOID dest) {
    // 检查传入的参数有效性，避免系统崩溃
    NTSTATUS status = CheckParams(start, size, dest);
    if (!NT_SUCCESS(status)) return status;

    // 获取目标进程 EPROCESS 对象
    PEPROCESS pTargetProcess;
    status = GetTargetProcess(pid, &pTargetProcess);
    if (!NT_SUCCESS(status)) return status;

    // 分配临时缓冲区
    PVOID buff = AllocateAndZeroBuffer(size, &status);
    if (NULL == buff) {
        ObDereferenceObject(pTargetProcess);
        return status;
    }

    // 附加到目标进程并读取
    KAPC_STATE apc = { 0 };
    KeStackAttachProcess(pTargetProcess, &apc);
    if (!(MmIsAddressValid(start) && MmIsAddressValid((ULONG64)start + size))) {
        KeUnstackDetachProcess(&apc);
        ObDereferenceObject(pTargetProcess);
        FreeKernelBuffer(buff);
        return STATUS_ACCESS_VIOLATION;
    }
    RtlCopyMemory(buff, start, size);
    KeUnstackDetachProcess(&apc);

    // 将数据复制到目标缓冲区
    RtlCopyMemory(dest, buff, size);

    // 清理资源
    FreeKernelBuffer(buff);
    ObDereferenceObject(pTargetProcess);
    return STATUS_SUCCESS;
}

// 通过修改CR3读取指定进程的内存
NTSTATUS ReadR3MemoryByCr3(HANDLE pid, PVOID start, ULONG64 size, PVOID dest) {
    // 参数检查
    NTSTATUS status = CheckParams(start, size, dest);
    if (!NT_SUCCESS(status)) return status;

    // 获取目标进程对象
    PEPROCESS pTargetProcess;
    status = GetTargetProcess(pid, &pTargetProcess);
    if (!NT_SUCCESS(status)) return status;

    // 分配内核临时缓冲区
    PVOID buff = AllocateAndZeroBuffer(size, &status);
    if (NULL == buff) {
        ObDereferenceObject(pTargetProcess);
        return status;
    }

    // CR3 切换准备
    // 保存当前 CR3
    ULONG64 curCr3 = __readcr3();   
    // 获取目标CR3
    ULONG64 targetCr3 = *(ULONG64*)((ULONG64)pTargetProcess + 0x28);

    // 进入临界区 + 关中断，保证切换原子性
    KeEnterCriticalRegion();   // 提升 IRQL 至 APC_LEVEL，禁用 APC
    _disable();                // 关闭外部中断

    // 切换到目标进程地址空间
    __writecr3(targetCr3);

    // 读取目标进程内存
    __try {
        // 验证目标用户地址可读（可选，但推荐）
        ProbeForRead(start, size, 1);
        RtlCopyMemory(buff, start, size);   // 复制到内核缓冲区
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        // 读取失败：立即恢复原始地址空间，返回错误码
        __writecr3(curCr3);
        KeLeaveCriticalRegion();
        _enable();
        ObDereferenceObject(pTargetProcess);
        FreeKernelBuffer(buff);
        return GetExceptionCode();
    }

    // 恢复原始地址空间（切换回当前进程）
    __writecr3(curCr3);
    KeLeaveCriticalRegion();
    _enable();

    // 将数据写入目标缓冲区（当前进程上下文）
    __try {
        ProbeForWrite(dest, size, 1);       // 验证目标缓冲区可写
        RtlCopyMemory(dest, buff, size);    // 复制到用户目标地址
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();        // 捕获写入异常
    }

    // 清理资源
    FreeKernelBuffer(buff);
    ObDereferenceObject(pTargetProcess);
    return status;
}
// 通过虚拟内存(MmCopyVirtualMemory)读取指定进程内存
NTSTATUS ReadR3MemoryByVirtualMemory(HANDLE pid, PVOID start, ULONG64 size, PVOID dest) {
    // 检查传入的参数有效性，避免系统崩溃
    NTSTATUS status = CheckParams(start, size, dest);
    if (!NT_SUCCESS(status)) return status;

    // 获取目标进程 EPROCESS 对象
    PEPROCESS pTargetProcess;
    status = GetTargetProcess(pid, &pTargetProcess);
    if (!NT_SUCCESS(status)) return status;

    // 分配临时缓冲区
    PVOID buff = AllocateAndZeroBuffer(size, &status);
    if (NULL == buff) {
        ObDereferenceObject(pTargetProcess);
        return status;
    }

    // 调用 MmCopyVirtualMemory 跨进程复制
    SIZE_T readSize = 0;
    status = MmCopyVirtualMemory(pTargetProcess, start, IoGetCurrentProcess(), buff, size, KernelMode, &readSize);

    // 若成功，将数据复制到调用者提供的缓冲区
    if (NT_SUCCESS(status)) {
        RtlCopyMemory(dest, buff, size);
    }

    // 清理资源
    ExFreePool(buff);
    ObDereferenceObject(pTargetProcess);
    return status;
}

PVOID RwMapMemory(PVOID toAddress, ULONG buffSize, PMDL* pMdl) {
    // 创建 MDL
    PMDL mdl = NULL;
    mdl = IoAllocateMdl(toAddress, buffSize, FALSE, FALSE, NULL);
    // 锁定内存页
    __try {
        MmProbeAndLockPages(mdl, UserMode, IoReadAccess);
    } __except (1) {
        DbgPrint("MmProbeAndLockPages Error!");
        IoFreeMdl(mdl);
        return NULL;
    }
    PVOID mappedAddr = NULL;
    __try {
        mappedAddr = MmMapLockedPagesSpecifyCache(mdl, KernelMode, MmNonCached, NULL, FALSE, HighPagePriority);
    } __except (1) {
        DbgPrint("MmMapLockedPagesSpecifyCache Error!");
        MmUnlockPages(mdl);
        IoFreeMdl(mdl);
        return NULL;
    }
    *pMdl = mdl;
    DbgPrint("RwMapMemory returned: 0x%p", mappedAddr);
    return mappedAddr;
}

void RwMapUnMemory(PVOID toAddress, PMDL pMdl) {
    MmUnmapLockedPages(toAddress, pMdl);
    MmUnlockPages(pMdl);
    IoFreeMdl(pMdl);
}

// 通过MDL读取进程内存
NTSTATUS ReadR3MemoryByMdl(HANDLE pid, PVOID start, ULONG64 size, PVOID dest) {
    // 检查传入的参数有效性，避免系统崩溃
    NTSTATUS status = CheckParams(start, size, dest);
    if (!NT_SUCCESS(status)) return status;

    // 获取目标进程 EPROCESS 对象
    PEPROCESS pTargetProcess;
    status = GetTargetProcess(pid, &pTargetProcess);
    if (!NT_SUCCESS(status)) return status;

    // 分配临时缓冲区
    PVOID buff = AllocateAndZeroBuffer(size, &status);
    if (NULL == buff) {
        ObDereferenceObject(pTargetProcess);
        return status;
    }

    // 附加到目标进程
    KAPC_STATE apc = { 0 };
    KeStackAttachProcess(pTargetProcess, &apc);
    if (!(MmIsAddressValid(start) && MmIsAddressValid((ULONG64)start + size))) {
        KeUnstackDetachProcess(&apc);
        ObDereferenceObject(pTargetProcess);
        FreeKernelBuffer(buff);
        return STATUS_ACCESS_VIOLATION;
    }

    // 使用MDL映射内存并读取
    PMDL pMdl = NULL;
    PVOID mapAddr = RwMapMemory(start, size, &pMdl);
    if (NULL == mapAddr) {
        KeUnstackDetachProcess(&apc);
        ObDereferenceObject(pTargetProcess);
        FreeKernelBuffer(buff);
        return STATUS_ACCESS_VIOLATION;
    }
    RtlCopyMemory(buff, mapAddr, size);
    RwMapUnMemory(mapAddr, pMdl);
    KeUnstackDetachProcess(&apc);

    // 将数据复制到目标缓冲区
    __try {
        ProbeForWrite(dest, size, 1);
        RtlCopyMemory(dest, buff, size);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    // 清理资源
    FreeKernelBuffer(buff);
    ObDereferenceObject(pTargetProcess);
    return STATUS_SUCCESS;
}