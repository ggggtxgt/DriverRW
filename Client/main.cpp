#include <iostream>
#include <Windows.h>

// 异步IO状态块
typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;     // IO 操作状态码
        PVOID    Pointer;    // 用于返回的指针
    };
    ULONG_PTR Information;   // 操作返回的附加信息
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

// 驱动-应用层通信的数据包结构
typedef struct _MESSAGE_PACKAGE {
    ULONG64 flag;   // 通信标志（用于区分是否为自定义通信）
    ULONG64 data;   // 通信传递的数据（通常为内存地址）
    ULONG64 size;   // 数据的长度
    ULONG result;   // 执行结果
} MESSAGE_PACKAGE, *PMESSAGE_PACKAGE;

// 定义 _NtQueryInformationFile 函数指针类型，位于 ntdll.dll 之中
typedef LONG(*_NtQueryInformationFile)(HANDLE FileHandle, PIO_STATUS_BLOCK IlStatusBlock, PVOID FileInformation, ULONG Length, LONG FileInformationClass);

int main() {
    // 获取 _NtQueryInformationFile 函数地址
    _NtQueryInformationFile myQueryInformationFile = NULL;
    myQueryInformationFile = (_NtQueryInformationFile) GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationFile");

    // 创建文件对象，作为通信的触发器
    HANDLE hFile = CreateFileA("C:\\test.txt", FILE_ALL_ACCESS, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    // 准备通信缓冲区
    IO_STATUS_BLOCK ioStatusBlock = {0};    // 用于接收IO状态
    char fileBuffer[0XE0] = {0};            // 224字节的缓冲区
    PMESSAGE_PACKAGE message = (PMESSAGE_PACKAGE) fileBuffer;
    message->flag = 1234;

    // 触发驱动层回调函数
    myQueryInformationFile(hFile, &ioStatusBlock, fileBuffer, 0xE0, 0x34);

    system("pause");
    return 0;
}
