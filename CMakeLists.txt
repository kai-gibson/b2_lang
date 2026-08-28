cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_OSX_DEPLOYMENT_TARGET "14.7" CACHE STRING "Minimum OS X version" FORCE)

project(LearningLLVM VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_COVERAGE "Enable LLVM coverage" OFF)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

separate_arguments(LLVM_DEFINITIONS_LIST UNIX_COMMAND "${LLVM_DEFINITIONS}")

llvm_map_components_to_libnames(llvm_libs
    core executionengine mcjit native x86codegen
)
list(REMOVE_DUPLICATES llvm_libs)

add_library(project_settings INTERFACE)

target_include_directories(project_settings INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(project_settings SYSTEM INTERFACE
    ${LLVM_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/external/CLI11/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/multiprecision/include
)

target_compile_definitions(project_settings INTERFACE
    ${LLVM_DEFINITIONS_LIST}
    _LIBCPP_ENABLE_CXX17_REMOVED_FEATURES
)

target_compile_options(project_settings INTERFACE
    -Wall -Wextra -Wpedantic -Werror=switch
    -Wno-deprecated-declarations -Wno-unused-parameter
)

target_link_options(project_settings INTERFACE
)

if(ENABLE_COVERAGE)
    target_compile_options(project_settings INTERFACE
        -fprofile-instr-generate -fcoverage-mapping
    )
    target_link_options(project_settings INTERFACE
        -fprofile-instr-generate -fcoverage-mapping

    )
endif()

add_library(b2_lib OBJECT
    src/frontend.cxx
    src/lexer.cxx
    src/file_io.cxx
    src/parser.cxx
    src/ast.cxx
    src/type_check_visitor.cxx
    src/codegen/code_generation_visitor.cxx
    src/codegen/operation_map.cxx
)

target_link_libraries(b2_lib
    PUBLIC ${llvm_libs}
    PRIVATE project_settings
)

add_executable(b2 src/main.cxx)

target_link_libraries(b2 PRIVATE
    b2_lib
    project_settings
)

enable_testing()
add_subdirectory(tests)
