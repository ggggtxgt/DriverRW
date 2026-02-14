#pragma once
#include <ntifs.h>

// 32位进程的进程环境块(PEB)结构
typedef struct _PEB32 {
    UCHAR InheritedAddressSpace;                    // 指示进程是否继承自父进程的地址空间
    UCHAR ReadImageFileExecOptions;                 // 指示映像文件执行选项是否被读取
    UCHAR BeingDebugged;                            // 进程是否处于被调试状态（1表示被调试）
    union {
        UCHAR BitField;                             // 位字段组合
        struct {  
            UCHAR IsImageUserLargePages : 1;        // 映像是否使用大页面
            UCHAR IsProtectedProcess : 1;           // 是否为受保护进程
            UCHAR IsLegacyProcess : 1;              // 是否为遗留进程（如WoW64）
            UCHAR IsImageDynamicallyRelocated : 1;  // 映像是否已动态重定位
            UCHAR SkipPatchingUser32Forwarders : 1; // 是否跳过修补user32转发器
            UCHAR SpareBits : 3;                    // 保留位
        };
    };
    ULONG Mutant;                                   // 用于调试的互斥体句柄
    ULONG ImageBaseAddress;                         // 进程主映像的加载基址（模块基址）
    ULONG Ldr;                                      // 指向PEB_LDR_DATA32结构的指针，包含模块加载信息
} PEB32, * PPEB32;

// 32位进程的加载器数据结构
typedef struct _PEB_LDR_DATA32 {
    ULONG Length;                               // 本结构的大小（字节）
    UCHAR Initialized;                          // 加载器是否已初始化
    ULONG SsHandle;                             // 会话句柄（未文档化）
    LIST_ENTRY32 InLoadOrderModuleList;         // 按加载顺序排列的模块链表头
    LIST_ENTRY32 InMemoryOrderModuleList;       // 按内存顺序排列的模块链表头
    LIST_ENTRY32 InInitializedOrderModuleList;  // 按初始化顺序排列的模块链表头
    ULONG EntryInProcess;                       // 指向进程入口点信息（未文档化）
    UCHAR ShutdownInProgress;                   // 进程是否正在关闭
    ULONG ShutdownThreadId;                     // 执行关闭操作的线程ID
} PEB_LDR_DATA32, * PPEB_LDR_DATA32;

// 32位进程的模块加载表项
typedef struct _LDR_DATA_TABLE_ENTRY32 {
    LIST_ENTRY32 InLoadOrderLinks;              // 按加载顺序的链表项（指向下一个/上一个模块）
    LIST_ENTRY32 InMemoryOrderLinks;            // 按内存顺序的链表项
    LIST_ENTRY32 InInitializedOrderLinks;       // 按初始化顺序的链表项
    ULONG DllBase;                              // 模块在进程空间中的基址（加载地址）
    ULONG EntryPoint;                           // 模块入口点地址（通常为DllMain）
    ULONG SizeOfImage;                          // 模块映像大小（字节）
    UNICODE_STRING32 FullDllName;               // 模块完整路径（如 "C:\Windows\System32\kernel32.dll"）
    UNICODE_STRING32 BaseDllName;               // 模块短名称（如 "kernel32.dll"）
    ULONG Flags;                                // 模块标志位（如是否已加载、是否固定等）
    USHORT LoadCount;                           // 加载计数（模块被引用的次数）
    USHORT TlsIndex;                            // 线程局部存储索引
    union {
        LIST_ENTRY32 HashLinks;                 // 用于哈希链表的项
        struct {
            ULONG SectionPointer;               // 指向内存区段对象的指针
            ULONG CheckSum;                     // 映像校验和
        };
    };
    union {
        ULONG TimeDateStamp;                     // 文件时间戳（编译时间）
        ULONG LoadedImports;                     // 已导入模块的标记
    };
    ULONG EntryPointActivationContext;           // 入口点激活上下文
    ULONG PatchInformaion;
    LIST_ENTRY32 ForwarderLinks;                 // 用于转发器的链表
    LIST_ENTRY32 ServiceTagLinks;                // 服务标签链表
    LIST_ENTRY32 StaticLinks;                    // 静态链接的模块链表
    ULONG ContextInformation;                    // 上下文信息（未文档化）
    ULONG OriginalBase;                          // 原始基址（若已重定位）
    LARGE_INTEGER LoadTime;                      // 模块加载时间（系统启动以来的时间）
} LDR_DATA_TABLE_ENTRY32, * PLDR_DATA_TABLE_ENTRY32;

