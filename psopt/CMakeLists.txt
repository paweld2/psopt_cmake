cmake_minimum_required(VERSION 3.8)


PROJECT(psopt)

ENABLE_TESTING()

include_directories(src)

FILE(GLOB PSOPT_SOURCE_FILES src/*.cxx)
FILE(GLOB PSOPT_HEADER_FILES src/*.h)

ADD_LIBRARY(psopt SHARED ${PSOPT_SOURCE_FILES} ${PSOPT_HEADER_FILES})

target_link_libraries(psopt dmatrix)
include_directories(../dmatrix/include)

target_link_libraries(psopt ${ADOLC_LIBRARIES} -lColPack)
include_directories(${ADOLC_INCLUDEDIR} /usr/include/ColPack)

target_link_libraries(psopt ${IPOPT_LIBRARIES})
include_directories(${IPOPT_INCLUDEDIR})

target_link_libraries(psopt ${LUSOL_LIBRARIES})
include_directories(${LUSOL_INCLUDEDIR})

add_subdirectory (examples)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/psopt.pc.in
		       ${CMAKE_CURRENT_BINARY_DIR}/psopt.pc @ONLY)


INSTALL(FILES ${PSOPT_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/psopt)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/psopt.pc DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")