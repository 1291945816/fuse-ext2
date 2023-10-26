aux_source_directory(.  SRC)

# 启用调试的宏
add_definitions(-DFEXT2_DEBUG -D_FILE_OFFSET_BITS=64  -Wall )
# add_definitions( -D_FILE_OFFSET_BITS=64  -Wall )
add_executable(${PROJECT_NAME} ${SRC})

# 针对目标增加的搜索路径
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include ${FUSE_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} ${FUSE_LIBRARIES})