// 获取某个模块的基地址(32位进程)
ULONG64 RwGetModuleHandle32(HANDLE pid, char* dllName);

/********************************************************************************************************************/
// 64位进程的进程环境块(PEB)结构
typedef struct _PEB {
    UCHAR InheritedAddressSpace;                    // 指示进程是否继承自父进程的地址空间
    UCHAR ReadImageFileExecOptions;                 // 指示映像文件执行选项是否被读取
    UCHAR BeingDebugged;                            // 进程是否处于被调试状态（1表示被调试）
    union {
        UCHAR BitField;                             // 位字段组合
        struct {
            UCHAR IsImageUserLargePages : 1;        // 映像是否使用大页面
            UCHAR IsProtectedProcess : 1;           // 是否为受保护进程
            UCHAR IsLegacyProcess : 1;              // 是否为遗留进程（如WoW64）
            UCHAR IsImageDynamicallyRelocated : 1;  // 映像是否已动态重定位
            UCHAR SkipPatchingUser32Forwarders : 1; // 是否跳过修补user32转发器
            UCHAR SpareBits : 3;                    // 保留位
        };
    };
    VOID* Mutant;                                   // 用于调试的互斥体句柄
    VOID* ImageBaseAddress;                         // 进程主映像的加载基址（模块基址）
    struct _PEB_LDR_DATA* Ldr;                      // 指向PEB_LDR_DATA结构的指针，包含模块加载信息
} PEB, * PPEB;

// 64位进程的加载器数据结构
typedef struct _PEB_LDR_DATA {
    ULONG Length;                                   // 本结构的大小（字节）
    UCHAR Initialized;                              // 加载器是否已初始化
    VOID* SsHandle;                                 // 会话句柄（未文档化）
    struct _LIST_ENTRY InLoadOrderModuleList;       // 按加载顺序排列的模块链表头
    struct _LIST_ENTRY InMemoryOrderModuleList;     // 按内存顺序排列的模块链表头
    struct _LIST_ENTRY InInitializedOrderModuleList;// 按初始化顺序排列的模块链表头
    VOID* EntryInProcess;                           // 指向进程入口点信息（未文档化）
    UCHAR ShutdownInProgress;                       // 进程是否正在关闭
    VOID* ShutdownThreadId;                         // 执行关闭操作的线程ID
} PEB_LDR_DATA, * PPEB_LDR_DATA;

// 64位进程的模块加载表项
typedef struct _LDR_DATA_TABLE_ENTRY {
    struct _LIST_ENTRY InLoadOrderLinks;            // 按加载顺序的链表项（指向下一个/上一个模块）
    struct _LIST_ENTRY InMemoryOrderLinks;          // 按内存顺序的链表项
    struct _LIST_ENTRY InInitializedOrderLinks;     // 按初始化顺序的链表项
    VOID* DllBase;                                  // 模块在进程空间中的基址（加载地址）
    VOID* EntryPoint;                               // 模块入口点地址（通常为DllMain）
    ULONG SizeOfImage;                              // 模块映像大小（字节）
    struct _UNICODE_STRING FullDllName;             // 模块完整路径
    struct _UNICODE_STRING BaseDllName;             // 模块短名称
    ULONG Flags;                                    // 模块标志位（如是否已加载、是否固定等）
    USHORT LoadCount;                               // 加载计数（模块被引用的次数）
    USHORT TlsIndex;                                // 线程局部存储索引
    union {
        struct _LIST_ENTRY HashLinks;               // 用于哈希链表的项
        struct {
            VOID* SectionPointer;                   // 指向内存区段对象的指针
            ULONG CheckSum;                         // 映像校验和
        };
    };
    union {
        ULONG TimeDateStamp;                        // 文件时间戳（编译时间）
        VOID* LoadedImports;                        // 已导入模块的标记
    };
    struct _ACTIVATION_CONTEXT* EntryPointActivationContext;// 入口点激活上下文
    VOID* PatchInformaion;
    struct _LIST_ENTRY ForwarderLinks;               // 用于转发器的链表
    struct _LIST_ENTRY ServiceTagLinks;              // 服务标签链表
    struct _LIST_ENTRY StaticLinks;                  // 静态链接的模块链表
    VOID* ContextInformation;                        // 上下文信息（未文档化）
    ULONG OriginalBase;                              // 原始基址（若已重定位）
    union _LARGE_INTEGER LoadTime;                   // 模块加载时间（系统启动以来的时间）
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;
// 获取某个模块的基地址(64位进程)
ULONG64 RwGetModuleHandle64(HANDLE pid, char* dllName);