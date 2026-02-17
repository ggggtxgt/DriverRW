#pragma once
#include <ntifs.h>

// 自定义的内存基本信息结构，用于查询进程虚拟内存区域信息
typedef struct _MY_MEMORY_BASIC_INFORMATION {
    ULONG64 BaseAddress;          // 内存区域的起始基址
    ULONG64 AllocationBase;       // 分配区域的起始基址
    ULONG64 AllocationProtect;    // 最初分配时的保护属性
    ULONG64 RegionSize;           // 内存区域的大小
    ULONG64 State;                // 内存区域的状态
    ULONG64 Protect;              // 当前内存区域的保护属性
    ULONG64 Type;                 // 内存区域的类型
} MYMEMORY_BASIC_INFORMATION, * PMYMEMORY_BASIC_INFORMATION;

