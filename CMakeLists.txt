cmake_minimum_required (VERSION 3.12)
project ("CMakeProject1")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Set a sensible default build type for single-config generators
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
 set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

# Compiler optimization flags
if(MSVC)
 message(STATUS "Enabling MSVC optimization flags: /O2 /Ob2 /DNDEBUG")
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
file(GLOB headers "${CMAKE_SOURCE_DIR}/include/*.h" "${CMAKE_SOURCE_DIR}/include/*.hpp")
file(GLOB sources "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/src/*.c")
list(LENGTH sources src_count)
if(src_count GREATER 0)
  add_library(library STATIC ${sources})
  target_include_directories(library PUBLIC ${CMAKE_SOURCE_DIR}/include)
else()
  add_library(library INTERFACE)
  target_include_directories(library INTERFACE ${CMAKE_SOURCE_DIR}/include)
endif()
set_target_properties(library PROPERTIES LINKER_LANGUAGE CXX)

# Benchmarks
file(GLOB benchmarks "benchmarks/*.cpp")
list(LENGTH benchmarks benchmark_count)
if(benchmark_count GREATER 0)
	add_executable(bench ${benchmarks})
	target_link_libraries(bench PRIVATE library benchmark benchmark_main)
endif()

# tests
file(GLOB tests "tests/*.cpp")
foreach(testfile ${tests})
 get_filename_component(testname ${testfile} NAME_WE)
 add_executable(${testname} ${testfile})
 target_link_libraries(${testname} PRIVATE library)
endforeach()
