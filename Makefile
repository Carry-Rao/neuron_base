# 编译器设置
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2 `pkg-config --cflags gtk+-3.0` -I./include -I./include/stb
LDFLAGS = -lm `pkg-config --libs gtk+-3.0`

CFLAGS += -O3 -march=native -fopenmp
CXXFLAGS += -O3 -march=native -fopenmp

# 目录设置
INCLUDE_DIR = ./include
CODE_DIR = ./src
OUT_DIR = ./out
PROGRAM_DIR = ./bin

# 创建输出目录
$(shell mkdir -p $(OUT_DIR) $(PROGRAM_DIR))

# 收集源文件：
# 1. code目录下的所有cpp文件（包括no_training.cpp）
# 2. code子目录下的train.cpp和recognize.cpp
ROOT_CPP_FILES = $(wildcard $(CODE_DIR)/*.cpp)
SUB_TRAIN_FILES = $(shell find $(CODE_DIR) -type f -name "train.cpp")
SUB_RECOGNIZE_FILES = $(shell find $(CODE_DIR) -type f -name "recognize.cpp")
ALL_CPP_FILES = $(ROOT_CPP_FILES) $(SUB_TRAIN_FILES) $(SUB_RECOGNIZE_FILES)

# 收集include目录下的cc文件
CC_FILES = $(wildcard $(INCLUDE_DIR)/*.cc)

# 生成目标文件路径（保持目录结构）
CC_OBJS = $(patsubst $(INCLUDE_DIR)/%.cc, $(OUT_DIR)/%.o, $(CC_FILES))
CPP_OBJS = $(patsubst $(CODE_DIR)/%.cpp, $(OUT_DIR)/%.o, $(ALL_CPP_FILES))

# 生成可执行文件路径（保持目录结构）
EXECUTABLES = $(patsubst $(CODE_DIR)/%.cpp, $(PROGRAM_DIR)/%, $(ALL_CPP_FILES))

# 总目标
all: $(CC_OBJS) $(CPP_OBJS) $(EXECUTABLES)

# 链接规则：根据源文件路径生成对应可执行文件
$(PROGRAM_DIR)/%: $(OUT_DIR)/%.o $(CC_OBJS)
	@mkdir -p $(dir $@)  # 确保输出目录存在
	$(CXX) -o $@ $< $(CC_OBJS) $(LDFLAGS)

# 编译include目录的cc文件
$(OUT_DIR)/%.o: $(INCLUDE_DIR)/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 编译code目录及其子目录的cpp文件
$(OUT_DIR)/%.o: $(CODE_DIR)/%.cpp
	@mkdir -p $(dir $@)  # 自动创建对应的子目录
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理目标
clean:
	rm -rf $(OUT_DIR)/* $(PROGRAM_DIR)/*

.PHONY: all clean
