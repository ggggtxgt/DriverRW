#pragma once
#include <ntifs.h>

// 内核模块数据表条目结构（KLDR_DATA_TABLE_ENTRY）
typedef struct _KLDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;    // 按加载顺序链接所有模块的双向链表指针
    PVOID ExceptionTable;           // 指向异常处理表（通常为 NULL）
    ULONG ExceptionTableSize;       // 异常处理表的大小（字节）
    PVOID GpValue;                  // 全局指针值
    ULONG64 NonPagedDebugInfo;      // 非分页调试信息指针（未文档化）
    PVOID DllBase;                  // 模块在系统空间中的加载基址
    PVOID EntryPoint;               // 模块入口点地址
    ULONG SizeOfImage;              // 模块映像大小（字节）
    UNICODE_STRING FullDllName;     // 模块的完整路径名
    UNICODE_STRING BaseDllName;     // 模块的基本文件名
    ULONG Flags;                    // 模块标志
    USHORT LoadCount;               // 模块的加载计数
    USHORT __Unused5;               // 未使用的字段，可能是填充
    PVOID SectionPointer;           // 指向内存区段对象的指针
    ULONG CheckSum;                 // 映像校验和
    PVOID LoadedImports;            // 已加载导入模块的信息
    PVOID PatchInformation;         // 修补信息（通常为 NULL）
} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;

// 注册回调以实现进程保护
void PobPostOperationCallback(PVOID RegistrationContext, POB_POST_OPERATION_INFORMATION OperationInformation);
OB_PREOP_CALLBACK_STATUS PobPreOperationCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation);
void ObUnRegister();
// 注册回调实现进程保护
NTSTATUS ProcessProtected(PDRIVER_OBJECT pDriver);

// 通过修改进程对象头实现进程保护
NTSTATUS EditHeaderProtected(HANDLE pid);

// 隐藏回调保护
NTSTATUS InstallCallback();
void UninstallCallback();