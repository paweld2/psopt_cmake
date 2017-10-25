
file(GLOB d_examples "*.cxx")
foreach(d_example ${d_examples})
    get_filename_component(D_FILENAME ${d_example} NAME_WE)

    ADD_EXECUTABLE(${D_FILENAME} ${d_example})
    target_link_libraries(${D_FILENAME} dmatrix)
endforeach()