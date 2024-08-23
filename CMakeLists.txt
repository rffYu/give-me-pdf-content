cmake_minimum_required(VERSION 3.18)
project( give_me_toc )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} src/main.cpp)

message(STATUS "Library path: ${CMAKE_SOURCE_DIR}/third_party/mupdf/build")

include_directories(${CMAKE_SOURCE_DIR}/third_party/mupdf/include)
link_directories(${CMAKE_SOURCE_DIR}/third_party/mupdf/build/shared-release)

target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/third_party/mupdf/build/shared-release/libmupdf.so)

