#include "WindowsApi.h"
#include "RwGetModule.h"

ULONG64 RwGetModuleHandle(HANDLE pId, char *dllName) {
    ULONG64 dllBase = 0;

    // 参数检查
    if (NULL == dllName) {
        return 0;
    }

    // 通过进程ID获取目标进程的EPROCESS对象
    PEPROCESS process = NULL;
    NTSTATUS status = PsLookupProcessByProcessId(pId, &process);
    if (!NT_SUCCESS(status)) {
        return 0;
    }

    // 附加到目标进程地址空间
    KAPC_STATE state = {0};
    KeStackAttachProcess(process, &state);  // 进程挂靠

    // 获取 PEB 地址
    PPEB32 pEb32 = PsGetProcessWow64Process(process);
    if (NULL == pEb32) {
        KeUnstackDetachProcess(&state);
        ObDereferenceObject(process);
        return 0;
    }

    // PPEB64 pEb64 = PsGetProcesspeb(process);

    // 锁定内存
    SIZE_T realRead = 0;
    MmCopyVirtualMemory(process, pEb32, process, pEb32, 1, UserMode, &realRead);    // 主要用于锁定内存

    // 获取 LDR 数据结构指针并遍历模块链表
    PPEB_LDR_DATA32 ldr = (PPEB_LDR_DATA32) pEb32->Ldr;
    PLDR_DATA_TABLE_ENTRY32 listEntry32 = (PLDR_DATA_TABLE_ENTRY32) &ldr->InLoadOrderModuleList;
    PLDR_DATA_TABLE_ENTRY32 listEntryNext = listEntry32->InLoadOrderLinks.Blink;

    // 字符串的数据类型转换
    ANSI_STRING ansiStr = {0};
    UNICODE_STRING unicodeStr = {0};
    RtlInitAnsiString(&ansiStr, dllName);
    RtlAnsiStringToUnicodeString(&unicodeStr, &ansiStr, TRUE);

    // 遍历模块链表，查找匹配的 DLL
    while (listEntry32 != listEntryNext) {
        PWCHAR wstr = listEntryNext->BaseDllName.Buffer;
        UNICODE_STRING udllName = {0};
        RtlInitUnicodeString(&udllName, wstr);
        if (0 == RtlCompareUnicodeString(&unicodeStr, &udllName, TRUE)) {
            dllBase = (ULONG64) listEntryNext->DllBase;
            break;
        }
        listEntryNext = listEntryNext->InLoadOrderLinks.Flink;
    }

    // 释放资源
    KeUnstackDetachProcess(&state);         // 取消进程挂靠
    ObDereferenceObject(process);
    return dllBase;
}