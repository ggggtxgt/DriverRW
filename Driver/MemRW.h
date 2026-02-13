#pragma once

#include "KernelAPI.h"

// 辅助函数 - 检查读取内存时所需参数是否正确
NTSTATUS CheckParams(PVOID start, ULONG64 size, PVOID dest);
// 辅助函数 - 根据PID获取PEPROCESS并验证进程存活
NTSTATUS GetTargetProcess(HANDLE pid, PEPROCESS* process);
// 辅助函数 - 分配并清零非分页内核缓冲区
PVOID AllocateAndZeroBuffer(ULONG64 size, NTSTATUS* status);
// 辅助函数 - 释放内核缓冲区
VOID FreeKernelBuffer(PVOID buffer);

/********************************************************************************************************************
 * @brief   以进程挂靠读取指定进程的应用层内存
 * @param   需要读取内存的进程ID（PID）
 * @param   需要读取的内存起始地址
 * @param   需要读取的内存大小（字节数）
 * @param   接收读取的数据缓冲区
 * @return  NTSTATUS类型的状态码
********************************************************************************************************************/
NTSTATUS ReadR3Memory(HANDLE pid, PVOID start, ULONG64 size, PVOID dest);
// 通过修改CR3读取指定进程的内存
NTSTATUS ReadR3MemoryByCr3(HANDLE pid, PVOID start, ULONG64 size, PVOID dest);
// 通过复制虚拟内存(MmCopyVirtualMemory)读取指定进程的内存
NTSTATUS ReadR3MemoryByVirtualMemory(HANDLE pid, PVOID start, ULONG64 size, PVOID dest);
// 辅助函数 - 通过MDL读取内存相关操作
PVOID RwMapMemory(PVOID toAddress, ULONG buffSize, PMDL* pMdl);
void RwMapUnMemory(PVOID toAddress, PMDL pMdl);
// 通过MDL读取进程内存
NTSTATUS ReadR3MemoryByMdl(HANDLE pid, PVOID start, ULONG64 size, PVOID dest);