#pragma once

#include <ntifs.h>

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