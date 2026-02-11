#include "Util.h"

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

    return STATUS_SUCCESS;
}