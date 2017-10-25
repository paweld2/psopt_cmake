cmake_minimum_required(VERSION 3.8)


PROJECT(dmatrix)

ENABLE_TESTING()

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/dmatrix.pc.in
		       ${CMAKE_CURRENT_BINARY_DIR}/dmatrix.pc @ONLY)

set(DMATRIX_HEADER_FILES include/dmatrixv.h include/f2c.h)
include_directories(include)

ADD_LIBRARY(dmatrix SHARED src/dmatrixv.cxx)

target_link_libraries(dmatrix ${LAPACK_LIBRARIES})
include_directories(${LAPACK_INCLUDE_DIRS})

target_link_libraries(dmatrix ${BLAS_LIBRARIES})
include_directories(${BLAS_INCLUDE_DIRS})

target_link_libraries(dmatrix ${CXSPARSE_LIBRARIES})
include_directories(${CXSPARSE_INCLUDE_DIRS})

target_link_libraries(dmatrix ${LUSOL_LIBRARIES})
include_directories(${LUSOL_INCLUDE_DIRS})

INSTALL(FILES ${DMATRIX_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dmatrix)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/dmatrix.pc DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")

add_subdirectory (examples)