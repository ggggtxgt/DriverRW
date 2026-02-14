/********************************************************************************************************************
 * @brief   KernelAPI.h
 * @details Windows 内核之中未导出的函数声明(未文档化的函数)；
********************************************************************************************************************/

#pragma once
#include <ntifs.h>

/********************************************************************************************************************
 * @brief   跨进程虚拟地址空间内存复制
 * @param   源进程的 PEPROCESS 指针，数据从此进程读取
 * @param   源进程内的虚拟地址（通常为用户空间地址）
 * @param   目标进程的 PEPROCESS 指针，数据写入此进程
 * @param   目标进程内的虚拟地址（若目标为内核空间，该进程可为任意进程）
 * @param   需要复制的字节数
 * @param   访问模式：决定地址合法性检查的严格程度
 * @return  NTSTATUS类型的状态码
********************************************************************************************************************/
NTSTATUS MmCopyVirtualMemory(
    IN PEPROCESS FromProcess,
    IN CONST VOID* FromAddress,
    IN PEPROCESS ToProcess,
    OUT PVOID ToAddress,
    IN SIZE_T BufferSize,
    IN KPROCESSOR_MODE PreviousMode,
    OUT PSIZE_T NumberOfBytesCopied
);

// 获取32位程序PEB
void* PsGetProcessWow64Process(PEPROCESS process); 
// 获取64位程序PEB
void* PsGetProcessPeb(PEPROCESS process);