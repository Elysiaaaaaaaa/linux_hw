# ��Ŀ���ƣ����������ɵĿ�ִ���ļ���
TARGET = linux_hw

# Դ�ļ��б�
SRCS = ipc.cpp main.cpp

# Ŀ���ļ��б���Դ�ļ��ĺ�׺.cpp�滻Ϊ.o
OBJS = $(patsubst %.cpp, %.o, $(SRCS))

# C++ ������
CXX = g++

# ����ѡ��
CXXFLAGS = -Wall -g

# Ĭ��Ŀ�ִ꣬��makeʱĬ��ִ��
all: $(TARGET)

# ��ִ���ļ����ɹ������������е�Ŀ���ļ�
$(TARGET): $(OBJS)
    $(CXX) $(CXXFLAGS) -o $@ $^

# Ŀ���ļ����ɹ���ÿ��.cpp�ļ����ɶ�Ӧ��.o�ļ�
%.o: %.cpp
    $(CXX) $(CXXFLAGS) -c -o $@ $<

# �������ɾ�����ɵ�Ŀ���ļ��Ϳ�ִ���ļ�
clean:
    rm -f $(OBJS) $(TARGET)

# ���������������ģ�⣬ʵ�ʿ��ܸ�����
depend:
    makedepend -- $(SRCS)

# ���������ļ�����������ɵĻ���
-include $(patsubst %.cpp, %.d, $(SRCS))