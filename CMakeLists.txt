cmake_minimum_required(VERSION 3.24)
project(fuse-ext2)


set(CMAKE_C_STANDARD 11)
aux_source_directory(${PROJECT_SOURCE_DIR}/src  SRC)
include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} ${SRC})
