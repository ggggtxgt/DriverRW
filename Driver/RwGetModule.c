/********************************************************************************************************************
 * @brief   RwGetModule.c
 * @details 获取应用层已加载的指定进程的某个模块
********************************************************************************************************************/

#include "RwGetModule.h"

// 获取某个模块的基地址(32位进程)
ULONG64 RwGetModuleHandle32(HANDLE pid, char* dllName) {
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

    // 判断进程位数（调试用）
    if (PsGetProcessWow64Process(process)) {
        DbgPrint("进程是32位");
    } else {
        DbgPrint("进程是64位");
    }

    // 获取32位PEB指针
    PPEB32 pEb32 = PsGetProcessWow64Process(process);
    if (NULL == pEb32) {
        KeUnstackDetachProcess(&state);
        ObDereferenceObject(process);
        return 0;
    }

    // 锁定内存防止发生页交换
    SIZE_T realRead = 0;
    MmCopyVirtualMemory(process, pEb32, process, pEb32, 1, UserMode, &realRead);

    // 获取 LDR 数据结构指针并遍历模块链表
    PPEB_LDR_DATA32 ldr = (PPEB_LDR_DATA32)pEb32->Ldr;
    PLDR_DATA_TABLE_ENTRY32 listEntry32 = (PLDR_DATA_TABLE_ENTRY32)&ldr->InLoadOrderModuleList;
    PLDR_DATA_TABLE_ENTRY32 listEntryNext = listEntry32->InLoadOrderLinks.Flink;

    // 字符串的数据类型转换
    ANSI_STRING ansiStr = { 0 };
    UNICODE_STRING unicodeStr = { 0 };
    RtlInitAnsiString(&ansiStr, dllName);
    RtlAnsiStringToUnicodeString(&unicodeStr, &ansiStr, TRUE);

    // 遍历模块链表，查找匹配的 DLL
    while (listEntry32 != listEntryNext) {
        PWCHAR wstr = listEntryNext->BaseDllName.Buffer;
        UNICODE_STRING udllName = { 0 };
        RtlInitUnicodeString(&udllName, wstr);
        if (0 == RtlCompareUnicodeString(&unicodeStr, &udllName, TRUE)) {
            dllBase = (ULONG64)listEntryNext->DllBase;
            break;
        }
        listEntryNext = listEntryNext->InLoadOrderLinks.Flink;
    }

    // 释放资源
    RtlFreeUnicodeString(&unicodeStr);
    KeUnstackDetachProcess(&state);
    ObDereferenceObject(process);
    return dllBase;
}

// 获取某个模块的基地址(64位进程)
ULONG64 RwGetModuleHandle64(HANDLE pid, char* dllName) {
    ULONG64 dllBase = 0;

    // 参数检查
    if (NULL == dllName) return 0;

    // 获取目标进程EPROCESS对象
    PEPROCESS process = NULL;
    NTSTATUS status = PsLookupProcessByProcessId(pid, &process);
    if (!NT_SUCCESS(status)) return 0;

    // 附加到目标进程地址空间
    KAPC_STATE state = { 0 };
    KeStackAttachProcess(process, &state);

    // 获取64位PEB指针
    PPEB pPeb64 = (PPEB)PsGetProcessPeb(process);
    if (NULL == pPeb64) {
        KeUnstackDetachProcess(&state);
        ObDereferenceObject(process);
        return 0;
    }

    // 锁定PEB页面（预读一个字节，防止缺页）
    SIZE_T realRead = 0;
    MmCopyVirtualMemory(process, pPeb64, process, pPeb64, 1, UserMode, &realRead);

    // 获取 LDR 数据结构指针并遍历模块链表
    PPEB_LDR_DATA ldr = (PPEB_LDR_DATA)pPeb64->Ldr;
    PLDR_DATA_TABLE_ENTRY listHead = (PLDR_DATA_TABLE_ENTRY)&ldr->InLoadOrderModuleList;
    PLDR_DATA_TABLE_ENTRY current = (PLDR_DATA_TABLE_ENTRY)listHead->InLoadOrderLinks.Flink;

    // 将输入模块名转换为 UNICODE_STRING
    ANSI_STRING ansiStr = { 0 };
    UNICODE_STRING unicodeStr = { 0 };
    RtlInitAnsiString(&ansiStr, dllName);
    RtlAnsiStringToUnicodeString(&unicodeStr, &ansiStr, TRUE);

    // 遍历模块链表，查找匹配的 DLL
    while (listHead != current) {
        UNICODE_STRING udllName;
        RtlInitUnicodeString(&udllName, current->BaseDllName.Buffer);

        if (0 == RtlCompareUnicodeString(&unicodeStr, &udllName, TRUE)) {
            dllBase = (ULONG64)current->DllBase;
            break;
        }
        current = (PLDR_DATA_TABLE_ENTRY)current->InLoadOrderLinks.Flink;
    }
    // 释放资源
    RtlFreeUnicodeString(&unicodeStr);
    KeUnstackDetachProcess(&state);
    ObDereferenceObject(process);
    return dllBase;
}