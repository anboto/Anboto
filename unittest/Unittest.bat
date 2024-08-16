@echo "Testing Anboto for CLANGX64"

@title "Testing BEMRosetta for Linux
@start call bash UnittestAnboto.sh
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing and deploying Anboto Linux"

@title "Testing BEMRosetta for CLANGX64
call UnittestAnboto.bat CLANGX64
@IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing Anboto CLANG"
@echo "Testing Anboto for MSVS22x64"
@echo call UnittestAnboto.bat MSVS22x64
@echo IF %ERRORLEVEL% NEQ 0 PAUSE "Error testing Anboto MSVS17x64"

