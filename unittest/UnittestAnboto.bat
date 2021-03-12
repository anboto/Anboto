
md test

umk Anboto examples/Functions4U_demo_cl CLANGx64 -r   test\Functions4U_DemoCLANGx64.exe
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1
test\Functions4U_DemoCLANGx64.exe	
@IF %ERRORLEVEL% NEQ 0  EXIT /B 1
umk Anboto examples/Functions4U_demo_cl MSBT19x64 -r   test\Functions4U_DemoMSBT19x64.exe	
@IF %ERRORLEVEL% NEQ 0  EXIT /B 1
test\Functions4U_DemoMSBT19x64.exe	
@IF %ERRORLEVEL% NEQ 0  EXIT /B 1

umk Anboto examples/SysInfo_demo_cl CLANGx64 -r   test\SysInfo_demo_consoleCLANGx64.exe
@IF %ERRORLEVEL% NEQ 0  EXIT /B 1
test\SysInfo_demo_consoleCLANGx64.exe	
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1
umk Anboto examples/SysInfo_demo_cl MSBT19x64 -r   test\SysInfo_demo_consoleMSBT19x64.exe	
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1
test\SysInfo_demo_consoleMSBT19x64.exe	
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1

umk Anboto examples/STEM4U_demo_test_cl CLANGx64 -r   test\STEM4U_DemoCLANGx64.exe
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1
test\STEM4U_DemoCLANGx64.exe -test	
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1
umk Anboto examples/STEM4U_demo_test_cl MSBT19x64 -r   test\STEM4U_DemoMSBT19x64.exe	
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1
test\STEM4U_DemoMSBT19x64.exe -test
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1

umk Anboto examples/Eigen_demo_cl CLANGx64 -r   test\Eigen_DemoCLANGx64.exe
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1
test\Eigen_DemoCLANGx64.exe	
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1
umk Anboto examples/Eigen_demo_cl MSBT19x64 -r   test\Eigen_DemoMSBT19x64.exe	
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1
test\Eigen_DemoMSBT19x64.exe	
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1

umk Anboto examples/ScatterDraw_demo_cl CLANGx64 -r   test\ScatterDraw_DemoCLANGx64.exe
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1
test\ScatterDraw_DemoCLANGx64.exe	
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1
umk Anboto examples/ScatterDraw_demo_cl MSBT19x64 -r   test\ScatterDraw_DemoMSBT19x64.exe
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1
test\ScatterDraw_DemoMSBT19x64.exe	
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1

umk Anboto examples/OfficeAutomation_demo_cl CLANGx64 -r   test\OfficeAutomation_demo_consoleCLANGx64.exe
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1
umk Anboto examples/OfficeAutomation_demo_cl MSBT19x64 -r  test\OfficeAutomation_demo_consoleMSBT19x64.exe
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1

umk Anboto examples/ScatterCtrl_demo CLANGx64 -r   test\ScatterCtrl_DemoCLANGx64.exe
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1
umk Anboto examples/ScatterCtrl_demo MSBT19x64 -r   test\ScatterCtrl_DemoMSBT19x64.exe
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1


