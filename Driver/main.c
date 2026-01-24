#include "util.h"
#include "RwGetModule.h"
#include "GetModuleUtil.h"

void DriverUnload(PDRIVER_OBJECT pDriver) {
    // UnRegisterCallBack();
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath) {
    DbgBreakPoint();
    pDriver->DriverUnload = DriverUnload;
    RTL_OSVERSIONINFOW version = { 0 };
    version.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
    RtlGetVersion(&version);    // 获取当前 Windows 版本，使用 version.dwBuildNumber 判断

    return STATUS_SUCCESS;
}