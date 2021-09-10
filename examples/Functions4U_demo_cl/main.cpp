#include <Core/Core.h>

using namespace Upp;

#include "Spreadsheet.h"


void SpreadsheetDemo(Spreadsheet &spreadsheet) {
	spreadsheet.Open("myfile.xls");
	spreadsheet.SetData(4, 6, "Hello world");
}

void PluginDemo() {
	Spreadsheet spreadsheet;
	
	if (!PluginInit(spreadsheet, "Open")) {
		UppLog() << "Impossible to init OpenOffice/LibreOffice";
		return;
	}
	SpreadsheetDemo(spreadsheet);
	if (!PluginInit(spreadsheet, "Excel")) {
		UppLog() << "Impossible to init Excel";
		return;
	}
	SpreadsheetDemo(spreadsheet);
}

void FilesDemo() {
	{
		String from = "/books/technology/computers"; 
		String path = "/books/biology/mammals";
		String ret;
		if (!GetRelativePath(from, path, ret))
			ret = "Null";
		Cout() << "\nGetRelativePath(\"" << from << "\", \"" << path << "\")\n= \"" << ret << "\" (should be: \"../../biology/mammals\")";
	}
	{
		String from = "/books/technology/computers";
		String path = "/books/technology/computers";
		String ret;
		if (!GetRelativePath(from, path, ret))
			ret = "Null";
		Cout() << "\nGetRelativePath(\"" << from << "\", \"" << path << "\")\n= \"" << ret << "\" (should be: \"\")";
	}
	{
		String from = "/books/technology/computers";
		String path = "/books2/biology/mammals";
		String ret;
		if (!GetRelativePath(from, path, ret))
			ret = "Null";
		Cout() << "\nGetRelativePath(\"" << from << "\", \"" << path << "\")\n= \"" << ret << "\" (should be: \"../../../books2/biology/mammals\")";
	}
	{
		String from = "c:/books/technology/computers";
		String path = "y:/books2/biology/mammals";
		String ret;
		if (!GetRelativePath(from, path, ret))
			ret = "Null";
		Cout() << "\nGetRelativePath(\"" << from << "\", \"" << path << "\")\n= \"" << ret << "\" (should be: Null)";
	}
	
	String filename1 = AppendFileNameX(GetExeFolder(), "Demo", "file1.txt");
	RealizePath(filename1);
	String str1 = "This is the First string";
	SaveFile(filename1, str1);
	String filename2 = AppendFileNameX(GetExeFolder(), "Demo", "file2.txt");
	String str2 = "This is the Second string";
	SaveFile(filename2, str2);
	
	FileCat(filename1, filename2);
	UppLog() << Format("\nFileCat(%s, %s)", filename1, filename2);

//	LaunchFile(filename1);	
//	UppLog() << Format("LaunchFile(%s)", filename1);		

	int intres = FileCompare(filename1, filename2);
	UppLog() << Format("\nFileCompare(%s, %s) = %d (has to be -1)", filename1, filename2, intres);

	int64 pos = FindStringInFile(filename1, "Second");
	UppLog() << Format("\nFindStringInFile(%s, %s) = %d (has to be 35)", filename1, filename2, pos);

	bool boolres = FileStrAppend(filename1, ". Text appended at the end.");
	UppLog() << Format("\nFileStrAppend(%s, \". Text appended at the end.\") = %s (has to be true)", 
				filename1, boolres ? "true" : "false");
	
	//boolres = UpperFolder(GetFileDirectory(filename1));
	//UppLog() << Format("\nUpperFolder(%s) = %s (has to be true)", filename1, boolres ? "true" : "false");
	
	String upperFolder = GetUpperFolder(GetFileDirectory(filename1));
	UppLog() << Format("\nGetUpperFolder(%s) = %s (has to be %s)", filename1, upperFolder, GetExeFolder());
	
	String stringres = GetNextFolder(GetUpperFolder(GetExeFolder()), GetFileDirectory(filename1));
	UppLog() << Format("\nGetNextFolder(%s, %s) = %s (has to be %s)", GetUpperFolder(GetExeFolder()), 
										GetFileDirectory(filename1), stringres, GetExeFolder());
	
	String lfilename1 = ToLower(filename1);
	filename1 = FileRealName(lfilename1);
	UppLog() << Format("\nFileRealName(%s) = %s", lfilename1, filename1);
}

void NonReentrantDemo() {
	UppLog() << "\nTrying to enter Non reentrant. ";
	NON_REENTRANT_V;
	
	UppLog() << "Entered in Non reentrant. It has to be once.";
	NonReentrantDemo();	
	NonReentrantDemo();
}

