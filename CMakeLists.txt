cmake_minimum_required(VERSION 3.24)
project(fuse-ext2)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS  OFF)


#  增加一个子目录
add_subdirectory(${PROJECT_SOURCE_DIR}/src )

#  用以测试
add_subdirectory(${PROJECT_SOURCE_DIR}/test)



