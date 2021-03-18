
md .test
del .test\*.* /q

umk Anboto examples/Functions4U_demo_cl %1 -r   .\.test\Functions4U_Demo.exe
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1
.\.test\Functions4U_Demo.exe	
@IF %ERRORLEVEL% NEQ 0  EXIT /B 1

umk Anboto examples/SysInfo_demo_cl %1 -r   .\.test\SysInfo_demo_console.exe
@IF %ERRORLEVEL% NEQ 0  EXIT /B 1
.\.test\SysInfo_demo_console.exe	
@IF %ERRORLEVEL% NEQ 0  EXIT /B 1

umk Anboto examples/STEM4U_demo_test_cl %1 -r   .\.test\STEM4U_Demo.exe
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1
.\.test\STEM4U_Demo.exe -test	
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1

umk Anboto examples/Eigen_demo_cl %1 -r   .\.test\Eigen_Demo.exe
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1
.\.test\Eigen_Demo.exe	
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1

umk Anboto examples/ScatterDraw_demo_cl %1 -r   .\.test\ScatterDraw_Demo.exe
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1
.\.test\ScatterDraw_Demo.exe	
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1

umk Anboto examples/OfficeAutomation_demo_cl %1 -r   .\.test\OfficeAutomation_demo_console.exe
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1

umk Anboto examples/ScatterCtrl_demo %1 -r   .\.test\ScatterCtrl_Demo.exe
@IF %ERRORLEVEL% NEQ 0 EXIT /B 1