void DistanceDemo() {
	UppLog() << Format("Distance between 'hello' and 'hello'  is %d", DamerauLevenshteinDistance("hello", "hello"));
	UppLog() << Format("Distance between 'hello' and 'helo'   is %d", DamerauLevenshteinDistance("hello", "helo"));
	UppLog() << Format("Distance between 'hello' and 'heloo'  is %d", DamerauLevenshteinDistance("hello", "helloo"));
	UppLog() << Format("Distance between 'hello' and 'yellow' is %d", DamerauLevenshteinDistance("hello", "yellow"));
}

void MiscellaneousDemos() {
	UppLog() << "Float formatting\n";
	
	const double num = 2345678.9017654;
	UppLog() << "\nFormatting 2345678.9017654";
	String str;
	UppLog() << "\nFormatF(, 2) = '" << (str = FormatF(num, 2)) << "'";
	VERIFY(str == "2345678.90");
	UppLog() << "\nFormatF(, 3) = '" << (str = FormatF(num, 3)) << "'";
	VERIFY(str == "2345678.902");
	UppLog() << "\nFormatF(, 4) = '" << (str = FormatF(num, 4)) << "'";
	VERIFY(str == "2345678.9018");
	UppLog() << "\nFormatF(, 5) = '" << (str = FormatF(num, 5)) << "'";
	VERIFY(str == "2345678.90177");
	
	UppLog() << "\nFormatDoubleSize(, 3) = '" << (str = FormatDoubleSize(num, 3, true)) << "'";
	VERIFY(str == "2.3E6");
	UppLog() << "\nFormatDoubleSize(, 4) = '" << (str = FormatDoubleSize(num, 4, true)) << "'";
	VERIFY(str == "2.3E6");
	UppLog() << "\nFormatDoubleSize(, 5) = '" << (str = FormatDoubleSize(num, 5, true)) << "'";
	VERIFY(str == "2.3E6");
	UppLog() << "\nFormatDoubleSize(, 6) = '" << (str = FormatDoubleSize(num, 6, true)) << "'";
	VERIFY(str == "2.35E6");
	UppLog() << "\nFormatDoubleSize(, 7) = '" << (str = FormatDoubleSize(num, 7, true)) << "'";
	VERIFY(str == "2345679");
	UppLog() << "\nFormatDoubleSize(, 8) = '" << (str = FormatDoubleSize(num, 8, true)) << "'";
	VERIFY(str == " 2345679");
	UppLog() << "\nFormatDoubleSize(, 9) = '" << (str = FormatDoubleSize(num, 9, true)) << "'";
	VERIFY(str == "2345678.9");
	UppLog() << "\nFormatDoubleSize(,10) = '" << (str = FormatDoubleSize(num,10, true)) << "'";
	VERIFY(str == " 2345678.9");
	UppLog() << "\nFormatDoubleSize(,11) = '" << (str = FormatDoubleSize(num,11, true)) << "'";
	VERIFY(str == "2345678.902");
	UppLog() << "\nFormatDoubleSize(,12) = '" << (str = FormatDoubleSize(num,12, true)) << "'";
	VERIFY(str == "2345678.9018");
	UppLog() << "\nFormatDoubleSize(,13) = '" << (str = FormatDoubleSize(num,13, true)) << "'";
	VERIFY(str == "2345678.90177");
	
	UppLog() << "\n";
	
	SetConsoleColor(CONSOLE_COLOR::LTRED);
	UppLog() << "This message is in red\n";
	SetConsoleColor(CONSOLE_COLOR::LTYELLOW);
	UppLog() << "This message is in yellow\n";
	SetConsoleColor(CONSOLE_COLOR::RESET);
	UppLog() << "This message is in normal color\n";
	
	UppLog() << "Next text messages (printf, Cout(), UppLog()) will be disabled\n";
	ConsoleOutputDisable(true);
	Cout() << "Text with Cout()\n";
	UppLog() << "Text with UppLog()\n";
	printf("Text with printf()\n");
	ConsoleOutputDisable(false);
	UppLog() << "Text messages are now enabled\n";
}

CONSOLE_APP_MAIN
{	
	StdLogSetup(LOG_COUT|LOG_FILE);

	UppLog() << "Miscellaneous demos";
	MiscellaneousDemos();
	
	UppLog() << "\n\nFiles demo";
	FilesDemo();
	
	UppLog() << "\n\nPlugin demo";
	PluginDemo();	

	UppLog() << "\n\nNonReentrant demo";
	NonReentrantDemo();	
	
	UppLog() << "\n\nDistance demo";
	DistanceDemo();	
	
	#ifdef flagDEBUG
	UppLog() << "\n";
	Cout() << "\nPress enter key to end";
	ReadStdIn();
	#endif   
}

