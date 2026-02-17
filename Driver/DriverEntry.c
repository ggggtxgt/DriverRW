#include "Util.h"
#include "RwGetModule.h"
#include "RwProtected.h"
#include "GetModuleUitl.h"

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

VOID processCreateFileter(_In_ HANDLE ParentId, _In_ HANDLE ProcessId, _In_ BOOLEAN Create) {
    if (Create) {
        DbgPrint("进程已创建!");
    }
    else {
        DbgPrint("进程已销毁!");
    }
}

VOID threadCreateFilter(_In_ HANDLE ProcessId, _In_ HANDLE ThreadId, _In_ BOOLEAN Create) {
    if (Create) {
        DbgPrint("线程已创建!");
    }
    else {
        DbgPrint("线程已销毁!");
    }
}

VOID loadImageFilter(_In_opt_ PUNICODE_STRING FullImageName, _In_ HANDLE ProcessId, _In_ PIMAGE_INFO ImageInfo) {
    DbgPrint("已有模块加载!");
}

VOID cancelProcess() {
    PsSetCreateProcessNotifyRoutine(processCreateFileter, TRUE);
}

VOID cancelThread() {
    PsSetCreateProcessNotifyRoutine(threadCreateFilter, TRUE);
}

VOID cancelImage() {
    PsSetCreateProcessNotifyRoutine(loadImageFilter, TRUE);
}

/********************************************************************************************************************
 * @brief   驱动卸载回调函数：当驱动被卸载时，系统自动启用该函数；
 * @param   驱动对象的结构体指针；
********************************************************************************************************************/
void DriverUnload(PDRIVER_OBJECT pDriver) {
    DbgPrint("DriverUnload!!!");
    // UnRegisterCallback();
    // ObUnRegister();
    cancelProcess();
    cancelThread();
    cancelImage();
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
    // NTSTATUS status = EditHeaderProtected(1842);
    // if (NT_SUCCESS(status)) DbgPrint("修改进程对象头实现进程保护成功!!!");

    // 创建进程
    NTSTATUS state = PsSetCreateProcessNotifyRoutine(processCreateFileter, FALSE);
    if (NT_SUCCESS(state)) DbgPrint("注册系统回调成功!\n");
    else DbgPrint("注册系统回调失败!");

    // 创建线程
    state = PsSetCreateThreadNotifyRoutine(threadCreateFilter);
    if (NT_SUCCESS(state)) DbgPrint("注册系统回调成功!\n");
    else DbgPrint("注册系统回调失败!");

    // 模块
    state = PsSetLoadImageNotifyRoutine(loadImageFilter);
    if (NT_SUCCESS(state)) DbgPrint("注册系统回调成功!\n");
    else DbgPrint("注册系统回调失败!");

	return STATUS_SUCCESS;
}