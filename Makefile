# 编译器设置
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -O2 `pkg-config --cflags gtk+-3.0`
LDFLAGS = -lm `pkg-config --libs gtk+-3.0`

# 查找所有源文件
CPP_FILES = $(wildcard *.cpp)   # 所有.cpp文件
CC_FILES = $(wildcard *.cc)     # 所有.cc文件

# 生成目标文件和可执行文件列表
CC_OBJS = $(CC_FILES:.cc=.o)    # .cc文件对应的.o文件
CPP_OBJS = $(CPP_FILES:.cpp=.o)  # .cpp文件对应的.o文件
ALL_OBJS = $(CC_OBJS) $(CPP_OBJS)  # 所有目标文件
EXECUTABLES = $(CPP_FILES:.cpp=)   # 每个.cpp对应一个可执行文件

# 默认目标：先编译所有目标文件，再生成可执行文件
all: $(ALL_OBJS) $(EXECUTABLES)

# 链接规则：将对应的.cpp目标文件和所有.cc目标文件链接为可执行文件
%: %.o $(CC_OBJS)
	@$(CXX) -o $@ $< $(CC_OBJS) $(LDFLAGS)

# 编译规则：.cc文件 -> .o文件
%.o: %.cc
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# 编译规则：.cpp文件 -> .o文件
%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理目标
clean:
	@rm -rf $(ALL_OBJS) $(EXECUTABLES)

# 伪目标
.PHONY: all clean
    
