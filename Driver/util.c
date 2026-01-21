#include "util.h"

NTSTATUS RegisterCallBack() {
    // 获取 ExRegisterAttributeInformationCallback 函数地址
    UNICODE_STRING funcName = {0};
    RtlInitUnicodeString(&funcName, L"ExRegisterAttributeInformationCallback");
    PVOID funcAddr = MmGetSystemRoutineAddress(&funcName);
    ULONG64 offset = *(PULONG)((ULONG64) funcAddr + 0x10);
    PULONG64 attr_info_addr = ((ULONG64) funcAddr + 0xd + 7 + offset);
}