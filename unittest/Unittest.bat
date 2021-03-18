call UnittestAnboto.bat CLANGX64
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing Anboto CLANG"
call UnittestAnboto.bat MSBT19x64
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing Anboto MSVC"