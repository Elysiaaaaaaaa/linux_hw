# 项目名称，即最终生成的可执行文件名
TARGET = linux_hw

# 源文件列表
SRCS = ipc.cpp main.cpp

# 目标文件列表，将源文件的后缀.cpp替换为.o
OBJS = $(patsubst %.cpp, %.o, $(SRCS))

# C++ 编译器
CXX = g++

# 编译选项
CXXFLAGS = -Wall -g

# 默认目标，执行make时默认执行
all: $(TARGET)

# 可执行文件生成规则，依赖于所有的目标文件
$(TARGET): $(OBJS)
    $(CXX) $(CXXFLAGS) -o $@ $^

# 目标文件生成规则，每个.cpp文件生成对应的.o文件
%.o: %.cpp
    $(CXX) $(CXXFLAGS) -c -o $@ $<

# 清理规则，删除生成的目标文件和可执行文件
clean:
    rm -f $(OBJS) $(TARGET)

# 依赖检查规则，这里简单模拟，实际可能更复杂
depend:
    makedepend -- $(SRCS)

# 包含依赖文件（如果有生成的话）
-include $(patsubst %.cpp, %.d, $(SRCS))