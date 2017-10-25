
cmake_minimum_required(VERSION 3.8)

project (psopt  C CXX)
set(PSOPT_SUMMARY               "This is the PSOPT library, a software tool for computational optimal control.")
set(PS_OPT_VENDOR_NAME          "Victor M. Becerra")
set(PS_OPT_VENDOR_CONTACT       "vmbecerra99@gmail.com")
set(PS_OPT_VERSION_MAJOR        4)
set(PS_OPT_VERSION_MINOR        0)
set(PS_OPT_VERSION_PATCH        0)
set(PROJECT_VERSION  "${PS_OPT_VERSION_MAJOR}.${PS_OPT_VERSION_MINOR}.${PS_OPT_VERSION_PATCH}")

set (CMAKE_CXX_STANDARD 11)

INCLUDE(FindPkgConfig)
include(GNUInstallDirs)
find_package(BLAS)
find_package(LAPACK)
if(LAPACK_FOUND AND BLAS_FOUND)
   set(lapackblas_libraries ${BLAS_LIBRARIES} ${LAPACK_LIBRARIES})
   add_definitions(-DLAPACK)
endif()

find_package(PkgConfig REQUIRED)

pkg_check_modules (CXSPARSE cxsparse)
pkg_check_modules (LUSOL lusol)
pkg_check_modules (ADOLC adolc REQUIRED)
pkg_check_modules (IPOPT ipopt REQUIRED)

if(CXSPARSE_FOUND)
   add_definitions(-DSPARSE_MATRIX)
endif()

add_definitions(-DUSE_IPOPT)
add_subdirectory (dmatrix)
add_subdirectory (psopt)