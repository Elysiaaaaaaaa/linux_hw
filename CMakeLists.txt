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

# 添加可执行文件
add_executable(linux_hw ${SOURCES})

# 添加头文件搜索路径
target_include_directories(linux_hw PRIVATE include)

# 设置编译选项
target_compile_options(linux_hw PRIVATE -Wall -g)

# 查找线程库（如果需要）
find_package(Threads REQUIRED)
target_link_libraries(linux_hw PRIVATE Threads::Threads)