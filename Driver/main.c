#include "util.h"
#include "RwGetModule.h"


void DriverUnload(PDRIVER_OBJECT pDriver) {
    // UnRegisterCallBack();
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath) {
    pDriver->DriverUnload = DriverUnload;
    // RegisterCallBack();
    ULONG64 addr = RwGetModuleHandle(2712, "kernel132.dll");
    DbgPrint("%d", addr);
    return STATUS_SUCCESS;
}