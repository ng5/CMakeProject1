cmake_minimum_required (VERSION 3.12)
project ("CMakeProject1")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Set a sensible default build type for single-config generators
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

# Prefer target-scoped options; minimal globals
# Explicit MSVC runtime (DLL)
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

# Detect and use active Conda environment include/lib paths (optional)
if(DEFINED ENV{CONDA_PREFIX} AND NOT "$ENV{CONDA_PREFIX}" STREQUAL "")
  set(CONDA_PREFIX "$ENV{CONDA_PREFIX}")
  message(STATUS "Using Conda environment at: ${CONDA_PREFIX}")
  set(CONDA_INC_DIRS "${CONDA_PREFIX}/Library/include" "${CONDA_PREFIX}/include")
  set(CONDA_LIB_DIRS "${CONDA_PREFIX}/Library/lib" "${CONDA_PREFIX}/lib")
else()
  message(STATUS "CONDA_PREFIX not set; proceeding without Conda include/link paths")
endif()

# Library under test
file(GLOB headers "${CMAKE_SOURCE_DIR}/include/*.h" "${CMAKE_SOURCE_DIR}/include/*.hpp")
file(GLOB sources "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/src/*.c")
list(LENGTH sources src_count)
if(src_count GREATER 0)
  add_library(library STATIC ${sources})
  target_include_directories(library PUBLIC ${CMAKE_SOURCE_DIR}/include ${CONDA_INC_DIRS})
  set_target_properties(library PROPERTIES LINKER_LANGUAGE CXX)
  if(MSVC)
    target_compile_options(library PRIVATE
      $<$<CONFIG:Release>:/O2 /Ob2 /DNDEBUG /GL /Gw /Gy /MP /EHsc /Zc:__cplusplus /Zc:inline /permissive->
      $<$<CONFIG:Debug>:/Od /Zi /RTC1 /EHsc /Zc:__cplusplus /permissive->)
    target_link_options(library PRIVATE
      $<$<CONFIG:Release>:/LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO>
      $<$<CONFIG:Debug>:/INCREMENTAL>)
  else()
    target_compile_options(library PRIVATE
      $<$<CONFIG:Release>:-O3 -march=native -DNDEBUG -funroll-loops>
      $<$<CONFIG:Debug>:-O0 -g>)
  endif()
else()
  add_library(library INTERFACE)
  target_include_directories(library INTERFACE ${CMAKE_SOURCE_DIR}/include ${CONDA_INC_DIRS})
endif()

# Link Conda libraries if any
if(CONDA_LIB_DIRS)
  link_directories(${CONDA_LIB_DIRS})
endif()

# Benchmarks
file(GLOB benchmarks "benchmarks/*.cpp")
list(LENGTH benchmarks benchmark_count)
if(benchmark_count GREATER 0)
  add_executable(bench ${benchmarks})
  target_link_libraries(bench PRIVATE library benchmark benchmark_main)
  target_include_directories(bench PRIVATE ${CMAKE_SOURCE_DIR}/include ${CONDA_INC_DIRS})
  if(MSVC)
    target_compile_options(bench PRIVATE
      $<$<CONFIG:Release>:/O2 /Ob2 /DNDEBUG /GL /Gw /Gy /MP /EHsc /Zc:__cplusplus /Zc:inline /permissive->
      $<$<CONFIG:Debug>:/Od /Zi /RTC1 /EHsc /Zc:__cplusplus /permissive->)
    target_link_options(bench PRIVATE
      $<$<CONFIG:Release>:/LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO>
      $<$<CONFIG:Debug>:/INCREMENTAL>)
  else()
    target_compile_options(bench PRIVATE
      $<$<CONFIG:Release>:-O3 -march=native -DNDEBUG -funroll-loops>
      $<$<CONFIG:Debug>:-O0 -g>)
  endif()
endif()

# tests
file(GLOB tests "tests/*.cpp")
foreach(testfile ${tests})
  get_filename_component(testname ${testfile} NAME_WE)
  add_executable(${testname} ${testfile})
  target_link_libraries(${testname} PRIVATE library)
  target_include_directories(${testname} PRIVATE ${CMAKE_SOURCE_DIR}/include ${CONDA_INC_DIRS})
  if(MSVC)
    target_compile_options(${testname} PRIVATE
      $<$<CONFIG:Release>:/O2 /Ob2 /DNDEBUG /GL /Gw /Gy /MP /EHsc /Zc:__cplusplus /Zc:inline /permissive->
      $<$<CONFIG:Debug>:/Od /Zi /RTC1 /EHsc /Zc:__cplusplus /permissive->)
    target_link_options(${testname} PRIVATE
      $<$<CONFIG:Release>:/LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO>
      $<$<CONFIG:Debug>:/INCREMENTAL>)
  else()
    target_compile_options(${testname} PRIVATE
      $<$<CONFIG:Release>:-O3 -march=native -DNDEBUG -funroll-loops>
      $<$<CONFIG:Debug>:-O0 -g>)
  endif()
endforeach()
