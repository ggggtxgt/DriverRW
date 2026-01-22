#pragma once

#include "WindowsApi.h"

#include <ntifs.h>

/******************************************************************************
 * @brief 读取指定进程的用户层内存
 * @param pId       目标进程的进程ID（PID）
 * @param startAddr 要读取的内存起始地址（用户层地址）
 * @param size      要读取的内存大小（字节数）
 * @param destAddr  驱动层缓冲区地址，用于接收读取的数据
 * @return NTSTATUS 状态码，STATUS_SUCCESS表示成功
*****************************************************************************/
NTSTATUS ReadR3Memory(HANDLE pId, PVOID startAddr, ULONG64 size, PVOID destAddr);
NTSTATUS ReadR3MemoryByCr3(HANDLE pId, PVOID startAddr, ULONG64 size, PVOID destAddr);
NTSTATUS ReadR3MemoryByVirtualMemory(HANDLE pId, PVOID startAddr, ULONG64 size, PVOID destAddr);