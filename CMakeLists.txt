# 指定CMake的最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(linux_hw)

# 设置C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# 查找当前目录下的所有源文件
file(GLOB_RECURSE SOURCES "./src/*.cpp")

# 添加可执行文件
add_executable(linux_hw ${SOURCES}
        include/logger.h
        include/mp.h
        include/Car.h
        include/Tunnel.h)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")