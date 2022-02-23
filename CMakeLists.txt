cmake_minimum_required(VERSION 3.22.0)
project(donkeyleveldb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wstrict-prototypes")
set(LEVELDB_PUBLIC_INCLUDE_DIR "./include/leveldb")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
  "."
  "${PROJECT_SOURCE_DIR}/include"
)

add_library(donkeyleveldb "")
target_sources(donkeyleveldb 
  PRIVATE
  "db/log_writer.cpp"
  "util/env.cpp"
  PUBLIC
  "${LEVELDB_PUBLIC_INCLUDE_DIR}/env.h"
  "${LEVELDB_PUBLIC_INCLUDE_DIR}/slice.h"
  "${LEVELDB_PUBLIC_INCLUDE_DIR}/status.h"
)

# ctest
enable_testing()
add_subdirectory("third_party/googletest")
# 定义一个function方便添加测试文件
function(leveldb_test test_file) 
  get_filename_component(test_target_name "${test_file}" NAME_WE)
  add_executable("${test_target_name}" "")
  target_sources("${test_target_name}"
    PRIVATE
    "${test_file}"
  )
  target_link_libraries("${test_target_name}" donkeyleveldb gtest)
  add_test(NAME "${test_target_name}" COMMAND "${test_target_name}")
endfunction(leveldb_test)  

leveldb_test("db/log_writer_test.cpp")