#include "GetModuleUtil.h"

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