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
	// RegisterCallBack();
	
	// 获取指定进程模块
	// ULONG64 addr = RwGetModuleHandle64(1192, "kernel32.dll");
	// DbgPrint("addr: %d", addr);

	// 查找指定内核模块
	ULONG64 addr = GetModuleBase("ntoskrnl.exe");
	DbgPrint("addr: %d", addr);

	return STATUS_SUCCESS;
}