#pragma once
#include <ntifs.h>

// 挂起指定线程的函数指针类型
// param: 指向要挂起的线程对象的指针（PETHREAD）
// param: 指向ULONG变量的指针，用于接收该线程之前的挂起计数
typedef NTSTATUS(*_PsSuspendThread)(IN PETHREAD Thread, OUT PULONG PreviousSuspendCount);
// 恢复指定线程的函数指针类型
// param: 指向要恢复的线程对象的指针（PETHREAD）
// param: 可选参数，指向ULONG变量的指针，用于接收该线程恢复前的挂起计数
typedef NTSTATUS(*_PsResumeThread)(IN PETHREAD Thread, OUT PULONG PreviousSuspendCount OPTIONAL);