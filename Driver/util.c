#include "util.h"
#include "MemoryRW.h"

// 保存原始回调函数指针
MyAttributeInofrmationCallback OldQueryCallback = NULL;
MyAttributeInofrmationCallback OldSetCallback = NULL;

// 保存原始回调地址
PULONG64 uCallBack = NULL;

// 自定义查询回调函数
NTSTATUS DrawQueryCallback(HANDLE handle, PVOID addr) {
    PMESSAGE_PACKAGE message = (PMESSAGE_PACKAGE) addr;
    if (MmIsAddressValid(addr)) {
        // 标志正确，调用自定义回调函数
        if (1234 == message->falg) {
            DbgPrint("已收到三环请求: ");
            message->result = DispatchCallEntry(message);
        } else {
            // 判断是否为空，防止出现系统崩溃
            if (OldQueryCallback) {
                // 标志错误，调用原始回调函数处理
                return OldQueryCallback(handle, addr);
            }
        }
    }
    return STATUS_SUCCESS;
}

// 自定义设置回调函数
NTSTATUS DrawSetCallback(HANDLE handle, PVOID addr) {
    PMESSAGE_PACKAGE message = (PMESSAGE_PACKAGE) addr;
    if (MmIsAddressValid(addr)) {
        // 标志正确，调用自定义回调函数
        if (1234 == message->falg) {
            DbgPrint("已收到三环请求: ");
            message->result = DispatchCallEntry(message);
        } else {
            // 判断是否为空，防止出现系统崩溃
            if (OldSetCallback) {
                // 标志错误，调用原始回调函数处理
                return OldSetCallback(handle, addr);
            }
        }
    }
    return STATUS_SUCCESS;
}

NTSTATUS DispatchCallEntry(PMESSAGE_PACKAGE package) {
    NTSTATUS status = STATUS_SUCCESS;
    switch (package->func) {
        case 1: {
            PRWMM rwmm = (PRWMM) package->data;
            status = (rwmm->pId, rwmm->startAddr, rwmm->size, rwmm->destAddr);
        }

        default:
            break;
    }
    return status;
}

NTSTATUS RegisterCallBack() {
    // 获取 ExRegisterAttributeInformationCallback 函数地址
    UNICODE_STRING funcName = {0};
    RtlInitUnicodeString(&funcName, L"ExRegisterAttributeInformationCallback");
    PVOID funcAddr = MmGetSystemRoutineAddress(&funcName);
    ULONG64 offset = *(PULONG)((ULONG64) funcAddr + 0x10);
    PULONG64 attrInfoAddr = ((ULONG64) funcAddr + 0xd + 7 + offset);
    uCallBack = attrInfoAddr;

    // 保存原函数信息
    OldQueryCallback = attrInfoAddr[0];
    OldSetCallback = attrInfoAddr[1];

    // 清空原函数信息
    attrInfoAddr[0] = 0;
    attrInfoAddr[1] = 0;

    // 注册自定义回调函数
    _ExRegisterAttributeInfomationCallback regAttrInfoCallback = funcAddr;
    RWCALL_BACK_FUNC rwCallBackFunc = {0};
    rwCallBackFunc.ExDisQueryAttributeInformation = DrawQueryCallback;
    rwCallBackFunc.ExDisSetAttributeInformation = DrawSetCallback;
    NTSTATUS result = regAttrInfoCallback(&rwCallBackFunc);
    if (NT_SUCCESS(result)) {
        DbgPrint("注册成功!");
    } else {
        DbgPrint("注册失败!");
    }
    return result;
}

void UnRegisterCallBack() {
    if (uCallBack) {
        uCallBack[0] = OldSetCallback;      // 恢复设置回调
        uCallBack[1] = OldQueryCallback;    // 万利查询回调
    }
}