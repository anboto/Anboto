@echo "Testing Anboto for CLANGX64"

@title "Testing Anboto for Linux
@start call bash UnittestAnboto.sh
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing and deploying Anboto Linux"

@title "Testing Anboto for CLANGX64
call UnittestAnboto.bat CLANGX64
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing Anboto CLANG"

set "prefix=CPU-C"
set "name=%COMPUTERNAME%"
set "first5=%name:~0,5%"
if /I not "%first5%"=="%prefix%" (
	echo "Testing Anboto for MSVS22x64"
	call UnittestAnboto.bat MSVS22x64
	IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing Anboto MSVS17x64"
)

