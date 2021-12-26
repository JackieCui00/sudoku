cmake_minimum_required(VERSION 3.20)

project(Sudoku VERSION 0.1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PROJ_CXX_FLAGS -g -Wall -Wextra -Werror -Wconversion -Wcast-align -Wunused -Wshadow -Wold-style-cast -Wpointer-arith -Wcast-qual -Wmissing-braces -pedantic)
set(PROJ_LD_FLAGS "")

set(PROJ_DEBUG_CXX_FLAGS -O0 -fsanitize=address -fsanitize=undefined -fomit-frame-pointer --coverage)
set(PROJ_DEBUG_LD_FLAGS -fsanitize=address -fsanitize=undefined --coverage)

set(main_file "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
list(FILTER sources EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/src/main\.cpp")

add_executable("sudoku" ${main_file} ${sources})
target_compile_options("sudoku" PRIVATE ${PROJ_CXX_FLAGS})
target_link_options("sudoku" PRIVATE ${PROJ_LD_FLAGS})
message(STATUS "add executable sudoku, sources: ${main_file} ${sources}")

file(GLOB test_sources CONFIGURE_DEPENDS "test/*.cpp")
add_executable("unittest" ${sources} ${test_sources})
target_compile_options("unittest" PRIVATE ${PROJ_DEBUG_CXX_FLAGS})
target_link_options("unittest" PRIVATE ${PROJ_DEBUG_LD_FLAGS})
target_include_directories("unittest" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
message(STATUS "add executable unittest, sources: ${sources} ${test_sources}")
