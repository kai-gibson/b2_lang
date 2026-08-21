include(FetchContent)

# Prevent GoogleTest from overriding our compiler/linker options if using MSVC
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(run_tests
  lexer_test.cxx
  utils_test.cxx
  parser_test.cxx
  type_check_test.cxx
  node.cxx
  #codegen_test.cxx
)

target_include_directories(run_tests PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/.."
  ../include/
  include/codegen/
)

# test executable links against .o from parent
target_link_libraries(run_tests PRIVATE
  learning_llvm_lib
  project_settings
  gtest_main
  gmock_main
)

# Force-keep LLVM's static initializers (e.g. MCJIT/target registration),
# which Apple's linker otherwise dead-strips since nothing calls into
# these archives directly. Scoped to just these libs (not -all_load) to
# avoid pulling in duplicate main() symbols from gtest_main/gmock_main.
find_library(LLVM_MCJIT_LIB LLVMMCJIT PATHS ${LLVM_LIBRARY_DIRS})
find_library(LLVM_EXECUTIONENGINE_LIB LLVMExecutionEngine PATHS ${LLVM_LIBRARY_DIRS})
find_library(LLVM_X86CODEGEN_LIB LLVMX86CodeGen PATHS ${LLVM_LIBRARY_DIRS})

target_link_options(run_tests PRIVATE
  -Wl,-force_load,${LLVM_MCJIT_LIB}
  -Wl,-force_load,${LLVM_EXECUTIONENGINE_LIB}
  -Wl,-force_load,${LLVM_X86CODEGEN_LIB}
)

include(GoogleTest)
gtest_discover_tests(run_tests)
