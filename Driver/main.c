#include <ntifs.h>

void DriverUnload(PDRIVER_OBJECT pDriver) {

}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath) {
	pDriver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}