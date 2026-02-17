#include "RwProtected.h"

// 全局变量，保存对象回调的注册句柄，用于卸载回调
HANDLE hCallback = NULL;

// 预操作回调函数：在创建或复制进程句柄之前被调用
OB_PREOP_CALLBACK_STATUS PobPreOperationCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation) {
    // 修改句柄的期望访问权限为完全访问权限，强制所有进程句柄都具有PROCESS_ALL_ACCESS
    OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = PROCESS_ALL_ACCESS;
    // 保存原始期望访问权限（此处也设置为完全访问，通常用于记录原始值）
    OperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess = PROCESS_ALL_ACCESS;
    // 对于重复句柄操作，同样修改期望访问权限
    OperationInformation->Parameters->DuplicateHandleInformation.DesiredAccess = PROCESS_ALL_ACCESS;
    return OB_PREOP_SUCCESS;
}

// 定义未文档化的函数指针类型，用于挂起和恢复线程（此处未使用）
typedef NTSTATUS(*_PsSuspendThread)(IN PETHREAD Thread, OUT PULONG PreviousSuspendCount);
typedef NTSTATUS(*_PsResumeThread)(IN PETHREAD Thread, OUT PULONG PreviousSuspendCount OPTIONAL);

// 后操作回调函数：在创建或复制句柄之后被调用（此处为空，不执行任何操作）
void PobPostOperationCallback(PVOID RegistrationContext, POB_POST_OPERATION_INFORMATION OperationInformation) {

}

// 注册回调以实现进程保护
NTSTATUS ProcessProtected(PDRIVER_OBJECT pDriver) {
    // 获取驱动模块在内核模块表中的条目，用于修改标志
    PKLDR_DATA_TABLE_ENTRY pLdr = pDriver->DriverSection;
    // 设置模块标志的第5位（0x20），可能标记驱动为受保护或系统关键驱动（未文档化）
    pLdr->Flags |= 0x20;

    // 配置对象操作注册结构，指定监控进程对象类型及回调函数
    OB_OPERATION_REGISTRATION obOperRegister = { 0 };
    obOperRegister.ObjectType = PsProcessType;            // 监控进程对象
    obOperRegister.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE; // 监控句柄创建和复制
    obOperRegister.PreOperation = PobPreOperationCallback;   // 指定预操作回调
    obOperRegister.PostOperation = PobPostOperationCallback; // 指定后操作回调（空）

    // 配置对象回调注册结构
    OB_CALLBACK_REGISTRATION obCallback = { 0 };
    obCallback.Version = ObGetFilterVersion();               // 获取当前系统支持的回调版本
    obCallback.OperationRegistrationCount = 1;               // 只有一个操作注册条目
    obCallback.RegistrationContext = NULL;                   // 上下文参数，传递给回调
    RtlInitUnicodeString(&obCallback.Altitude, L"obCallback.Altitude"); // 设置海拔（优先级），此处硬编码为字符串
    obCallback.OperationRegistration = &obOperRegister;      // 指向操作注册数组

    // 注册对象回调，成功时返回句柄存入 hCallback
    NTSTATUS status = ObRegisterCallbacks(&obCallback, &hCallback);
    return status;
}

// 注销回调函数，在驱动卸载时调用
void ObUnRegister() {
    if (hCallback) ObUnRegisterCallbacks(hCallback); // 如果回调句柄有效，则注销回调
}

// 修改进程对象头实现进程保护
NTSTATUS EditHeaderProtected(HANDLE pid) {
    PEPROCESS process = NULL;
    // 根据进程ID查找目标进程的 EPROCESS 结构
    NTSTATUS status = PsLookupProcessByProcessId(1872, &process);  // 注意：硬编码PID，实际应从参数获取
    if (NT_SUCCESS(status)) {
        // 在 Windows 内核中，每个对象（如进程）在内存中都有一个对象头 _OBJECT_HEADER 位于对象体之前
        // 对于64位系统，对象头大小为 0x30 字节。
        // 因此，将 EPROCESS 指针减去 0x30 得到指向对象头的基址。
        // 在对象头偏移 0x1B 处是 Flags 字段，该字段包含多个标志位，用于描述对象属性
        // Flags 的位定义（参考 _OBJECT_HEADER 结构）：
        //   Bit 0: NewObject
        //   Bit 1: KernelObject
        //   Bit 2: KernelOnlyAccess
        //   Bit 3: ExclusiveObject
        //   Bit 4: PermanentObject
        //   Bit 5: DefaultSecurityQuota
        //   Bit 6: SingleHandleEntry
        //   Bit 7: DeletedInline
        // 设置值为 4 (二进制 00000100) 即置位 KernelOnlyAccess (仅内核可访问)
        // 设置此标志后，用户模式代码将无法打开该进程的句柄，从而防止用户态恶意访问
        * ((PUCHAR)process - 0x30 + 0x1B) = 4;
        return STATUS_SUCCESS;
    }
    return STATUS_UNSUCCESSFUL;
}

// 安装回调函数 -- 隐藏回调保护
// @todo 待完善
NTSTATUS InstallCallback() {
    OB_OPERATION_REGISTRATION obOperRegister = { 0 };
    obOperRegister.ObjectType = PsProcessType;
    obOperRegister.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
    obOperRegister.PreOperation = PobPreOperationCallback;
    obOperRegister.PostOperation = PobPostOperationCallback;
    OB_CALLBACK_REGISTRATION obCallback = { 0 };
    obCallback.Version = ObGetFilterVersion();
    obCallback.OperationRegistrationCount = 1;
    obCallback.RegistrationContext = PobPreOperationCallback;
    RtlInitUnicodeString(&obCallback.Altitude, L"obCallback.Altitude");
    obCallback.OperationRegistration = &obOperRegister;
    NTSTATUS status = ObRegisterCallbacks(&obCallback, &hCallback);
    return status;
}

// 卸载回调函数 -- 隐藏回调保护
void UninstallCallback() {

}