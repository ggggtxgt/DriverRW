#include "MemoryRW.h"

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
        ObDereferenceObject(pTargetProcess);
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
    KeStackAttachProcess(pTargetProcess, &apc);
    if (!(MmIsAddressValid(startAddr) && MmIsAddressValid((ULONG64) startAddr + size))) {
        ObDereferenceObject(pTargetProcess);
        return STATUS_ACCESS_VIOLATION;
    }
    RtlCopyMemory(buff, startAddr, size);
    KeUnstackDetachProcess(&apc);

    RtlCopyMemory(destAddr, buff, size);
    ExFreePool(buff);
    ObDereferenceObject(pTargetProcess);
    return STATUS_SUCCESS;
}