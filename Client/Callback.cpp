#include "Callback.h"

// Win7 64位操作系统之下触发回调函数
int Win7Callback() {
	// 准备用于读写指定进程内存时所有参数
	ULONG64 readData = 0;
	HWND hwnd = FindWindowA(NULL, "Fate鼠标精灵");
	DWORD piid = 0;
	GetWindowThreadProcessId(hwnd, &piid);
	PRWMM rwmm = (PRWMM)malloc(sizeof(RWMM));
	memset(rwmm, 0, sizeof(RWMM));
	rwmm->pid = piid;
	rwmm->start = 0x400000;
	rwmm->dest = (ULONG64)&readData;
	rwmm->size = sizeof(ULONG64);

	// 获取 _NtQueryInformationFile 函数地址
	_NtQueryInformationFile MyQueryInfoFile = NULL;
	MyQueryInfoFile = (_NtQueryInformationFile)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationFile");
	if (NULL == MyQueryInfoFile) {
		std::cerr << "无法获取NtQueryInformationFile函数地址!!!" << std::endl;
		return -1;
	}

	// 创建文件对象，作为通信的触发器
	HANDLE hFile = CreateFileA("C:\\Client.txt", FILE_ALL_ACCESS, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile) {
		DWORD error = GetLastError();
		std::cerr << "创建文件失败，错误码: " << error << std::endl;
		return -1;
	}

	// 准备通信缓冲区
	IO_STATUS_BLOCK ioStatus = { 0 };	// 用于接收IO状态
	char fileBuffer[0xE0] = { 0 };		// 224字节的缓冲区(应用-驱动层通信桥梁)
	std::cout << "应用层 buffer 地址: 0x" << std::hex << (ULONG64)fileBuffer << std::dec << std::endl;

	PMESSAGE_PACKAGE message = (PMESSAGE_PACKAGE)fileBuffer;
	message->flag = 1234;
	message->func = 1;
	message->data = (ULONG64)rwmm;

	// 触发驱动层回调函数
	NTSTATUS status = MyQueryInfoFile(hFile, &ioStatus, fileBuffer, 0xE0, 0x34);
	if (0 != status) {
		std::cerr << "NtQueryInformationFile调用失败，状态码: 0x" << std::hex << status << std::endl;
	}
	else {
		std::cout << "调用成功完成，检查缓冲区内容" << std::endl;
	}
	printf("R0读取的数据为:%d\n", readData);
}

// 内存属性查询
int QueryVirtualMemory() {
	HWND hwdn = FindWindowA(NULL, "Fate鼠标精灵");
	DWORD piid = 0;
	GetWindowThreadProcessId(hwdn, &piid);
	PRWMM rwmm = (PRWMM)malloc(sizeof(RWMM));
	PMYMEMORY_BASIC_INFORMATION baseInfo = (PMYMEMORY_BASIC_INFORMATION)malloc(sizeof(MYMEMORY_BASIC_INFORMATION));
	memset(baseInfo, 0, sizeof(MYMEMORY_BASIC_INFORMATION));
	memset(rwmm, 0, sizeof(RWMM));
	ULONG64 readData = 0;
	rwmm->pid = piid;
	rwmm->start = 0x400000;
	rwmm->dest = (ULONG64)baseInfo;
	// rwmm->size = sizeof(ULONG64);
	// 获取 _NtQueryInformationFile 函数地址
	_NtQueryInformationFile myQueryInformationFile = NULL;
	myQueryInformationFile = (_NtQueryInformationFile)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationFile");

	// 创建文件对象，作为通信的触发器
	HANDLE hFile = CreateFileA("C:\\test.txt", FILE_ALL_ACCESS, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// 准备通信缓冲区
	IO_STATUS_BLOCK ioStatusBlock = { 0 };    // 用于接收IO状态
	char fileBuffer[0XE0] = { 0 };            // 224字节的缓冲区
	PMESSAGE_PACKAGE message = (PMESSAGE_PACKAGE)fileBuffer;
	message->flag = 1234;
	message->func = 2;
	message->data = (ULONG64)rwmm;
	// 触发驱动层回调函数
	myQueryInformationFile(hFile, &ioStatusBlock, fileBuffer, 0xE0, 0x34);
	printf("AllocationBase：%x\n", baseInfo->AllocationBase);
	printf("AllocationBase：%x\n", baseInfo->AllocationProtect);
	printf("BaseAddress：%x\n", baseInfo->BaseAddress);
	printf("Protect：%x\n", baseInfo->Protect);
	printf("State：%x\n", baseInfo->State);
	printf("Type：%x\n", baseInfo->Type);
	return 0;
}