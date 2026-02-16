/********************************************************************************************************************
 * @brief   Util.c
 * @details 主要实现拦截特定函数，并注册自定义回调函数；
********************************************************************************************************************/

#include "Util.h"
#include "MemRW.h"
#include "Struct.h"

// 保存原始回调函数指针
MyAttributeInofrmationCallback OldQueryCallback = NULL;
MyAttributeInofrmationCallback OldSetCallback = NULL;

// 保存原始回调地址
PULONG64 uCallBack = NULL;

/********************************************************************************************************************
 * @brief   自定义查询回调函数
 * @details 进行过滤，只拦截并处理指定调用者，其他调用转发给原始回调
 * @param   handle - 查询请求的句柄
 * @param   addr   - 查询请求的参数地址
 * @return  NTSTATUS类型的状态码
 * @warning 此函数运行在任意线程上下文，必须保证可重入性和线程安全性
********************************************************************************************************************/
NTSTATUS DrawQueryCallback(HANDLE handle, PVOID addr) {
    DbgPrint("DrawQueryCallback函数已被调用!!! addr=0x%p", addr);

    // 使用结构化异常处理(SEH)保护代码，防止访问无效内存
    __try {
        // 使用 ProbeForRead 更安全地检查内存 - 验证用户模式地址的有效性
        ProbeForRead(addr, sizeof(MESSAGE_PACKAGE), sizeof(ULONG));
        // 将地址转换为消息包结构体指针
        PMESSAGE_PACKAGE message = (PMESSAGE_PACKAGE)addr;
        DbgPrint("message->flag = %llu", message->flag);
        // 检查是否为自定义请求(通过flag=1234标识)
        if (message->flag == 1234) {
            DbgPrint("已收到三环请求!");
            message->result = DispatchCallbackEntry(message);
        } else {
            DbgPrint("非自定义请求，flag: %llu", message->flag);
            // 非自定义请求，转发给原始回调函数处理
            if (OldQueryCallback) {
                return OldQueryCallback(handle, addr);
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        // 发生内存访问异常时的处理
        DbgPrint("读取内存时发生异常!");
        // 返回异常代码，让上层知道发生了什么错误
        return GetExceptionCode();
    }
}

/********************************************************************************************************************
 * @brief   自定义设置回调函数
 * @details 进行过滤，只拦截并处理指定调用者，其他调用转发给原始回调
 * @param   handle - 设置请求的句柄
 * @param   addr   - 设置请求的参数地址
 * @return  NTSTATUS类型的状态码
 * @warning 此函数运行在任意线程上下文，必须保证可重入性和线程安全性
********************************************************************************************************************/
NTSTATUS DrawSetCallback(HANDLE handle, PVOID addr) {
    DbgPrint("[驱动] DrawSetCallback函数已被调用!!! addr=0x%p", addr);
    __try {
        // 同样使用ProbeForRead验证内存
        ProbeForRead(addr, sizeof(MESSAGE_PACKAGE), sizeof(ULONG));
        PMESSAGE_PACKAGE message = (PMESSAGE_PACKAGE)addr;
        // 检查是否为自定义请求
        if (message->flag == 1234) {
            DbgPrint("[驱动] 已收到三环请求(Set)!");
            message->result = DispatchCallbackEntry(message);
        } else {
            // 非自定义请求，转发给原始回调
            if (OldSetCallback) {
                return OldSetCallback(handle, addr);
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        DbgPrint("[驱动] 读取内存时发生异常(Set)!");
        return GetExceptionCode();
    }
}

// 根据需要完成的操作，分发不同类型的回调函数
NTSTATUS DispatchCallbackEntry(PMESSAGE_PACKAGE message) {
    NTSTATUS status = STATUS_SUCCESS;
    switch (message->func) {
        case 1: {
            // 如下四种方式均已通过测试，可以正确读取指定进程内存
            PRWMM rwmm = (PRWMM)message->data;
            // status = ReadR3Memory(rwmm->pid, rwmm->start, rwmm->size, rwmm->dest);
            // status = ReadR3MemoryByCr3(rwmm->pid, rwmm->start, rwmm->size, rwmm->dest);
            status = ReadR3MemoryByVirtualMemory(rwmm->pid, rwmm->start, rwmm->size, rwmm->dest);
            // status = ReadR3MemoryByMdl(rwmm->pid, rwmm->start, rwmm->size, rwmm->dest);
            break;
        }
        case 2: {
            PRWMM rwmm = (PRWMM)message->data;
            status = RwQueryVirtualMemory(rwmm->pid, rwmm->start, rwmm->dest);
            break;
        }

        default:
            break;
    }
    return status;
}

// 注册自定义属性信息回调函数
NTSTATUS RegisterCallBack() {
    // 获取 ExRegisterAttributeInformationCallback 函数地址
    // 定义变量存储函数名称
    UNICODE_STRING funcName = { 0 };
    // 初始化函数名称字符串，用于查找内核导出函数
    RtlInitUnicodeString(&funcName, L"ExRegisterAttributeInformationCallback");
    // 获取函数地址，避免硬编码函数地址
    PVOID funcAddr = MmGetSystemRoutineAddress(&funcName);
    if (NULL == funcAddr) {
        DbgPrint("MmGetSystemRoutineAddress Error!!!");
        return STATUS_PROCEDURE_NOT_FOUND;
    }

    // 解析函数内部偏移量
    ULONG64 offset = *(PULONG)((ULONG64)funcAddr + 0x10);
    // 计算属性信息结构地址
    PULONG64 attrInfoAddr = ((ULONG64)funcAddr + 0xd + 7 + offset);
    // 保存原始回调地址
    uCallBack = attrInfoAddr;

    // query 与 set 相邻，可以将两者地址作为一个数组(0 -> query; 0 -> set)
    // 保存原始回调函数指针
    OldQueryCallback = (MyAttributeInofrmationCallback)attrInfoAddr[0];
    OldSetCallback = (MyAttributeInofrmationCallback)attrInfoAddr[1];

    DbgPrint("原始查询回调地址: 0x%p", OldQueryCallback);
    DbgPrint("原始设置回调地址: 0x%p", OldSetCallback);

    // 如果想要成功注册回调函数，则两个地址必须为空，而不为空，就不会进行注册
    attrInfoAddr[0] = 0;
    attrInfoAddr[1] = 0;

    // 注册自定义回调函数
    _ExRegisterAttributeInfomationCallback regAttrInfoCallback = funcAddr;
    RWCALL_BACK_FUNC rwCallBackFunc = { 0 };
    rwCallBackFunc.ExDisQueryAttributeInformation = DrawQueryCallback;
    rwCallBackFunc.ExDisSetAttributeInformation = DrawSetCallback;
    NTSTATUS status = regAttrInfoCallback(&rwCallBackFunc);
    if (NT_SUCCESS(status)) {
        DbgPrint("注册成功!");
    } else {
        DbgPrint("注册失败!");
        // 注册失败时，应当恢复原始回调
        attrInfoAddr[0] = (ULONG64)OldQueryCallback;
        attrInfoAddr[1] = (ULONG64)OldSetCallback;
    }
    return status;
}

// 卸载回调函数
VOID UnRegisterCallback() {
    if (uCallBack) {
        uCallBack[0] = OldSetCallback;      // 恢复设置回调
        uCallBack[1] = OldQueryCallback;    // 恢复查询回调
    }
}

// 内存属性查询：查询指定进程中某个虚拟地址的内存基本信息
NTSTATUS RwQueryVirtualMemory(HANDLE pId, ULONG64 baseAddr, PMYMEMORY_BASIC_INFORMATION baseInformation) {
    // 参数检查：输出缓冲区不能为空
    NTSTATUS state = STATUS_SUCCESS;
    if (NULL == baseInformation) return STATUS_INVALID_PARAMETER;

    PEPROCESS process = NULL;
    // 根据进程ID获取EPROCESS内核对象
    state = PsLookupProcessByProcessId(pId, &process);
    if (!NT_SUCCESS(state)) return state;

    KAPC_STATE apcState = { 0 };
    // 附加到目标进程的地址空间，以便直接使用当前进程句柄调用 ZwQueryVirtualMemory
    KeStackAttachProcess(process, &apcState);

    // 分配内核非分页内存用于接收标准 MEMORY_BASIC_INFORMATION 结构
    PMEMORY_BASIC_INFORMATION information = ExAllocatePool(NonPagedPool, sizeof(MEMORY_BASIC_INFORMATION));
    if (NULL == information) {
        // 内存分配失败，先脱离进程再返回
        KeUnstackDetachProcess(&apcState);
        return STATUS_UNSUCCESSFUL;
    }

    // 清空分配的内存
    memset(information, 0, sizeof(MEMORY_BASIC_INFORMATION));

    SIZE_T realSize = 0;
    // 调用 ZwQueryVirtualMemory 查询当前进程（已挂靠为目标进程）的虚拟内存信息
    // NtCurrentProcess() 返回当前进程的伪句柄，在挂靠状态下实际查询的是目标进程
    state = ZwQueryVirtualMemory(NtCurrentProcess(), baseAddr, MemoryBasicInformation,
                                 information, sizeof(MEMORY_BASIC_INFORMATION), &realSize);

    // 脱离目标进程地址空间（无论查询是否成功都需要脱离）
    KeUnstackDetachProcess(&apcState);

    if (NT_SUCCESS(state)) {
        // 将标准结构的数据复制到自定义结构 baseInformation 中
        // 注意字段映射：标准结构中为 RegionSize，自定义中为 ReginSize（拼写稍异）
        baseInformation->AllocationBase = information->AllocationBase;
        baseInformation->AllocationProtect = information->AllocationProtect;
        baseInformation->BaseAddress = information->BaseAddress;
        baseInformation->Protect = information->Protect;
        baseInformation->RegionSize = information->RegionSize;
        baseInformation->State = information->State;
        baseInformation->Type = information->Type;
    }

    // 释放 EPROCESS 引用和分配的内存
    ObDereferenceObject(process);
    ExFreePool(information);
    return state;
}

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