cmake_minimum_required(VERSION 3.14)
project(SSSP VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for better IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Compiler-specific flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0 -fsanitize=address -fsanitize=undefined)
        add_link_options(-fsanitize=address -fsanitize=undefined)
    else()
        add_compile_options(-O3 -march=native)
    endif()
elseif(MSVC)
    add_compile_options(/W4)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(/Od /Zi)
    else()
        add_compile_options(/O2)
    endif()
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/include/*.hpp ${PROJECT_SOURCE_DIR}/include/*.h)

# Create library
add_library(sssp_lib STATIC ${SOURCES})
target_include_directories(sssp_lib PUBLIC 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Optional: Build example/demo executable
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES AND EXISTS ${PROJECT_SOURCE_DIR}/src/main.cpp)
    add_executable(sssp_demo ${PROJECT_SOURCE_DIR}/src/main.cpp)
    target_link_libraries(sssp_demo PRIVATE sssp_lib)
endif()

# Profiling and benchmarks
option(SSSP_PROFILE "Enable internal profiling" OFF)
if(SSSP_PROFILE)
    add_compile_definitions(SSSP_PROFILE)
endif()

option(BUILD_BENCHMARKS "Build benchmark programs" ON)
if(BUILD_BENCHMARKS AND EXISTS ${PROJECT_SOURCE_DIR}/benchmarks/bench_sssp.cpp)
    add_executable(bench_sssp ${PROJECT_SOURCE_DIR}/benchmarks/bench_sssp.cpp)
    target_link_libraries(bench_sssp PRIVATE sssp_lib)
endif()

# Testing
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    # Do not auto-aggregate all tests; add individually to avoid multiple mains
    # file(GLOB_RECURSE TEST_SOURCES ${PROJECT_SOURCE_DIR}/tests/*.cpp)
    # if(TEST_SOURCES)
    #     add_executable(sssp_tests ${TEST_SOURCES})
    #     target_link_libraries(sssp_tests PRIVATE sssp_lib)
    #     add_test(NAME sssp_tests COMMAND sssp_tests)
    # endif()

    if(EXISTS ${PROJECT_SOURCE_DIR}/tests/path_smoke.cpp)
        add_executable(test_paths ${PROJECT_SOURCE_DIR}/tests/path_smoke.cpp)
        target_link_libraries(test_paths PRIVATE sssp_lib)
        add_test(NAME test_paths COMMAND test_paths)
    endif()

    if(EXISTS ${PROJECT_SOURCE_DIR}/tests/distance_query_smoke.cpp)
        add_executable(test_distance_queries ${PROJECT_SOURCE_DIR}/tests/distance_query_smoke.cpp)
        target_link_libraries(test_distance_queries PRIVATE sssp_lib)
        add_test(NAME test_distance_queries COMMAND test_distance_queries)
    endif()

    if(EXISTS ${PROJECT_SOURCE_DIR}/tests/tie_break_smoke.cpp)
        add_executable(test_tie_break ${PROJECT_SOURCE_DIR}/tests/tie_break_smoke.cpp)
        target_link_libraries(test_tie_break PRIVATE sssp_lib)
        add_test(NAME test_tie_break COMMAND test_tie_break)
    endif()


    if(EXISTS ${PROJECT_SOURCE_DIR}/test_binary_heap.cpp)
        add_executable(test_binary_heap ${PROJECT_SOURCE_DIR}/test_binary_heap.cpp)
        target_link_libraries(test_binary_heap PRIVATE sssp_lib)
        add_test(NAME test_binary_heap COMMAND test_binary_heap)
    endif()

    if(EXISTS ${PROJECT_SOURCE_DIR}/test_block_structure.cpp)
        add_executable(test_block_structure ${PROJECT_SOURCE_DIR}/test_block_structure.cpp)
        target_link_libraries(test_block_structure PRIVATE sssp_lib)
        add_test(NAME test_block_structure COMMAND test_block_structure)
    endif()

    if(EXISTS ${PROJECT_SOURCE_DIR}/test_graph.cpp)
        add_executable(test_graph ${PROJECT_SOURCE_DIR}/test_graph.cpp)
        target_link_libraries(test_graph PRIVATE sssp_lib)
        add_test(NAME test_graph COMMAND test_graph)
    endif()

    if(EXISTS ${PROJECT_SOURCE_DIR}/test_transform.cpp)
        add_executable(test_transform ${PROJECT_SOURCE_DIR}/test_transform.cpp)
        target_link_libraries(test_transform PRIVATE sssp_lib)
        add_test(NAME test_transform COMMAND test_transform)
    endif()

    if(EXISTS ${PROJECT_SOURCE_DIR}/test_find_pivots.cpp)
        add_executable(test_find_pivots ${PROJECT_SOURCE_DIR}/test_find_pivots.cpp)
        target_link_libraries(test_find_pivots PRIVATE sssp_lib)
        add_test(NAME test_find_pivots COMMAND test_find_pivots)
    endif()

    if(EXISTS ${PROJECT_SOURCE_DIR}/test_validate_transform.cpp)
        add_executable(test_validate_transform ${PROJECT_SOURCE_DIR}/test_validate_transform.cpp)
        target_link_libraries(test_validate_transform PRIVATE sssp_lib)
        add_test(NAME test_validate_transform COMMAND test_validate_transform)
    endif()

    if(EXISTS ${PROJECT_SOURCE_DIR}/test_complexity.cpp)
        add_executable(test_complexity ${PROJECT_SOURCE_DIR}/test_complexity.cpp)
        target_link_libraries(test_complexity PRIVATE sssp_lib)
        add_test(NAME test_complexity COMMAND test_complexity)
    endif()

    add_executable(test_base_case ${PROJECT_SOURCE_DIR}/test_base_case.cpp)
    target_link_libraries(test_base_case PRIVATE sssp_lib)
    add_test(NAME test_base_case COMMAND test_base_case)

    add_executable(test_bmssp ${PROJECT_SOURCE_DIR}/test_bmssp.cpp)
    target_link_libraries(test_bmssp PRIVATE sssp_lib)
    add_test(NAME test_bmssp COMMAND test_bmssp)
endif()

# Installation rules
install(TARGETS sssp_lib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

# Package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/SSSPConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Print configuration summary
message(STATUS "")
message(STATUS "SSSP Library Configuration:")
message(STATUS "  Version:     ${PROJECT_VERSION}")
message(STATUS "  Build type:  ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler:    ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
message(STATUS "  Build examples: ${BUILD_EXAMPLES}")
message(STATUS "")