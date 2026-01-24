#include "util.h"
#include "RwGetModule.h"
#include "GetModuleUtil.h"

void DriverUnload(PDRIVER_OBJECT pDriver) {
    // UnRegisterCallBack();
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath) {
    DbgBreakPoint();
    pDriver->DriverUnload = DriverUnload;
    // ULONG64 addr = GetModuleBase("ntoskrnl.exe");
    // RegisterCallBack();
    // ULONG64 addr = RwGetModuleHandle(2712, "kernel132.dll");
    // DbgPrint("%d", addr);
    RwGetAddrByCode("4C8B324C89742430BA08000000448D42FC488BCE", 40);
    return STATUS_SUCCESS;
}