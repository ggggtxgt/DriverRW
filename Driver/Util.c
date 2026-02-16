/********************************************************************************************************************
 * @brief   Util.c
 * @details 主要实现拦截特定函数，并注册自定义回调函数；
********************************************************************************************************************/

#include "Util.h"
#include "MemRW.h"

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
