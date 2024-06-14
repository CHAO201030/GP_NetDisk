# 编译器
CC := gcc

# 编译器选项
CFLAGS := -Iinc -Wall -g -DDEBUG -pthread

# 链接器选项
LDFLAGS := -lssl -lcrypto -lcrypt 

# 源文件目录
SRC_DIR := src

# 头文件目录
INC_DIR := inc

# 目标文件目录
OBJ_DIR := obj

# 源文件列表
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)

# 目标文件列表
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

# 可执行文件
TARGET := client

# 生成规则
all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# 清理目标
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# 重建目标
rebuild: clean all

# 伪目标
.PHONY: all clean rebuild
