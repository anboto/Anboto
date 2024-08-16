#!/bin/bash
set -x
set -e
export PATH=$PATH:/home/aupa/upp

cwd=$(pwd)

umk Anboto examples/Functions4U_demo_cl CLANG    -r +SHARED  $cwd/.test/Functions4U_DemoCLANG
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
$cwd/.test/Functions4U_DemoCLANG	
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
umk Anboto examples/Functions4U_demo_cl CLANG_17 -r +SHARED  $cwd/.test/Functions4U_DemoCLANG_17	
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
$cwd/.test/Functions4U_DemoCLANG_17	
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }

umk Anboto examples/SysInfo_demo_cl CLANG    -r  +SHARED $cwd/.test/SysInfo_demo_consoleCLANG
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
$cwd/.test/SysInfo_demo_consoleCLANG	
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
umk Anboto examples/SysInfo_demo_cl CLANG_17 -r  +SHARED $cwd/.test/SysInfo_demo_consoleCLANG_17	
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
$cwd/.test/SysInfo_demo_consoleCLANG_17	
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }

umk Anboto examples/STEM4U_demo_test_cl CLANG    -r  +SHARED $cwd/.test/STEM4U_DemoCLANG
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
$cwd/.test/STEM4U_DemoCLANG -test	
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
umk Anboto examples/STEM4U_demo_test_cl CLANG_17 -r +SHARED  $cwd/.test/STEM4U_DemoCLANG_17
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
$cwd/.test/STEM4U_DemoCLANG_17 -test
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }

umk Anboto examples/Eigen_demo_cl CLANG    -r  +SHARED $cwd/.test/Eigen_DemoCLANG
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
$cwd/.test/Eigen_DemoCLANG	
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
umk Anboto examples/Eigen_demo_cl CLANG_17 -r  +SHARED $cwd/.test/Eigen_DemoCLANG_17
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
$cwd/.test/Eigen_DemoCLANG_17	
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }

umk Anboto examples/ScatterDraw_demo_cl CLANG  -r +SHARED  $cwd/.test/ScatterDraw_DemoCLANG
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
$cwd/.test/ScatterDraw_DemoCLANG	
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
umk Anboto examples/ScatterDraw_demo_cl CLANG_17 -r +SHARED  $cwd/.test/ScatterDraw_DemoCLANG_17
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
$cwd/.test/ScatterDraw_DemoCLANG_17	
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }

umk Anboto examples/Hdf5_demo_cl CLANG -r +SHARED  $cwd/.test/Hdf5_DemoCLANG
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
$cwd/.test/Hdf5_DemoCLANG
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
umk Anboto examples/Hdf5_demo_cl CLANG_17 -r +SHARED  $cwd/.test/Hdf5_DemoCLANG_17
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
$cwd/.test/Hdf5_DemoCLANG_17
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }

umk Anboto examples/NetCDF_demo_cl CLANG -r +SHARED  $cwd/.test/NetCDF_DemoCLANG
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
$cwd/.test/NetCDF_DemoCLANG
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
umk Anboto examples/NetCDF_demo_cl CLANG_17 -r +SHARED  $cwd/.test/NetCDF_DemoCLANG_17
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
$cwd/.test/NetCDF_DemoCLANG_17
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }

umk Anboto examples/ScatterCtrl_demo CLANG  -r  +GUI,SHARED $cwd/.test/ScatterCtrl_DemoCLANG
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }
umk Anboto examples/ScatterCtrl_demo CLANG_17 -r  +GUI,SHARED $cwd/.test/ScatterCtrl_DemoCLANG_17
[ $? -ne 0 ] && { echo "Problem found"; read -p "Press Enter to continue..."; }

exit
