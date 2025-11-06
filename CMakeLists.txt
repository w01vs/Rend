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
"src/main.cpp"
"src/lexer.cpp"
"src/tokenstream.cpp"
"src/type.cpp"
"src/ast_builder.cpp"
"src/parser.cpp"
"src/semantics.cpp"
"src/hir_generator.cpp"
)