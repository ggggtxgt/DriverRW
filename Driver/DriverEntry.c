#include "Util.h"
#include "RwGetModule.h"
#include "GetModuleUitl.h"

/********************************************************************************************************************
 * @brief   驱动卸载回调函数：当驱动被卸载时，系统自动启用该函数；
 * @param   驱动对象的结构体指针；
********************************************************************************************************************/
void DriverUnload(PDRIVER_OBJECT pDriver) {
	DbgPrint("DriverUnload!!!");
	// UnRegisterCallback();
    ObUnRegister();
}

#include <ntddk.h>  // 包含 RtlGetVersion 等内核函数

// 声明不同版本对应的回调函数（需根据实际需求定义）
extern VOID Win7Callback(VOID);
extern VOID Win10Callback(VOID);


// 功能：获取当前操作系统版本，并根据版本调用对应的回调函数
VOID GetVersion(VOID) {
    RTL_OSVERSIONINFOW version = { 0 };
    version.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
    NTSTATUS status = RtlGetVersion(&version);

    if (!NT_SUCCESS(status)) {
        DbgPrint("RtlGetVersion failed with status 0x%X\n", status);
        return;
    }

    // 根据主次版本号和构建号判断
    if (version.dwMajorVersion == 6 && version.dwMinorVersion == 1) {
        // Windows 7 (6.1)
        DbgPrint("Detected Windows 7 (build %lu)\n", version.dwBuildNumber);
        RegisterCallBack();
    } else if (version.dwMajorVersion == 10 && version.dwBuildNumber >= 10240) {
        // Windows 10 (build 10240 及以上，包括 Windows 11)
        DbgPrint("Detected Windows 10/11 (build %lu)\n", version.dwBuildNumber);
        RwGetAddrByCode("4C8B324C89742430BA08000000448D42FC488BCE", 40);
    } else {
        // 其他版本（如 Windows 8/8.1、Server 等）
        DbgPrint("Unsupported OS version: %lu.%lu (build %lu)\n",
            version.dwMajorVersion, version.dwMinorVersion, version.dwBuildNumber);
        // 可选择默认处理或忽略
    }
}

/********************************************************************************************************************
 * @brief   驱动入口函数：相当于驱动程序的 main 函数；
 * @param   指向驱动对象的结构体指针；
 * @param   指向驱动在注册表中路径的UNICODE字符串指针；
 * @return  NTSTATUS类型的状态码，表示驱动是否加载成功；
********************************************************************************************************************/
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath) {
	DbgPrint("DriverEntry!!!");
	pDriver->DriverUnload = DriverUnload;
	// 读取指定进程内存
	RegisterCallBack();
	
	// 获取指定进程模块
	// ULONG64 addr = RwGetModuleHandle64(1192, "kernel32.dll");
	// DbgPrint("addr: %d", addr);

	// 查找指定内核模块
	// ULONG64 addr = GetModuleBase("ntoskrnl.exe");
	// DbgPrint("addr: %d", addr);

	// 通过特征码搜索
	// ULONG64 addr = RwGetAddrByCode("4C8B324C89742430BA08000000448D42FC488BCE", 40);
	// DbgPrint("addr: %llx", addr);

    // 根据不同操作系统版本调用对应函数
    // GetVersion();
    
    // 注册回调实现进程保护
    // NTSTATUS status = ProcessProtected(pDriver);
    // if (STATUS_SUCCESS == status) DbgPrint("注册回调实现进程保护成功!!!");

    // 修改进程对象头实现进程保护
    NTSTATUS status = EditHeaderProtected(1842);
    if (NT_SUCCESS(status)) DbgPrint("修改进程对象头实现进程保护成功!!!");
	return STATUS_SUCCESS;
}

