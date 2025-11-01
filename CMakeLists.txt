cmake_minimum_required (VERSION 3.8)
project ("CMakeProject1")

# Include sub-projects.
file(GLOB files "src/*.cpp" "src/*.c")
foreach(file ${files})
    get_filename_component(x ${file} NAME_WLE)
    add_executable("${x}" ${file})
endforeach()