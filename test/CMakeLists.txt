
# 启用调试的宏
add_definitions(-DFEXT2_DEBUG)
add_executable(test ../src/device.c 
                    ../src/fext2_init.c 
                    ../src/super.c 
                    ../src/alloc.c
                    ../src/bitmap.c
                    ../src/inode.c
                    ../src/utils.c
                    test.c)

# 针对目标增加的搜索路径
target_include_directories(test PUBLIC ${PROJECT_SOURCE_DIR}/include)