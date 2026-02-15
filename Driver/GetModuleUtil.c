/********************************************************************************************************************
 * @brief   GetModuleUitl.c
 * @details 实现读取内核进程模块的相关操作；
********************************************************************************************************************/

#include "GetModuleUitl.h"

// 获取指定内核模块基地址
ULONG64 GetModuleBase(PCHAR moduleName) {
	// 由于不知道所需缓冲区大小，先随意指定大小
	ULONG64 testSize = 0;
	// 该参数可以收到真正所需的缓冲区大小
	ULONG realLength = 0;
	PVOID moduleBuff = NULL;

	// 第一次调用，仅获取所需缓冲区大小
	NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, &testSize, sizeof(ULONG64), &realLength);
	if (STATUS_INFO_LENGTH_MISMATCH == status) {
		moduleBuff = ExAllocatePool(NonPagedPool, realLength);
		if (!moduleBuff) return 0;

		// 第二次调用，实际获取模块信息
		status = ZwQuerySystemInformation(SystemModuleInformation, moduleBuff, realLength, &realLength);
		if (!NT_SUCCESS(status)) {
			ExFreePool(moduleBuff);
			return 0;
		}

		// 获取模块数量（结构体第一个字段是 NumberOfModules，类型为 ULONG64）
		ULONG64 modulesCount = *(ULONG64*)moduleBuff;
		if (0 == modulesCount) {
			ExFreePool(moduleBuff);
			return 0;
		}

		// 指向第一个模块信息结构的指针
		PRTL_PROCESS_MODULE_INFORMATION moduleArray = (PRTL_PROCESS_MODULE_INFORMATION)((ULONG64)moduleBuff + sizeof(ULONG64));
		for (size_t i = 0; i < modulesCount; i++) {
			PCHAR fullName = moduleArray->FullPathName;		// 模块完整路径名
			if (strstr(fullName, moduleName)) {
				DbgPrint("找到指定模块!");
				ULONG64 baseAddr = moduleArray->ImageBase;	// 获取模块基址
				ExFreePool(moduleBuff);
				return baseAddr;
			}
			moduleArray++;
		}
	}
	if (moduleBuff) ExFreePool(moduleBuff);
	return 0;
}

// byte 转换为 十六进制字符
void ByteToHexStr(const unsigned char* source, char* dest, int sourceLen) {
	short i;
	unsigned char highByte, lowByte;
	for (i = 0; i < sourceLen; i++) {
		highByte = source[i] >> 4;		// 取出高4位
		lowByte = source[i] & 0x0f;		// 取出低4位

		// 将高4位转换为 ASCII 字符（0-9，A-F）
		highByte += 0x30;				// 0x30 为字符 '0'
		if (highByte > 0x39) {			// 大于 '9' 则需要加上 0x07 得到 'A'-'F'
			dest[i * 2] = highByte + 0x07;
		} else {
			dest[i * 2] = highByte;
		}

		// 将低4位转换为 ASCII 字符
		lowByte += 0x30;
		if (lowByte > 0x39) {
			dest[i * 2 + 1] = lowByte + 0x07;
		} else {
			dest[i * 2 + 1] = lowByte;
		}
	}
	return;
}

// 通过特征码进行搜索
ULONG64 RwSearchCode(char* beginAddr, char* endAddr, char* code, ULONG codeLen) {
	ULONG64 resultAddr = 0;
	BOOLEAN cmpFlag = TRUE;

	// 分配临时缓冲区用于存储转换后的十六进制字符串
	char* convertCode = ExAllocatePool(NonPagedPool, (codeLen + 2));
	if (NULL == convertCode) {
		DbgPrint("NULL == convertCode");
		return resultAddr;
	}

	// 在指定地址范围内逐字节对比
	for (size_t i = beginAddr; i < endAddr; i++) {
		memset(convertCode, 0, (codeLen + 2));
		// 参数为 codeLen / 2 为了适用 ByteToHexStr 函数
		ByteToHexStr(beginAddr, convertCode, codeLen / 2);
		for (size_t j = 0; j < codeLen; j++) {
			// 通配符或字符匹配
			if ('?' == code[j] || convertCode[j] == code[j]) {
				continue;
			} else {
				cmpFlag = FALSE;
				break;
			}
			if (TRUE == cmpFlag) {
				resultAddr = beginAddr;
				break;
			}
			beginAddr++;
		}
	}
	if (convertCode) ExFreePool(convertCode);
	DbgPrint("RwSearchCode: resultAddr -> %d", resultAddr);
	return resultAddr;
}

// 通过特征码获取地址
ULONG64 RwGetAddrByCode(char* code, ULONG codeLen) {
	ULONG64 resultAddr = 0;
	ULONG64 moduleAddr = GetModuleBase("ntoskrnl.exe");
	if (0 == moduleAddr) {
		DbgPrint("0 == moduleAddr!!!");
		return resultAddr;
	}

	// 解析 DOS 头部
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)moduleAddr;
	// 解析 NT 头部
	PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)(moduleAddr + pDosHeader->e_lfanew);
	// 获取第一个节头
	PIMAGE_SECTION_HEADER pFirstSection = IMAGE_FIRST_SECTION(pNtHeader);
	BOOLEAN cmpFlag = FALSE;
	// 遍历所有节，查找名为 "PAGE" 的节
	for (size_t i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++) {
		char sectionName[9] = { 0 };
		memcpy(sectionName, pFirstSection->Name, 8);
		if (0 == _stricmp("PAGE", sectionName)) {
			cmpFlag = TRUE;
			DbgPrint("cmpFlag = TRUE;");
			break;
		}
		pFirstSection++;
	}
	if (FALSE == cmpFlag) {
		DbgPrint("RwGetAddrByCode Error!!!");
		return resultAddr;
	}
	// 计算 PAGE 节在内存之中的起始和结束地址
	char* beginAddr = pFirstSection->VirtualAddress + moduleAddr;
	char* endAddr = pFirstSection->VirtualAddress + moduleAddr + pFirstSection->SizeOfRawData;
	resultAddr = RwSearchCode(beginAddr, endAddr, code, codeLen);
	return resultAddr;
}