// SPDX-License-Identifier: Apache-2.0
// Copyright 2021 - 2022, the Anboto author and contributors
#include <Core/Core.h>

using namespace Upp;

#include <Functions4U/Functions4U.h>
#include <OfficeAutomation/OfficeAutomation.h>


#ifdef _WIN32

bool end = false;

void TestGetchar()
{
	if (end)
		getchar();
}
void Puts(String s)
{
	puts(s);
	String file = AppendFileNameX(GetDesktopFolder(), "Automation demo.log");
	SaveFile(file, LoadFile(file) + "\n" + s);
}

void TestSheetDetail(OfficeSheet &sheet)
{
	Puts("Spreadsheet automation functions demo");

	String test  = AppendFileNameX(GetDesktopFolder(), "Test.xlsx");
	String test2 = AppendFileNameX(GetDesktopFolder(), sheet.GetType() == "Open" ? "TestOpen.xlsx": "TestMicrosoft.xlsx");
	
	if (!sheet.OpenSheet(test, true)) {
		puts(test + " does not exist. Adding new");
		sheet.AddSheet(true);
	}
	
	Puts("Number of tabs is: " + FormatInt(sheet.GetNumTabs()));
	
	if(sheet.InsertTab("My new tab"))
		Puts("New tab inserted");
	else
		Puts("New tab NOT inserted");
	Puts("Press enter to continue...");	getchar();
	
	if(sheet.ChooseTab(1))
		Puts("Tab 1 chosen");
	else
		Puts("Tab 1 NOT chosen");
	Puts("Press enter to continue...");	getchar();
	
	if(sheet.RemoveTab("My new tab"))
		Puts("Tab 'My new tab' removed");
	else
		Puts("Tab 'My new tab' NOT removed");
	Puts("Press enter to continue...");	getchar();

	if(sheet.ChooseTab(0))
		Puts("Tab 0 chosen");
	else
		Puts("Tab 0 NOT chosen");
	Puts("Press enter to continue...");	getchar();
		
	Puts("Changing col width and row height");  
	sheet.SetRowHeight(4, 30);
	sheet.SetColWidth(3, 30);

	Puts("Filling header using SetValue");  
	for (int x = 1; x <= 20; ++x)
		sheet.SetValue(x, 1, Format("Header %d", x));
	
	sheet.Select(2, 2, 5, 4);
	sheet.SetBold(true);	
	sheet.SetItalic(true);	
	sheet.SetUnderline(true);	
	sheet.SetFont("Times New Roman", 12);	
	sheet.SetBorder(OfficeSheet::BORDER_BOTTOM, OfficeSheet::DASHDOT, OfficeSheet::MEDIUM, LtRed());
	sheet.SetBorder(OfficeSheet::BORDER_RIGHT, OfficeSheet::CONTINUOUS, OfficeSheet::THICK, LtGreen());
	
	Puts("Changing cell alignment");  
	sheet.SetHorizAlignment(4, 3, OfficeSheet::CENTER);
	sheet.SetVertAlignment(4, 3, OfficeSheet::CENTER);
	
	Puts("Filling cells using SetValue");  
	for (int y = 2; y <= 10; ++y)
		for (int x = 1; x <= 20; ++x)
			sheet.SetValue(x, y, x*y);	 

			
	Puts("Filling cells MUCH faster using Range functions");  
	
	Vector<Vector<Value> > data;
	for (int y = 0; y < 300; ++y) {
		data.Add();
		for (int x = 0; x < 200; ++x) 
			data[y].Add(x*y);
	}
	sheet.MatrixSet(1, 11, data);
	
	data.Clear();
	sheet.MatrixGet(1, 11, 10, 10, data);
	Puts(Format("Retrieved data from cell 1, 11: %s", AsString(data[0][0])));
	Puts(Format("Retrieved data from cell 5, 16: %s", AsString(data[5][5])));
		
	sheet.SetValue(2, 2, "=A7*B5");
	sheet.SetValue(3, 21, "Hello");				Puts("Cell(3, 21) = " + sheet.GetValue(3, 21).ToString());
	sheet.SetValue("BD25", 23242.343);			Puts("Cell(BD25) = " + sheet.GetValue("BD25").ToString());
	sheet.SetValue(1, 2, "'123456789123456789");
	sheet.SetValue("BE25", OfficeSheet::ColRowToCell(30, 12));
	sheet.SetValue("B26", GetSysTime());		Puts("Cell(B26) = " + sheet.GetValue("B26").ToString());
	sheet.SetValue("D22", "Sum");
	sheet.SetValue("E22", "=sum(E14:E20)");		Puts("Cell(E22) = " + sheet.GetValue("E22").ToString());
	sheet.SetValue("E23", sheet.Function("hyperlink", "\"http://www.ultimatepp.org\"", "\"Ultimate link\""));	Puts("Cell(E23) = " + sheet.GetValue("E23").ToString());
	
	Puts("\nPress enter to check Replace()");	TestGetchar();
	sheet.Select();		// Select all active sheet
	sheet.Replace("Hello", "Hello World");
	Puts("All done.");

	//sheet.Print();	// Be careful. This demo will print many sheets of paper

	FileDelete(test2);
	
	Puts("\nPress enter to end TestSheet...");	TestGetchar();
	
	sheet.SaveAs(test2, "xlsx");
	sheet.Quit();  	
}

