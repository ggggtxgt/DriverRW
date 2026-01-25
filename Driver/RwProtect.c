#include "RwProtect.h"

HANDLE hCallback = NULL;

OB_PREOP_CALLBACK_STATUS PobPreOperationCallback(PVOID RegistrationContext, POB_PRE_OPERATION_CALLBACK OperationInformation) {
	return STATUS_SUCCESS;
}

void PobPostOperationCallback(PVOID RegistrationContext, POB_POST_OPERATION_CALLBACK OperationInformation) {

}

void InstallCallback() {
	OB_OPERATION_REGISTRATION obOperRegister = { 0 };
	obOperRegister.ObjectType = PsProcessType;
	obOperRegister.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
	obOperRegister.PreOperation = PobPreOperationCallback;
	obOperRegister.PostOperation = PobPostOperationCallback;
	OB_CALLBACK_REGISTRATION obCallback = { 0 };
	obCallback.Version = ObGetFilterVersion();
	obCallback.OperationRegistrationCount = 1;
	obCallback.RegistrationContext = NULL;
	RtlInitUnicodeString(&obCallback.Altitude, L"obCallback.Altitude");
	obCallback.OperationRegistration = &obOperRegister;
	NTSTATUS status = ObRegisterCallbacks(&obCallback, &hCallback);
}

void UninstallCallback() {

}
