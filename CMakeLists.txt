# 指定CMake的最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(linux_hw)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# 查找当前目录下的所有源文件
file(GLOB_RECURSE SOURCES "./src/*.cpp")

# 添加可执行文件，只列出源文件
add_executable(linux_hw ${SOURCES})

# 添加头文件搜索路径
include_directories(include)

# 如果ipc.h有对应的库，这里添加链接库
# target_link_libraries(linux_hw some_ipc_library)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")