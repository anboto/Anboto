
md .test
del .test\*.* /q

umk Anboto examples/Functions4U_demo_cl %1 -r   .\.test\Functions4U_Demo.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Problem compiling Functions4U_demo_cl"
.\.test\Functions4U_Demo.exe	
@IF %ERRORLEVEL% NEQ 0 PAUSE "Problem testing Functions4U_demo_cl"

umk Anboto examples/SysInfo_demo_cl %1 -r   .\.test\SysInfo_demo_console.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Problem compiling SysInfo_demo_cl"
.\.test\SysInfo_demo_console.exe	
@IF %ERRORLEVEL% NEQ 0 PAUSE "Problem testing SysInfo_demo_cl"

umk Anboto examples/STEM4U_demo_test_cl %1 -r   .\.test\STEM4U_Demo.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Problem compiling STEM4U_demo_test_cl"
.\.test\STEM4U_Demo.exe -test	
@IF %ERRORLEVEL% NEQ 0 PAUSE "Problem testing STEM4U_demo_test_cl"

umk Anboto examples/Eigen_demo_cl %1 -r   .\.test\Eigen_Demo.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Problem compiling Eigen_demo_cl"
.\.test\Eigen_Demo.exe	
@IF %ERRORLEVEL% NEQ 0 PAUSE "Problem testing Eigen_demo_cl"

umk Anboto examples/ScatterDraw_demo_cl %1 -r   .\.test\ScatterDraw_Demo.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Problem compiling ScatterDraw_demo_cl"
.\.test\ScatterDraw_Demo.exe	
@IF %ERRORLEVEL% NEQ 0 PAUSE "Problem testing ScatterDraw_demo_cl"

umk Anboto examples/OfficeAutomation_demo_cl %1 -r   .\.test\OfficeAutomation_demo_console.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Problem compiling OfficeAutomation_demo_cl"

umk Anboto examples/ScatterCtrl_demo %1 -r +GUI  .\.test\ScatterCtrl_Demo.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Problem compiling ScatterCtrl_demo"

umk Anboto examples/Surface_demo_test_cl %1 -r   .\.test\Surface_demo_test_cl.exe
@IF %ERRORLEVEL% NEQ 0 PAUSE "Problem compiling Surface_demo_test_cl"

umk Anboto examples/SysInfo_demo_cl %1 -r   .\.test\SysInfo_demo_cl.exe
@IF %ERRORLEVEL% NEQ 0  PAUSE "Problem compiling SysInfo_demo_cl"

umk Anboto examples/SysInfo_demo_gui %1 -r +GUI  .\.test\SysInfo_demo_gui.exe
@IF %ERRORLEVEL% NEQ 0  PAUSE "Problem compiling SysInfo_demo_gui"

umk Anboto examples/Controls4U_demo %1 -r +GUI  .\.test\Controls4U_demo.exe
@IF %ERRORLEVEL% NEQ 0  PAUSE "Problem compiling Controls4U_demo"

umk Anboto examples/GLCanvas_demo %1 -r +GUI  .\.test\GLCanvas_demo.exe
@IF %ERRORLEVEL% NEQ 0  PAUSE "Problem compiling GLCanvas_demo"

umk Anboto examples/MatIO_demo_cl %1 -r   .\.test\MatIO_demo_cl.exe
@IF %ERRORLEVEL% NEQ 0  PAUSE "Problem compiling MatIO_demo_cl"
cd .\.test
MatIO_demo_cl.exe -test	
@IF %ERRORLEVEL% NEQ 0  PAUSE "Problem testing MatIO_demo_cl"
cd ..
@IF %ERRORLEVEL% NEQ 0 PAUSE "Problem"

umk Anboto examples/Hdf5_demo_cl %1 -r   .\.test\Hdf5_demo_cl.exe
@IF %ERRORLEVEL% NEQ 0  PAUSE "Problem compiling Hdf5_demo_cl"
cd .\.test
Hdf5_demo_cl.exe -test	
cd ..
@IF %ERRORLEVEL% NEQ 0 PAUSE "Problem"

umk Anboto examples/NetCDF_demo_cl %1 -r   .\.test\NetCDF_demo_cl.exe
@IF %ERRORLEVEL% NEQ 0  PAUSE "Problem compiling NetCDF_demo_cl"
cd .\.test
NetCDF_demo_cl.exe -test	
@IF %ERRORLEVEL% NEQ 0  PAUSE "Problem testing NetCDF_demo_cl"
cd ..
@IF %ERRORLEVEL% NEQ 0 PAUSE "Problem "

umk Anboto examples/Npy_demo_cl %1 -r   .\.test\Npy_demo_cl.exe
@IF %ERRORLEVEL% NEQ 0  PAUSE "Problem compiling Npy_demo_cl"
copy ..\..\Npy\examples\Npy_demo_cl\matrices.npz .\.test
cd .\.test
Npy_demo_cl.exe -test	
@IF %ERRORLEVEL% NEQ 0  PAUSE "Problem testing Npy_demo_cl"
cd ..
@IF %ERRORLEVEL% NEQ 0 PAUSE "Problem "