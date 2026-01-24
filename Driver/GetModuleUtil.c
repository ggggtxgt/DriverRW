#include "GetModuleUtil.h"

#include <ntimage.h>

ULONG64 GetModuleBase(PCHAR moduleName) {
	ULONG64 testSize = 0;
	ULONG realLength = 0;
	PVOID moduleBuff = NULL;

	NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, &testSize, sizeof(ULONG64), &realLength);
	if (STATUS_INFO_LENGTH_MISMATCH == status) {
		moduleBuff = ExAllocatePool(NonPagedPool, realLength);
		if (!moduleBuff) {
			return 0;
		}
		status = ZwQuerySystemInformation(SystemModuleInformation, moduleBuff, realLength, &realLength);
		if (NT_SUCCESS(status)) {
			ExFreePool(moduleBuff);
			return 0;
		}
		ULONG64 modulesCount = *(ULONG64*)moduleBuff;
		if (0 == modulesCount) {
			ExFreePool(moduleBuff);
			return 0;
		}
		PRTL_PROCESS_MODULE_INFORMATION moduleArray = (PRTL_PROCESS_MODULE_INFORMATION)((ULONG64)moduleBuff + sizeof(ULONG64));
		for (size_t i = 0; i < modulesCount; i++) {
			PCHAR fullName = moduleArray->FullPathName;
			if (strstr(fullName, moduleName)) {
				DbgPrint("找到指定模块!");
				ExFreePool(moduleBuff);
				return moduleArray->ImageBase;
			}
			moduleArray++;
		}
	}
	if (moduleBuff) {
		ExFreePool(moduleBuff);
	}
	return 0;
}

void ByteToHexStr(const unsigned char *source, char *dest, int sourceLen) {
	short i;
	unsigned char highByte, lowByte;
	for (i = 0; i < sourceLen; i++) {
		highByte = source[i] >> 4;
		lowByte = source[i] & 0x0f;
		highByte += 0x30;
		if (highByte > 0x39) {
			dest[i * 2] = highByte + 0x07;
		} else {
			dest[i * 2] = highByte;
		}
		lowByte += 0x30;
		if (lowByte > 0x39) {
			dest[i * 2 + 1] = lowByte + 0x07;
		} else if (highByte > 0x39) {
			dest[i * 2] = highByte + 0x07;
		} else {
			dest[i * 2] = highByte;
		}
		lowByte += 0x30;
		if (lowByte > 0x39) {
			dest[i * 2 + 1] = lowByte + 0x07;
		} else {
			dest[i * 2 + 1] = lowByte;
		}
	}
	return;
}

ULONG64 RwSearchCode(char* beginAddr, char* endAddr, char* code, ULONG codeLen) {
	ULONG64 resultAddr = 0;
	BOOLEAN cmpFlag = TRUE;
	char* convertCode = ExAllocatePool(NonPagedPool, codeLen);
	if (NULL == convertCode) {
		return resultAddr;
	}
	for (size_t i = beginAddr; i < endAddr; i++) {
		memset(convertCode, 0, codeLen);
		ByteToHexStr(beginAddr, convertCode, codeLen);
		for (size_t j = 0; j < codeLen; j++) {
			if ('?' == code || convertCode[j] == code) {
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
	if (convertCode) {
		ExFreePool(convertCode);
	}
	return resultAddr;
}

ULONG64 RwGetAddrByCode(char* code, ULONG codeLen) {
	ULONG64 resultAddr = 0;
	ULONG64 moduleAddr = GetModuleBase("ntoskrnl.exe");
	if (0 == moduleAddr) {
		return resultAddr;
	}
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)moduleAddr;
	PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)(moduleAddr + pDosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER pFirstSection = IMAGE_FIRST_SECTION(pNtHeader);
	BOOLEAN cmpFlag = FALSE;
	for (size_t i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++) {
		char sectionName[9] = { 0 };
		memcpy(sectionName, pFirstSection->Name, 8);
		if (0 == _stricmp("PAGE", sectionName)) {
			cmpFlag = TRUE;
			break;
		}
		pFirstSection++;
	}
	if (FALSE == cmpFlag) {
		DbgPrint("RwGetAddrByCode Error!");
		return resultAddr;
	}
	char* beginAddr = pFirstSection->VirtualAddress + moduleAddr;
	char* endAddr = pFirstSection->VirtualAddress + moduleAddr + pFirstSection->SizeOfRawData;
	resultAddr = RwSearchCode(beginAddr, endAddr, code, codeLen);
	return resultAddr;
}