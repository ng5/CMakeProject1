cmake_minimum_required (VERSION 3.12)
project ("CMakeProject1")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set a sensible default build type for single-config generators
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
 set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

# Compiler optimization flags
if(MSVC)
 message(STATUS "Enabling MSVC optimization flags: /O2 /Ob2 /DNDEBUG")
 add_compile_options(/O2 /Ob2 /Oi /Gy /DNDEBUG)
 set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /Ob2 /DNDEBUG")
 set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /DNDEBUG")
else()
 message(STATUS "Enabling GCC/Clang optimization flags: -O3 -march=native -DNDEBUG -funroll-loops")
 add_compile_options(-O3 -march=native -DNDEBUG -funroll-loops)
 set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -march=native -DNDEBUG")
 set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -DNDEBUG")
endif()

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
