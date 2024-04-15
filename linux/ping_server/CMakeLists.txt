cmake_minimum_required(VERSION 3.4.1)
project(ping C)

# 设置C编译器标准为C11
set(CMAKE_C_STANDARD 11)

# 添加包含文件的目录
include_directories(
    ${CMAKE_SOURCE_DIR}/inc/
    ${CMAKE_SOURCE_DIR}/inc/rknn_api
)

# 设置链接库的目录
link_directories(
    ${CMAKE_SOURCE_DIR}/libs/opencv
)

# 添加 RKNN API 库文件
set(RKNN_API_LIBRARY
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/rknn_api/librknnrt.so
)


# 静态库输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/libs)
# 动态库输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
# 可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)

# 编译 inference.cpp 生成静态库
add_library(inference STATIC inference.cpp)

# 编译 inferinfo.cpp 生成静态库
add_library(inferinfo STATIC inferinfo.cpp)

# 指定链接的库文件
set(LIBRARIES
    m
    dl
    pthread
    opencv_highgui
    opencv_imgcodecs
    opencv_imgproc
    opencv_videoio
    opencv_core
)

# 创建可执行文件ping_server,将 main.c 作为源文件，并链接静态库
add_executable(ping_server main.c)
target_link_libraries(httpTest inferinfo inference  ${LIBRARIES} ${RKNN_API_LIBRARY})


