#include "util.h"
#include "structure.h"
#include "RwProtect.h"
#include "RwGetModule.h"
#include "GetModuleUtil.h"

HANDLE hCallback = NULL;

void DriverUnload(PDRIVER_OBJECT pDriver) {
    /*
    // UnRegisterCallBack();
    if (hCallback) {
        ObUnRegisterCallbacks(hCallback);
    }
    */

}
/*
OB_PREOP_CALLBACK_STATUS PobPreOperationCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation) {
    OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = PROCESS_ALL_ACCESS;
    OperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess = PROCESS_ALL_ACCESS;
    OperationInformation->Parameters->DuplicateHandleInformation.DesiredAccess = PROCESS_ALL_ACCESS;
    OperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess = PROCESS_ALL_ACCESS;
    return OB_PREOP_SUCCESS;
}
*/
typedef NTSTATUS (*_PsSuspendThread)(IN PETHREAD Thread, OUT PULONG PreviousSuspendCount);
typedef NTSTATUS (*_PsResumeThread)(IN PETHREAD Thread, OUT PULONG PreviousSuspendCount OPTIONAL);

void PobPostOperationCallback(PVOID RegistrationContext, POB_POST_OPERATION_INFORMATION OperationInformation) {

}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath) {
    /*
    pDriver->DriverUnload = DriverUnload;
    PKLDR_DATA_TABLE_ENTRY pLdr = pDriver->DriverSection;
    pLdr->Flags |= 0x20;  // 没有微软签名的作法
    OB_OPERATION_REGISTRATION obOperRegister = { 0 };
    obOperRegister.ObjectType = PsProcessType;
    obOperRegister.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
    obOperRegister.PreOperation = PobPreOperationCallback;
    obOperRegister.PostOperation = PobPostOperationCallback;
    OB_CALLBACK_REGISTRATION obCallback = { 0 };
    obCallback.Version = ObGetFilterVersion();
    obCallback.OperationRegistrationCount = 1;
    obCallback.RegistrationContext = NULL;
    RtlInitUnicodeString(&obCallback.Altitude, L"obCallback.Altitude");
    obCallback.OperationRegistration = &obOperRegister;
    NTSTATUS status = ObRegisterCallbacks(&obCallback, &hCallback);
    */
    /*
    RTL_OSVERSIONINFOW version = { 0 };
    version.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
    RtlGetVersion(&version);    // 获取当前 Windows 版本，使用 version.dwBuildNumber 判断
    */
    // RegisterCallBack();
    /*
    // 修改进程对象头实现进程保护
    PEPROCESS process = NULL;
    NTSTATUS status = PsLookupProcessByProcessId(1872, &process);
    if (NT_SUCCESS(status)) {
        *((PUCHAR)process - 0X30 + 0X1B) = 4;
    }
    */
    // 33FF897C2424654C8B2425880100004C89A424800000006641FF8C24C40100004881C1300400000F0D09
    pDriver->DriverUnload = DriverUnload;

    ULONG64 uaddr = RwGetAddrByCode("33FF897C2424654C8B2425880100004C89A424800000006641FF8C24C40100004881C1300400000F0D09", 84);
    uaddr -= 42;
    if (0 == uaddr) return STATUS_SUCCESS;
    static _PsSuspendThread myThreadSuspend = NULL;
    if (NULL == myThreadSuspend) {
        myThreadSuspend = (_PsSuspendThread)uaddr;
    }
    PETHREAD peThread = NULL;
    NTSTATUS status = PsLookupThreadByThreadId(111, &peThread);
    if (!NT_SUCCESS(status)) return STATUS_SUCCESS;
    myThreadSuspend(peThread, 0);
    return STATUS_SUCCESS;
}