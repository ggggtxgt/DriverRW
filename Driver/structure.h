#pragma once

#include <ntifs.h>

typedef struct _MY_MEMORY_BASIC_INFORMATION {
	ULONG64 BaseAddress;
	ULONG64 AllocationBase;
	ULONG64 AllocationProtect;
	ULONG64 ReginSize;
	ULONG64 State;
	ULONG64 Protect;
	ULONG64 Type;
} MYMEMORY_BASIC_INFORMATION, *PMYMEMORY_BASIC_INFORMATION;