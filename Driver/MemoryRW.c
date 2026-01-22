#include "MemoryRW.h"

#include <intrin.h>

NTSTATUS ReadR3Memory(HANDLE pId, PVOID startAddr, ULONG64 size, PVOID destAddr) {
    // 检查传入的参数有效性，避免系统崩溃
    {
        // 判断读取的起始地址是否为驱动层地址
        if ((ULONG64) startAddr > (ULONG64) MmHighestUserAddress ||
            (ULONG64) startAddr + size > (ULONG64) MmHighestUserAddress ||
            (ULONG64) startAddr + size < (ULONG64) startAddr) {
            // 错误：无效的内存地址
            return STATUS_ACCESS_VIOLATION;
        }
        if (0 == size) {
            // 错误：第3个参数无效
            return STATUS_INVALID_PARAMETER_3;
        }
        if (NULL == destAddr) {
            // 错误：第4个参数无效
            return STATUS_INVALID_PARAMETER_4;
        }
    }

    // 获取目标进程 EPROCESS 对象
    PEPROCESS pTargetProcess;
    NTSTATUS status = PsLookupProcessByProcessId(pId, &pTargetProcess);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    if (PsGetProcessExitStatus(pTargetProcess) != 0x103) {
        ObDereferenceObject(pTargetProcess);    // 释放进程引用次数
        // 错误：第1个参数错误--无效的进程ID
        return STATUS_INVALID_PARAMETER_1;
    }

    // 分配临时缓冲区
    PVOID buff = ExAllocatePool(NonPagedPool, size);
    if (NULL == buff) {
        ObDereferenceObject(pTargetProcess);
        return STATUS_UNSUCCESSFUL;
    }

    // 附加到目标进程地址空间
    RtlZeroMemory(buff, size);
    KAPC_STATE apc = {0};
    KeStackAttachProcess(pTargetProcess, &apc); // 进程挂靠
    if (!(MmIsAddressValid(startAddr) && MmIsAddressValid((ULONG64) startAddr + size))) {
        ObDereferenceObject(pTargetProcess);
        KeUnstackDetachProcess(&apc);
        ExFreePool(buff);
        return STATUS_ACCESS_VIOLATION;
    }
    RtlCopyMemory(buff, startAddr, size);
    KeUnstackDetachProcess(&apc);

    RtlCopyMemory(destAddr, buff, size);
    ExFreePool(buff);
    ObDereferenceObject(pTargetProcess);
    return STATUS_SUCCESS;
}

NTSTATUS ReadR3MemoryByCr3(HANDLE pId, PVOID startAddr, ULONG64 size, PVOID destAddr) {
    // 检查传入的参数有效性，避免系统崩溃
    {
        // 判断读取的起始地址是否为驱动层地址
        if ((ULONG64) startAddr > (ULONG64) MmHighestUserAddress ||
            (ULONG64) startAddr + size > (ULONG64) MmHighestUserAddress ||
            (ULONG64) startAddr + size < (ULONG64) startAddr) {
            // 错误：无效的内存地址
            return STATUS_ACCESS_VIOLATION;
        }
        if (0 == size) {
            // 错误：第3个参数无效
            return STATUS_INVALID_PARAMETER_3;
        }
        if (NULL == destAddr) {
            // 错误：第4个参数无效
            return STATUS_INVALID_PARAMETER_4;
        }
    }

    // 获取目标进程 EPROCESS 对象
    PEPROCESS pTargetProcess;
    NTSTATUS status = PsLookupProcessByProcessId(pId, &pTargetProcess);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    if (PsGetProcessExitStatus(pTargetProcess) != 0x103) {
        ObDereferenceObject(pTargetProcess);    // 释放进程引用次数
        // 错误：第1个参数错误--无效的进程ID
        return STATUS_INVALID_PARAMETER_1;
    }

    // 分配临时缓冲区
    PVOID buff = ExAllocatePool(NonPagedPool, size);
    if (NULL == buff) {
        ObDereferenceObject(pTargetProcess);
        return STATUS_UNSUCCESSFUL;
    }

    // 附加到目标进程地址空间
    RtlZeroMemory(buff, size);
    KAPC_STATE apc = {0};
    // 获取当前进程CR3并保存
    ULONG64 curCr3 = __readcr3();
    // 获取目标进程CR3
    ULONG64 targetCr3 = *(ULONG64 * )((ULONG64) pTargetProcess + 0x28);
    // 修改当前进程CR3
    KeEnterCriticalRegion();    // 关闭 APC
    _disable();                 // 关闭中断
    __writecr3(targetCr3);
    if (!(MmIsAddressValid(startAddr) && MmIsAddressValid((ULONG64) startAddr + size))) {
        __writecr3(curCr3);
        KeLeaveCriticalRegion();// 启用 APC
        _enable();              // 启用中断
        ObDereferenceObject(pTargetProcess);
        ExFreePool(buff);
        return STATUS_ACCESS_VIOLATION;
    }

    RtlCopyMemory(buff, startAddr, size);
    RtlCopyMemory(destAddr, buff, size);
    __writecr3(targetCr3);
    KeLeaveCriticalRegion();
    _enable();
    ExFreePool(buff);
    ObDereferenceObject(pTargetProcess);
    return STATUS_SUCCESS;
}

