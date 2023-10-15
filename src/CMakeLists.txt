aux_source_directory(.  SRC)

# 启用调试的宏
add_definitions(-DFEXT2_DEBUG)
add_executable(${PROJECT_NAME} ${SRC})

# 针对目标增加的搜索路径
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
