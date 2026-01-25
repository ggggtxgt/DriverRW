#pragma once

#include <ntifs.h>

OB_PREOP_CALLBACK_STATUS PobPreOperationCallback(PVOID RegistrationContext, POB_PRE_OPERATION_CALLBACK OperationInformation);
void PobPostOperationCallback(PVOID RegistrationContext, POB_POST_OPERATION_CALLBACK OperationInformation);
void InstallCallback();
void UninstallCallback();
