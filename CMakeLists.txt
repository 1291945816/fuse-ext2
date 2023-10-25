cmake_minimum_required(VERSION 3.24)
project(fuse-ext2)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS  OFF)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    "${CMAKE_SOURCE_DIR}/CMakeModules/")
    
# 根据CMAKE_MODULE_PATH下的配置寻找配置文件
find_package(FUSE REQUIRED)

#  增加一个子目录
add_subdirectory(${PROJECT_SOURCE_DIR}/src out)

#  用以测试
add_subdirectory(${PROJECT_SOURCE_DIR}/test)



