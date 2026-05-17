cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

project(compiler)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug" )


add_executable(Rend
src/driver/main.cpp
src/frontend/lexer/lexer.cpp
src/frontend/parser/tokenstream.cpp
src/shared/type.cpp
src/shared/ast_builder.cpp
src/frontend/parser/parser.cpp
src/mid_end/analysis/semantics.cpp
src/mid_end/hir/hir_generator.cpp
src/mid_end/hir/hir_def.cpp
src/backend/lir/lir_generator.cpp
src/backend/lir/lir_def.cpp
src/backend/codegen/codegen.cpp
# src/backend/register_allocator/register_allocator.cpp
# src/mid_end/optimizer/constants.cpp
)

target_include_directories(Rend PUBLIC src)