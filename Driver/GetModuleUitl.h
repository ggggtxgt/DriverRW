#pragma once

#include <ntifs.h>

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation,                // 00 Y 基本系统信息
    SystemProcessorInformation,            // 01 Y 处理器信息
    SystemPerformanceInformation,          // 02 Y 系统性能信息
    SystemTimeOfDayInformation,            // 03 Y 系统时间信息
    SystemNotImplemented1,                 // 04 N 未实现
    SystemProcessesAndThreadsInformation,  // 05 Y 进程和线程信息
    SystemCallCounts,                      // 06 Y 系统调用计数
    SystemConfigurationInformation,        // 07 Y 系统配置信息
    SystemProcessorTimes,                  // 08 Y 处理器时间
    SystemGlobalFlag,                      // 09 Y 系统全局标志
    SystemNotImplemented2,                 // 10 N 未实现
    SystemModuleInformation,               // 11 Y 内核模块信息
    SystemLockInformation,                 // 12 Y 锁信息
    SystemNotImplemented3,                 // 13 N 未实现
    SystemNotImplemented4,                 // 14 N 未实现
    SystemNotImplemented5,                 // 15 N 未实现
    SystemHandleInformation,               // 16 Y 句柄信息
    SystemObjectInformation,               // 17 Y 对象信息
    SystemPagefileInformation,             // 18 Y 页面文件信息
    SystemInstructionEmulationCounts,      // 19 Y 指令模拟计数
    SystemInvalidInfoClass1,               // 20 N 无效信息类
    SystemCacheInformation,                // 21 Y 缓存信息
    SystemPoolTagInformation,              // 22 Y 池标记信息
    SystemProcessorStatistics,             // 23 Y 处理器统计
    SystemDpcInformation,                  // 24 Y DPC信息
    SystemNotImplemented6,                 // 25 N 未实现
    SystemLoadImage,                       // 26 Y 加载映像
    SystemUnloadImage,                     // 27 Y 卸载映像
    SystemTimeAdjustment,                  // 28 Y 时间调整
    SystemNotImplemented7,                 // 29 N 未实现
    SystemNotImplemented8,                 // 30 N 未实现
    SystemNotImplemented9,                 // 31 N 未实现
    SystemCrashDumpInformation,            // 32 Y 崩溃转储信息
    SystemExceptionInformation,            // 33 Y 异常信息
    SystemCrashDumpStateInformation,       // 34 Y 崩溃转储状态信息
    SystemKernelDebuggerInformation,       // 35 Y 内核调试器信息
    SystemContextSwitchInformation,        // 36 Y 上下文切换信息
    SystemRegistryQuotaInformation,        // 37 Y 注册表配额信息
    SystemLoadAndCallImage,                // 38 Y 加载并调用映像
    SystemPrioritySeparation,              // 39 Y 优先级分离
    SystemNotImplemented10,                // 40 N 未实现
    SystemNotImplemented11,                // 41 N 未实现
    SystemInvalidInfoClass2,               // 42 N 无效信息类
    SystemInvalidInfoClass3,               // 43 N 无效信息类
    SystemTimeZoneInformation,             // 44 Y 时区信息
    SystemLookasideInformation,            // 45 Y 后备列表信息
    SystemSetTimeSlipEvent,                // 46 Y 设置时间滑移事件
    SystemCreateSession,                   // 47 Y 创建会话
    SystemDeleteSession,                   // 48 Y 删除会话
    SystemInvalidInfoClass4,               // 49 N 无效信息类
    SystemRangeStartInformation,           // 50 Y 范围起始信息
    SystemVerifierInformation,             // 51 Y 驱动程序验证器信息
    SystemAddVerifier,                     // 52 Y 添加验证器
    SystemSessionProcessesInformation      // 53 Y 会话进程信息
} SYSTEM_INFORMATION_CLASS, * PSYSTEM_INFORMATION_CLASS;

// 用于描述系统内核中加载的一个模块（驱动程序）的详细信息。
typedef struct _RTL_PROCESS_MODULE_INFORMATION {
    HANDLE Section;         // 模块对应的区段对象句柄（内核句柄）
    PVOID MappedBase;       // 模块在系统空间中的映射基址（加载地址）
    PVOID ImageBase;        // 模块的预设加载基址（可能因重定位而不同）
    ULONG ImageSize;        // 模块映像的大小（字节）
    ULONG Flags;            // 模块标志（如是否已加载等）
    USHORT LoadOrderIndex;  // 模块在加载顺序中的索引
    USHORT InitOrderIndex;  // 模块在初始化顺序中的索引
    USHORT LoadCount;       // 模块的加载计数
    USHORT OffsetToFileName;// 模块文件名在 FullPathName 中的偏移（以字节为单位）
    UCHAR FullPathName[256];// 模块的完整路径名（以 null 结尾的 ANSI 字符串）
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

// 内核模块列表结构
typedef struct _RTL_PROCESS_MODULES {
    ULONG64 NumberOfModules;                     // 当前系统加载的模块总数
    RTL_PROCESS_MODULE_INFORMATION Modules[1];   // 模块信息数组，实际大小由 NumberOfModules 决定
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

// 查询指定的系统信息（内核模式系统服务）
// 此函数是未文档化的系统服务，但在驱动开发中常用于查询系统模块列表、句柄信息等；
// 调用时必须保证缓冲区位于内核模式地址空间（或已探测的用户模式地址）；
// 信息类 SystemModuleInformation 可用于获取内核加载的所有驱动程序模块列表；
NTSTATUS ZwQuerySystemInformation(
    SYSTEM_INFORMATION_CLASS SystemInformationClass,   // 需要查询的系统信息类型
    PVOID SystemInformation,                           // 指向接收数据的缓冲区
    ULONG SystemInformationLength,                     // 缓冲区大小（字节）
    PULONG ReturnLength                                // 可选，返回实际需要的字节数
);
// 获取指定内核模块基地址
ULONG64 GetModuleBase(PCHAR moduleName);
