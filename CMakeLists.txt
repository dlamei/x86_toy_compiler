cmake_minimum_required(VERSION 3.10)

set(PROJ Compiler)

project(${PROJ})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

set(SRC ${SRC}
	src/main.cpp
	src/Compiler.cpp
	src/Error.cpp
	src/Parser.cpp
	src/TypeChecker.cpp
	src/lexer.cpp
)

add_executable(${PROJ} ${SRC})
