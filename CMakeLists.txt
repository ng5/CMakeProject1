cmake_minimum_required (VERSION 3.8)
project ("CMakeProject1")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Detect and use active Conda environment include/lib paths
include_directories(${CMAKE_SOURCE_DIR}/include)
if(DEFINED ENV{CONDA_PREFIX})
    set(CONDA_PREFIX "$ENV{CONDA_PREFIX}")
    include_directories(BEFORE SYSTEM "${CONDA_PREFIX}/Library/include" "${CONDA_PREFIX}/include")
    link_directories("${CONDA_PREFIX}/Library/lib" "${CONDA_PREFIX}/lib")
    message(STATUS "Using Conda environment at: ${CONDA_PREFIX}")
else()
     message(FATAL_ERROR "Missing CONDA_PREFIX" )
endif()

# Library under test
file(GLOB files "src/*.cpp" "src/*.c")
add_library(library STATIC ${files})

# Benchmarks
file(GLOB benchmarks "benchmarks/*.cpp")
add_executable(bench ${benchmarks})
target_link_libraries(bench PRIVATE library benchmark benchmark_main)

# tests
file(GLOB tests "tests/*.cpp")
foreach(testfile ${tests})
    get_filename_component(testname ${testfile} NAME_WE)
    add_executable(${testname} ${testfile})
    target_link_libraries(${testname} PRIVATE library)
endforeach()
