#include "util.h"

void DriverUnload(PDRIVER_OBJECT pDriver) {
    UnRegisterCallBack();
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath) {
    pDriver->DriverUnload = DriverUnload;
    RegisterCallBack();
    return STATUS_SUCCESS;
}