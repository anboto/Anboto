// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#ifndef _CrashHandler_Crash_h_
#define _CrashHandler_Crash_h_

#include <Core/Core.h>

namespace Upp {

class CrashHandler {
public:
    CrashHandler();
    virtual ~CrashHandler() {};

private:
#if defined(PLATFORM_WIN32) 
    static LONG WINAPI UnhandledHandler(EXCEPTION_POINTERS *p);
    static void __cdecl SEHHandler(unsigned u, EXCEPTION_POINTERS* p);
#endif
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
    static void __cdecl TerminateHandler();
    static void __cdecl UnexpectedHandler();

    static void __cdecl PureCallHandler();

    static void __cdecl InvalidParameterHandler(const wchar_t* expression, 
        const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved);

    static void __cdecl NewHandler();
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
    static void SigabrtHandler(int);
    static void SigfpeHandler(int);
    static void SigintHandler(int);
    static void SigillHandler(int);
    static void SigsegvHandler(int);
    static void SigtermHandler(int);
};

}

#endif
