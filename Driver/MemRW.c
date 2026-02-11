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