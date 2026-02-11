#pragma once
#include <iostream>
#include <Windows.h>

// 异步IO状态块 - 用于接收异步IO操作的状态和结果信息
typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;     // IO 操作状态码
        PVOID    Pointer;    // 用于返回的指针
    };
    ULONG_PTR Information;   // 操作返回的附加信息
} IO_STATUS_BLOCK, * PIO_STATUS_BLOCK;

// NtQueryInformationFile 函数原型
// 其位于 ntdll.dll 之中，不能直接调用，需要先在指定模块中找到其函数地址
/********************************************************************************************************************
 * @brief   NtQueryInformationFile函数指针类型定义
 * @details NtQueryInformationFile是Windows Native API函数，用于查询文件信息，
 *          该函数位于ntdll.dll中，是用户模式进入内核模式的桥梁；
 * @param   需要查询的文件句柄；
 * @param   指向IO状态块的指针，接收操作状态；
 * @param   指向输出缓冲区的指针，接收查询结果；
 * @param   输出缓冲区的大小（字节）；
 * @param   需要查询的信息类型（实际为枚举值）；
 * @return  NTSTATUS类型的状态码
********************************************************************************************************************/
typedef LONG(*_NtQueryInformationFile)(
    HANDLE FileHandle,               // 文件句柄
    PIO_STATUS_BLOCK IoStatusBlock,  // IO状态块指针
    PVOID FileInformation,           // 输出缓冲区
    ULONG Length,                    // 缓冲区大小
    LONG FileInformationClass        // 文件信息类（枚举值）
);

// 驱动-应用层通信的数据包结构
typedef struct _MESSAGE_PACKAGE {
    ULONG64 func;   // 需要调用的函数（需要完成的操作）
    ULONG64 flag;   // 通信标志（用于区分是否为自定义通信）
    ULONG64 data;   // 通信传递的数据（通常为内存地址）
    ULONG64 size;   // 数据的长度
    ULONG result;   // 执行结果
} MESSAGE_PACKAGE, * PMESSAGE_PACKAGE;
