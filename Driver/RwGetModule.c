/********************************************************************************************************************
 * @brief   RwGetModule.c
 * @details 获取应用层已加载的指定进程的某个模块
********************************************************************************************************************/

#include "RwGetModule.h"

// 获取某个模块的基地址
ULONG64 RwGetModuleHandle(HANDLE pid, char* dllName) {
    ULONG64 dllBase = 0;

    // 参数检查：模块名不能为空
    if (NULL == dllName) return 0;

    // 通过进程ID获取目标进程的EPROCESS内核对象
    PEPROCESS process = NULL;
    NTSTATUS status = PsLookupProcessByProcessId(pid, &process);
    if (!NT_SUCCESS(status)) return 0;

    // 附加到目标进程地址空间
    KAPC_STATE state = { 0 };
    KeStackAttachProcess(process, &state);

    // 获取 PEB 地址
    PPEB32 pEb32 = PsGetProcessWow64Process(process);
    if (NULL == pEb32) {
        KeUnstackDetachProcess(&state);
        ObDereferenceObject(process);
        return 0;
    }

    // PPEB64 pEb64 = PsGetProcesspeb(process);

    // 尝试读取PEB的第一个字节，以触发页面错误并确保PEB所在页面被加载到内存中
    SIZE_T realRead = 0;
    MmCopyVirtualMemory(process, pEb32, process, pEb32, 1, UserMode, &realRead);    // 主要用于锁定内存

    // 从PEB中获取LDR数据结构指针
    PPEB_LDR_DATA32 ldr = (PPEB_LDR_DATA32)pEb32->Ldr;
    // 获取按加载顺序排列的模块链表头
    PLDR_DATA_TABLE_ENTRY32 listEntry32 = (PLDR_DATA_TABLE_ENTRY32)&ldr->InLoadOrderModuleList;
    // 获取第一个实际模块节点（头节点的Blink指向最后一个模块）
    PLDR_DATA_TABLE_ENTRY32 listEntryNext = listEntry32->InLoadOrderLinks.Blink;

    // 将输入模块名转换为 UNICODE_STRING
    ANSI_STRING ansiStr = { 0 };
    UNICODE_STRING unicodeStr = { 0 };
    RtlInitAnsiString(&ansiStr, dllName);
    status = RtlAnsiStringToUnicodeString(&unicodeStr, &ansiStr, TRUE);
    if (!NT_SUCCESS(status)) {
        KeUnstackDetachProcess(&state);
        ObDereferenceObject(process);
        return 0;
    }

    __try {
        // 验证 PEB 指针本身
        ProbeForRead(pEb32, 1, 1);
        // 获取 LDR 指针并验证
        PPEB_LDR_DATA32 ldr = (PPEB_LDR_DATA32)pEb32->Ldr;
        ProbeForRead(ldr, sizeof(PEB_LDR_DATA32), 1);
        // 获取链表头并验证
        PLDR_DATA_TABLE_ENTRY32 listEntry32 = (PLDR_DATA_TABLE_ENTRY32)&ldr->InLoadOrderModuleList;
        ProbeForRead(listEntry32, sizeof(LDR_DATA_TABLE_ENTRY32), 1);  // 验证链表头结构
        PLDR_DATA_TABLE_ENTRY32 listEntryNext = listEntry32->InLoadOrderLinks.Blink;
        ULONG maxModules = 1024;
        while (listEntry32 != listEntryNext && maxModules-- > 0) {
            ProbeForRead(listEntryNext, sizeof(LDR_DATA_TABLE_ENTRY32), 1);
            ProbeForRead(listEntryNext->BaseDllName.Buffer,
            listEntryNext->BaseDllName.Length, 1);
            UNICODE_STRING udllName = { 0 };
            RtlInitUnicodeString(&udllName, listEntryNext->BaseDllName.Buffer);
            if (0 == RtlCompareUnicodeString(&unicodeStr, &udllName, TRUE)) {
                dllBase = (ULONG64)listEntryNext->DllBase;
                break;
            }
            listEntryNext = listEntryNext->InLoadOrderLinks.Flink;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        dllBase = 0;
        DbgPrint("RwGetModuleHandle: Exception occurred, code=0x%X\n", GetExceptionCode());
    }

    // 释放资源
    RtlFreeUnicodeString(&unicodeStr);
    KeUnstackDetachProcess(&state);
    ObDereferenceObject(process);
    return dllBase;
}