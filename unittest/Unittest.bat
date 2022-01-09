@echo "Testing Anboto for CLANGX64"
call UnittestAnboto.bat CLANGX64
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing Anboto CLANG"
@echo "Testing Anboto for MSBT19x64"
call UnittestAnboto.bat MSBT19x64
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing Anboto MSVC"

