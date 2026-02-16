#pragma once

#include <ntifs.h>
#include "Struct.h"

/********************************************************************************************************************
 * @brief   自定义属性信息回调函数类型：
 *			当应用层通过ExQueryAttributeInformation等API时，如果该属性已注册回调，系统将调用此函数；
 * @param   与查询请求相关的句柄；
 * @param   查询请求的附加参数；
 * @return  NTSTATUS类型的状态码，表示驱动是否加载成功；
********************************************************************************************************************/
typedef NTSTATUS(*MyAttributeInofrmationCallback)(HANDLE handle, PVOID arg);

/********************************************************************************************************************
 * @brief   注册自定义属性信息回调函数；
 *			此函数通过调用 ExRegisterAttributeInformationCallback 注册自定义回调，
 *			用于处理特定属性类（如AttributeMyCustomType）的属性信息查询；
********************************************************************************************************************/
NTSTATUS RegisterCallBack();

// 卸载回调函数 - 恢复原始回调函数
VOID UnRegisterCallback();

// 读写回调函数结构体
typedef struct _RWCALL_BACK_FUNC {
    MyAttributeInofrmationCallback ExDisSetAttributeInformation;    // 设置属性信息回调函数
    MyAttributeInofrmationCallback ExDisQueryAttributeInformation;  // 查询属性信息回调函数
} RWCALL_BACK_FUNC, * PRWCALL_BACK_FUNC;
// 注册属性信息回调函数的函数指针类型
// 指向ExRegisterAttributeInformationCallback函数的指针
typedef NTSTATUS(*_ExRegisterAttributeInfomationCallback)(PRWCALL_BACK_FUNC arg);

// 驱动-应用层通信的数据包结构
typedef struct _MESSAGE_PACKAGE {
    ULONG64 func;   // 需要调用的函数（需要完成的操作）
    ULONG64 flag;   // 通信标志（用于区分是否为自定义通信）
    ULONG64 data;   // 通信传递的数据（通常为内存地址）
    ULONG64 size;   // 数据的长度
    ULONG result;   // 执行结果（返回应用层）
} MESSAGE_PACKAGE, * PMESSAGE_PACKAGE;

// 根据需要完成的操作，分发不同类型的回调函数
NTSTATUS DispatchCallbackEntry(PMESSAGE_PACKAGE message);

// 用于读写指定进程内存时所有参数
typedef struct _RWMM {
    ULONG64 pid;    // 进程号
    ULONG64 start;  // 读取的起始地址
    ULONG64 size;   // 读取的大小
    ULONG64 dest;   // 存储已读取内存的缓冲区
} RWMM, * PRWMM;

// 内存属性查询
NTSTATUS RwQueryVirtualMemory(HANDLE pId, ULONG64 baseAddr, PMYMEMORY_BASIC_INFORMATION baseInformation);
// 注册回调以实现进程保护
void PobPostOperationCallback(PVOID RegistrationContext, POB_POST_OPERATION_INFORMATION OperationInformation);
OB_PREOP_CALLBACK_STATUS PobPreOperationCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation);
void ObUnRegister();
// 注册回调实现进程保护
NTSTATUS ProcessProtected(PDRIVER_OBJECT pDriver);

// 通过修改进程对象头实现进程保护
NTSTATUS EditHeaderProtected(HANDLE pid);