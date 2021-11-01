// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2021, the Anboto author and contributors
#include <Core/Core.h>
#include <CrashHandler/Crash.h>

using namespace Upp;


CONSOLE_APP_MAIN
{	
	StdLogSetup(LOG_COUT|LOG_FILE);
	SetExitCode(0);
	
	try {
		UppLog() << "\nTesting division by zero";  
		
		double a = 4, b = 0;
		
		double c = a/b;
		
		UppLog() << "\nPassed";  


	} catch (Exc e) {
		UppLog() << "\nError: " << e << "\n";  
		SetExitCode(1);
	} catch (...) {
		UppLog() << "\nUnknown Error\n";  
		SetExitCode(1);
	}
	
//	#ifdef flagDEBUG
	UppLog() << "\n";
	Cout() << "\nPress enter key to end";
	ReadStdIn();
//	#endif  
}
