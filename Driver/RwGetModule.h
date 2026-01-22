#pragma once

#include <ntifs.h>

typedef struct _PEB32 {
    UCHAR InheritedAddressSpace;
    UCHAR ReadImageFileExecOptions;
    UCHAR BeingDebugged;
    union {
        UCHAR BitField;
        struct {
            UCHAR IsImageUserLargePages: 1;
            UCHAR IsProtectedProcess: 1;
            UCHAR IsLegacyProcess: 1;
            UCHAR IsImageDynamicallyRelocated: 1;
            UCHAR SkipPatchingUser32Forwarders: 1;
            UCHAR SpareBits: 3;
        };
    };
    ULONG Mutant;
    ULONG ImageBaseAddress;
    ULONG Ldr;
} PEB32, *PPEB32;

typedef struct _PEB_LDR_DATA32 {
    ULONG Length;
    UCHAR Initialized;
    ULONG SsHandle;
    LIST_ENTRY32 InLoadOrderModuleList;
    LIST_ENTRY32 InMemoryOrderModuleList;
    LIST_ENTRY32 InInitializedOrderModuleList;
    ULONG EntryInProcess;
    UCHAR ShutdownInProgress;
    ULONG ShutdownThreadId;
} PEB_LDR_DATA32, *PPEB_LDR_DATA32;

typedef struct _LDR_DATA_TABLE_ENTRY32 {
    LIST_ENTRY32 InLoadOrderLinks;
    LIST_ENTRY32 InMemoryOderLinks;
    LIST_ENTRY32 InInitializedOrderLinks;
    ULONG DllBase;
    ULONG EntryPoint;
    ULONG SizeOrImage;
    UNICODE_STRING32 FullDllName;
    UNICODE_STRING32 BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
    union {
        LIST_ENTRY32 HashLinks;
        struct {
            ULONG SectionPointer;
            ULONG CheckSum;
        };
    };
    union {
        ULONG TimeDateStamp;
        ULONG LoadedImports;
    };
    ULONG EntryPointActivationContext;
    LIST_ENTRY32 ForwarderLinks;
    LIST_ENTRY32 ServiceTagLinks;
    LIST_ENTRY32 StaticLinks;
    ULONG ContextInformation;
    ULONG OriginalBase;
    LARGE_INTEGER LoadTime;
} LDR_DATA_TABLE_ENTRY32, *PLDR_DATA_TABLE_ENTRY32;

// 获取某个模块的基地址
ULONG64 RwGetModuleHandle(HANDLE pId, char *dllName);
