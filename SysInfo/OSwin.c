// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#ifdef _WIN32


#include <windows.h>
#include "OSwin.h"

PGNSI Get_GetNativeSystemInfo() {
   	return (PGNSI)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
}

PGPI Get_GetProductInfo() {
	return (PGPI)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");
}


#endif