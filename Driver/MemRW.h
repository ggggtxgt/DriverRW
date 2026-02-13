#pragma once

#include <ntifs.h>

/********************************************************************************************************************
 * @brief   以进程挂靠读取指定进程的应用层内存
 * @details 需要读取内存的进程ID（PID）
 * @param   需要读取的内存起始地址
 * @param   需要读取的内存大小（字节数）
 * @param   接收读取的数据缓冲区
 * @return  NTSTATUS类型的状态码
********************************************************************************************************************/
NTSTATUS ReadR3Memory(HANDLE pid, PVOID start, ULONG64 size, PVOID dest);
// 通过修改CR3读取指定进程的内存
NTSTATUS ReadR3MemoryByCr3(HANDLE pId, PVOID startAddr, ULONG64 size, PVOID destAddr);
// 通过复制虚拟内存(MmCopyVirtualMemory)读取指定进程的内存
NTSTATUS ReadR3MemoryByVirtualMemory(HANDLE pId, PVOID startAddr, ULONG64 size, PVOID destAddr) {
// 通过MDL读取进程内存
NTSTATUS ReadR3MemoryByMdl(HANDLE pId, PVOID startAddr, ULONG64 size, PVOID destAddr)