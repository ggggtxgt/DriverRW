#pragma once
#include <ntifs.h>
// 自定义回调函数
typedef NTSTATUS(*MyAttributeInofrmationCallback)(HANDLE handle, PVOID arg);
// 注册回调函数
NTSTATUS RegisterCallBack();