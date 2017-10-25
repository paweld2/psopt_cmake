

include_directories(${ADOLC_INCLUDEDIR} /usr/include/ColPack)

file(GLOB examples "*/*.cxx")
foreach(example ${examples})
    get_filename_component(CXX_FILENAME ${example} NAME_WE)
    get_filename_component(CXX_DIR ${example} DIRECTORY)

    ADD_EXECUTABLE(${CXX_FILENAME} ${example})
    target_link_libraries(${CXX_FILENAME} psopt)
endforeach()