NTSTATUS ReadR3MemoryByVirtualMemory(HANDLE pId, PVOID startAddr, ULONG64 size, PVOID destAddr) {
    // 检查传入的参数有效性，避免系统崩溃
    {
        // 判断读取的起始地址是否为驱动层地址
        if ((ULONG64) startAddr > (ULONG64) MmHighestUserAddress ||
            (ULONG64) startAddr + size > (ULONG64) MmHighestUserAddress ||
            (ULONG64) startAddr + size < (ULONG64) startAddr) {
            // 错误：无效的内存地址
            return STATUS_ACCESS_VIOLATION;
        }
        if (0 == size) {
            // 错误：第3个参数无效
            return STATUS_INVALID_PARAMETER_3;
        }
        if (NULL == destAddr) {
            // 错误：第4个参数无效
            return STATUS_INVALID_PARAMETER_4;
        }
    }

    // 获取目标进程 EPROCESS 对象
    PEPROCESS pTargetProcess;
    NTSTATUS status = PsLookupProcessByProcessId(pId, &pTargetProcess);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    if (PsGetProcessExitStatus(pTargetProcess) != 0x103) {
        ObDereferenceObject(pTargetProcess);    // 释放进程引用次数
        // 错误：第1个参数错误--无效的进程ID
        return STATUS_INVALID_PARAMETER_1;
    }

    // 分配临时缓冲区
    PVOID buff = ExAllocatePool(NonPagedPool, size);
    if (NULL == buff) {
        ObDereferenceObject(pTargetProcess);
        return STATUS_UNSUCCESSFUL;
    }

    // 附加到目标进程地址空间
    RtlZeroMemory(buff, size);
    SIZE_T readSize = 0;
    status = MmCopyVirtualMemory(pTargetProcess, startAddr, IoGetCurrentProcess(), buff, size, UserMode, &readSize);

    ExFreePool(buff);
    ObDereferenceObject(pTargetProcess);
    return status;
}

PVOID RwMapMemory(PVOID toAddress, ULONG buffSize, PMDL *pMdl) {
    // 创建 MDL
    PMDL mdl = NULL;
    mdl = IoAllocateMdl(toAddress, buffSize, FALSE, FALSE, NULL);
    // 锁定内存页
    __try
    {
        MmProbeAndLockPages(mdl, UserMode, IoModifyAccess);
    }
    __except(1)
    {
        DbgPrint("MmProbeAndLockPages Error!");
        IoFreeMdl(mdl);
        return NULL;
    }
    PVOID mappedAddr = NULL;
    __try
    {
        mappedAddr = MmMapLockedPagesSpecifyCache(mdl, KernelMode, MmNonCached, toAddress, FALSE, HighPagePriority);
    }
    __except(1)
    {
        DbgPrint("MmMapLockedPagesSpecifyCache Error!");
        MmUnlockPages(mdl);
        IoFreeMdl(mdl);
        return NULL;
    }
    *pMdl = mdl;
    return mappedAddr;
}

void RwMapUnMemory(PVOID toAddress, PMDL pMdl) {
    MmUnmapLockedPages(toAddress, pMdl);
    MmUnlockPages(pMdl);
    IoFreeMdl(pMdl);
}

NTSTATUS ReadR3MemoryByMdl(HANDLE pId, PVOID startAddr, ULONG64 size, PVOID destAddr) {
    // 检查传入的参数有效性，避免系统崩溃
    {
        // 判断读取的起始地址是否为驱动层地址
        if ((ULONG64) startAddr > (ULONG64) MmHighestUserAddress ||
            (ULONG64) startAddr + size > (ULONG64) MmHighestUserAddress ||
            (ULONG64) startAddr + size < (ULONG64) startAddr) {
            // 错误：无效的内存地址
            return STATUS_ACCESS_VIOLATION;
        }
        if (0 == size) {
            // 错误：第3个参数无效
            return STATUS_INVALID_PARAMETER_3;
        }
        if (NULL == destAddr) {
            // 错误：第4个参数无效
            return STATUS_INVALID_PARAMETER_4;
        }
    }

    // 获取目标进程 EPROCESS 对象
    PEPROCESS pTargetProcess;
    NTSTATUS status = PsLookupProcessByProcessId(pId, &pTargetProcess);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    if (PsGetProcessExitStatus(pTargetProcess) != 0x103) {
        ObDereferenceObject(pTargetProcess);    // 释放进程引用次数
        // 错误：第1个参数错误--无效的进程ID
        return STATUS_INVALID_PARAMETER_1;
    }

    // 分配临时缓冲区
    PVOID buff = ExAllocatePool(NonPagedPool, size);
    if (NULL == buff) {
        ObDereferenceObject(pTargetProcess);
        return STATUS_UNSUCCESSFUL;
    }

    // 附加到目标进程地址空间
    RtlZeroMemory(buff, size);
    KAPC_STATE apc = {0};
    KeStackAttachProcess(pTargetProcess, &apc); // 进程挂靠
    if (!(MmIsAddressValid(startAddr) && MmIsAddressValid((ULONG64) startAddr + size))) {
        ObDereferenceObject(pTargetProcess);
        KeUnstackDetachProcess(&apc);
        ExFreePool(buff);
        return STATUS_ACCESS_VIOLATION;
    }
    PMDL pMdl = NULL;
    PVOID mapAddr = RwMapMemory(startAddr, size, &pMdl);
    if (NULL == mapAddr) {
        ObDereferenceObject(pTargetProcess);
        KeUnstackDetachProcess(&apc);
        ExFreePool(buff);
        return STATUS_ACCESS_VIOLATION;
    }
    RtlCopyMemory(buff, mapAddr, size);
    RwMapUnMemory(mapAddr, pMdl);
    KeUnstackDetachProcess(&apc);

    ExFreePool(buff);
    ObDereferenceObject(pTargetProcess);
    return STATUS_SUCCESS;
}