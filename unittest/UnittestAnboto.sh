#!/bin/bash
set -x
set -e

cwd=$(pwd)

umk Anboto examples/Functions4U_demo_cl CLANG -r +SHARED  $cwd/.test/Functions4U_DemoCLANG
$cwd/.test/Functions4U_DemoCLANG	
umk Anboto examples/Functions4U_demo_cl GCC -r +SHARED  $cwd/.test/Functions4U_DemoGCC	
$cwd/.test/Functions4U_DemoGCC	

umk Anboto examples/SysInfo_demo_cl CLANG  -r  +SHARED $cwd/.test/SysInfo_demo_consoleCLANG
$cwd/.test/SysInfo_demo_consoleCLANG	
umk Anboto examples/SysInfo_demo_cl GCC -r  +SHARED $cwd/.test/SysInfo_demo_consoleGCC	
$cwd/.test/SysInfo_demo_consoleGCC	

umk Anboto examples/STEM4U_demo_test_cl CLANG  -r  +SHARED $cwd/.test/STEM4U_DemoCLANGx64
$cwd/.test/STEM4U_DemoCLANGx64 -test	
umk Anboto examples/STEM4U_demo_test_cl GCC -r +SHARED  $cwd/.test/STEM4U_DemoMSBT19x64	
$cwd/.test/STEM4U_DemoMSBT19x64 -test

umk Anboto examples/Eigen_demo_cl CLANG  -r  +SHARED $cwd/.test/Eigen_DemoCLANGx64
$cwd/.test/Eigen_DemoCLANGx64	
umk Anboto examples/Eigen_demo_cl GCC -r  +SHARED $cwd/.test/Eigen_DemoMSBT19x64	
$cwd/.test/Eigen_DemoMSBT19x64	

umk Anboto examples/ScatterDraw_demo_cl CLANG  -r +SHARED  $cwd/.test/ScatterDraw_DemoCLANGx64
$cwd/.test/ScatterDraw_DemoCLANGx64	
umk Anboto examples/ScatterDraw_demo_cl GCC -r +SHARED  $cwd/.test/ScatterDraw_DemoMSBT19x64
$cwd/.test/ScatterDraw_DemoMSBT19x64	

umk Anboto examples/OfficeAutomation_demo_cl CLANG  -r  +SHARED $cwd/.test/OfficeAutomation_demo_consoleCLANGx64
umk Anboto examples/OfficeAutomation_demo_cl GCC -r +SHARED $cwd/.test/OfficeAutomation_demo_consoleMSBT19x64

umk Anboto examples/ScatterCtrl_demo CLANG  -r  +GUI,SHARED $cwd/.test/ScatterCtrl_DemoCLANGx64
umk Anboto examples/ScatterCtrl_demo GCC -r  +GUI,SHARED $cwd/.test/ScatterCtrl_DemoMSBT19x64

echo -----------All done. NO error!