void TestSheet()
{
	OfficeSheet sheet;
	
	bool openAvailable = sheet.IsAvailable("Open");
	bool microsoftAvailable = sheet.IsAvailable("Microsoft");
	
	Puts(openAvailable 		? "OpenOffice Calc is available": "OpenOffice Calc is not available");
	Puts(microsoftAvailable ? "MS Office Excel is available": "MS Office Excel is not available");
	
	if (openAvailable) {
		sheet.Init("Open");
		TestSheetDetail(sheet); 
	}
	if (microsoftAvailable) {
		sheet.Init("Microsoft");
		TestSheetDetail(sheet);
	}
}

void TestDocDetail(OfficeDoc &doc)
{
	Puts("Word processor automation functions demo");

	String test  = AppendFileNameX(GetDesktopFolder(), "Test.docx");
	String test2 = AppendFileNameX(GetDesktopFolder(), doc.GetType() == "Open" ? "TestOpen.docx": "TestMicrosoft.docx");
	
	if (!doc.OpenDoc(test, true)) {
		puts(test + " does not exist. Adding new");
		doc.AddDoc(true);
	}
	//doc.SetVisible(false);
	
	doc.SetFont("Arial", 14);
	doc.WriteText("Hello all!");
	doc.SetFont("Arial", 12); 
	doc.SetBold(true);
	doc.WriteText("\nNow it is bold");
	doc.SetBold(false);
	doc.SetItalic(true);
	doc.WriteText("\nAnd now is italic");
	doc.SetItalic(false);
	doc.WriteText("\n\nNon me latet [To replace] nonnullos vestrum hoc libro gavisuros atque obiter ampliore munituros esse. [To replace]\nXaverius noster, struenissimus Latinitatis vivae cultor catalaunicus, nobis misit inscriptionem situs, quo commodius legamus litteras erasmianas. Gratias tibi pro benevolentia agimus quam plurimas.");
	
	//doc.Print(); // Try it

	Puts("\nPress enter to check Replace()");	TestGetchar();
	doc.Select();		// Select all doc
	String replace = "Ultimate++ is a C++ cross-platform rapid application development suite focused on programmers productivity.\nIt includes a set of libraries (GUI, SQL, etc..), and an integrated development environment.\nRapid development is achieved by the smart and aggressive use of C++ rather than through fancy code generators.\nIn this respect, U++ competes with popular scripting languages while preserving C/C++ runtime characteristics.\nThe U++ integrated development environment, TheIDE, introduces modular concepts to C++ programming.\nIt features BLITZ-build technology to speedup C++ rebuilds up to 4 times, Visual designers for U++ libraries, Topic++ system for documenting code and creating rich text resources for applications (like help and code documentation) and Assist++ - a powerful C++ code analyzer that provides features like code completion, navigation and transformation.\nTheIDE can work with GCC, MinGW and Visual C++ 7.1 or 8.0 compilers (including free Visual C++ Toolkit 2003 and Visual C++ 2005 Express Edition) and contains a full featured debugger.\nTheIDE can also be used to develop non-U++ applications.\nU++ distributions combine U++ with 3rd party tools like MinGW compiler or SDL library to provide an instant development platform.\nWhat you can get with the Ultimate++ download in plain English\n - Very effective C++ library for cross-platform development in source form.\n - A good integrated development environment, designed for developing large C++ applications.\nYou can use both, or you can use whichever you need.\nSome special characters to test: >=<ºª%€&()$1234567890áéíóúÁÉÍÓÚñÑçÇ,.,:-_/¿?+*[]{}'\"!¡\n";
	Puts(Format("Result: %s", doc.Replace("[To replace]", replace)? "Well done": "Problems replacing"));
	Puts("All done.");

	FileDelete(test2);
	
	Puts("\nPress enter to end TestDoc...");	TestGetchar();

	doc.SaveAs(test2, "docx");
	doc.Quit();  	
}

void TestDoc()
{
	OfficeDoc doc;
	
	bool openAvailable = doc.IsAvailable("Open");
	bool microsoftAvailable = doc.IsAvailable("Microsoft");
	
	Puts(openAvailable 		? "OpenOffice Text is available": "OpenOffice Text is not available");
	Puts(microsoftAvailable ? "MS Office Word is available": "MS Office Word is not available");
	
	if (openAvailable) {
		doc.Init("Open");
		TestDocDetail(doc);
	}
	if (microsoftAvailable) {
		doc.Init("Microsoft");
		TestDocDetail(doc);
	}
}


CONSOLE_APP_MAIN
{
	FileDelete(AppendFileNameX(GetDesktopFolder(), "Automation demo.log"));
		
	Puts("Introduce number of test cycles or just type enter to run it once: ");
	char str[50];	
	fgets(str, 49, stdin);
	int numTests = atoi(str);
	for (int i = 0; i < numTests; ++i) {
		TestSheet();
		TestDoc();
	}
	end = true;
	TestSheet();
	TestDoc();
}

#else

CONSOLE_APP_MAIN 
{
	puts("\nOfficeAutomation package only works in Windows");	
	puts("\nPress a key to end...");		getchar();
}

#endif