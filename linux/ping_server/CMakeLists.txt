cmake_minimum_required(VERSION 3.0)
project(ping_server C)

# 添加源代码文件
file(GLOB SOURCES "src/*.c")

# 添加头文件搜索路径
include_directories(src/.)
# 或者使用 target_include_directories 命令将特定目录添加到特定目标
# target_include_directories(my_program PUBLIC .)

# 生成可执行文件
add_executable(ping_server ${SOURCES})

# 设置静态编译
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

# 链接静态库
target_link_libraries(ping_server -static)