#pragma once
#include <ntifs.h>

#include "structure.h"

// 自定义回调函数
typedef NTSTATUS(*MyAttributeInofrmationCallback)(HANDLE handle, PVOID arg);
// 注册回调函数
NTSTATUS RegisterCallBack();
// 卸载回调函数
void UnRegisterCallBack();

// 用于读写内存
typedef struct _RWMM {
    ULONG64 pId;        // 进程号
    ULONG64 startAddr;  // 读取的起始地址
    ULONG64 size;       // 读取的大小
    ULONG64 destAddr;   // 存储已读取内存的缓冲区
}RWMM, *PRWMM;

// 驱动-应用层通信的数据包结构
typedef struct _MESSAGE_PACKAGE {
    ULONG64 func;   // 需要调用的函数（需要完成的操作）
    ULONG64 falg;   // 通信标志（用于区分是否为自定义通信）
    ULONG64 data;   // 通信传递的数据（通常为内存地址）
    ULONG64 size;   // 数据的长度
    ULONG result;   // 执行结果（返回应用层）
}MESSAGE_PACKAGE, *PMESSAGE_PACKAGE;

// *********************************************************************************
// 自定义读写回调函数
typedef struct _RWCALL_BACK_FUNC {
    MyAttributeInofrmationCallback ExDisSetAttributeInformation;
    MyAttributeInofrmationCallback ExDisQueryAttributeInformation;
}RWCALL_BACK_FUNC, *PRWCALL_BACK_FUNC;
typedef NTSTATUS(*_ExRegisterAttributeInfomationCallback)(PRWCALL_BACK_FUNC arg);
// *********************************************************************************

// 分配各种回调函数
NTSTATUS DispatchCallEntry(PMESSAGE_PACKAGE package);

NTSTATUS RwQueryVirtualMemory(HANDLE pId, ULONG64 baseAddr, PMYMEMORY_BASIC_INFORMATION baseInformation);