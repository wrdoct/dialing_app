cmake_minimum_required(VERSION 3.10)
project(ping C)

# 设置C编译器标准为C11
set(CMAKE_C_STANDARD 11)

# 添加可执行文件 ping，并将 icmp_ping.c 作为源文件
add_executable(ping icmp_ping.c)

