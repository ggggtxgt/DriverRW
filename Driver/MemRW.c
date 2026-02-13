/********************************************************************************************************************
 * @brief   MemRW.c
 * @details 主要实现进程内存的读写操作；
********************************************************************************************************************/

#include "MemRW.h"

// 以进程挂靠读取指定进程的应用层内存
NTSTATUS ReadR3Memory(HANDLE pId, PVOID start, ULONG64 size, PVOID dest) {
    // 检查传入的参数有效性，避免系统崩溃
    {
        // 判断读取的起始地址是否为驱动层地址
        if ((ULONG64)start > (ULONG64)MmHighestUserAddress ||
            (ULONG64)start + size > (ULONG64)MmHighestUserAddress ||
            (ULONG64)start + size < (ULONG64)start) {
            return STATUS_ACCESS_VIOLATION;     // 无效地址
        }
        if (0 == size) {
            return STATUS_INVALID_PARAMETER_3;  // 参数3无效
        }
        if (NULL == dest) {
            return STATUS_INVALID_PARAMETER_4;  // 参数4无效
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
        return STATUS_INVALID_PARAMETER_1;      // 参数1错误：进程已终止或状态异常
    }

    // 分配临时缓冲区
    PVOID buff = ExAllocatePool(NonPagedPool, size);
    if (NULL == buff) {
        ObDereferenceObject(pTargetProcess);
        return STATUS_UNSUCCESSFUL;
    }
    RtlZeroMemory(buff, size);

    // 附加到目标进程地址空间
    KAPC_STATE apc = { 0 };
    // 进程挂靠：切换至目标进程CR3
    KeStackAttachProcess(pTargetProcess, &apc);
    // 验证用户地址是否可读
    if (!(MmIsAddressValid(start) && MmIsAddressValid((ULONG64)start + size))) {
        KeUnstackDetachProcess(&apc);
        ObDereferenceObject(pTargetProcess);
        ExFreePool(buff);
        return STATUS_ACCESS_VIOLATION;
    }

    // 从用户地址复制数据到缓冲区
    RtlCopyMemory(buff, start, size);
    // 取消挂靠：脱离目标进程地址空间
    KeUnstackDetachProcess(&apc);
    // 将数据从内核临时缓冲区复制到调用者提供的目标缓冲区
    RtlCopyMemory(dest, buff, size);
    // 释放资源，返回成功
    ExFreePool(buff);
    ObDereferenceObject(pTargetProcess);
    return STATUS_SUCCESS;
}

// 通过修改CR3读取指定进程的内存
NTSTATUS ReadR3MemoryByCr3(HANDLE pId, PVOID startAddr, ULONG64 size, PVOID destAddr) {
    // 检查传入的参数有效性，避免系统崩溃
    {
        // 判断读取的起始地址是否为驱动层地址
        if ((ULONG64)startAddr > (ULONG64)MmHighestUserAddress ||
            (ULONG64)startAddr + size > (ULONG64)MmHighestUserAddress ||
            (ULONG64)startAddr + size < (ULONG64)startAddr) {
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
    KAPC_STATE apc = { 0 };
    // 获取当前进程CR3并保存
    ULONG64 curCr3 = __readcr3();
    // 获取目标进程CR3
    ULONG64 targetCr3 = *(ULONG64*)((ULONG64)pTargetProcess + 0x28);
    // 修改当前进程CR3
    KeEnterCriticalRegion();    // 关闭 APC
    _disable();                 // 关闭中断
    __writecr3(targetCr3);
    if (!(MmIsAddressValid(startAddr) && MmIsAddressValid((ULONG64)startAddr + size))) {
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

// 通过虚拟内存(MmCopyVirtualMemory)读取指定进程内存
NTSTATUS ReadR3MemoryByVirtualMemory(HANDLE pId, PVOID startAddr, ULONG64 size, PVOID destAddr) {
    // 检查传入的参数有效性，避免系统崩溃
    {
        // 判断读取的起始地址是否为驱动层地址
        if ((ULONG64)startAddr > (ULONG64)MmHighestUserAddress ||
            (ULONG64)startAddr + size > (ULONG64)MmHighestUserAddress ||
            (ULONG64)startAddr + size < (ULONG64)startAddr) {
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

// 通过MDL读取进程内存
NTSTATUS ReadR3MemoryByMdl(HANDLE pId, PVOID startAddr, ULONG64 size, PVOID destAddr) {
    // 检查传入的参数有效性，避免系统崩溃
    {
        // 判断读取的起始地址是否为驱动层地址
        if ((ULONG64)startAddr > (ULONG64)MmHighestUserAddress ||
            (ULONG64)startAddr + size > (ULONG64)MmHighestUserAddress ||
            (ULONG64)startAddr + size < (ULONG64)startAddr) {
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
    KAPC_STATE apc = { 0 };
    KeStackAttachProcess(pTargetProcess, &apc); // 进程挂靠
    if (!(MmIsAddressValid(startAddr) && MmIsAddressValid((ULONG64)startAddr + size))) {
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