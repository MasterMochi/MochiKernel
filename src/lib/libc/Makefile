#******************************************************************************#
#*                                                                            *#
#* src/libraries/libc/Makefile                                                *#
#*                                                                 2019/04/21 *#
#* Copyright (C) 2016-2019 Mochi.                                             *#
#*                                                                            *#
#******************************************************************************#
#******************************************************************************#
#* 設定                                                                       *#
#******************************************************************************#
# バイナリ名
LIB = libc

# ソースコード
SRCS  = string/memcpy.c
SRCS += string/memset.c
SRCS += string/strcpy.c
SRCS += string/strncmp.c
SRCS += string/strncpy.c
SRCS += string/strlen.c

# リリースディレクトリ
RELEASE_DIR = ../../../build/lib/
# ビルドディレクトリ
BUILD_DIR   = ../../../build
# オブジェクトディレクトリ
OBJ_DIR     = $(BUILD_DIR)/obj/lib/$(LIB)

# Cフラグ
CFLAGS  = -O
CFLAGS += -Wall
CFLAGS += -masm=intel
CFLAGS += -m32
CFLAGS += -ffreestanding
CFLAGS += -I../../include
CFLAGS += -I$(BUILD_DIR)/include


#******************************************************************************#
#* 定義                                                                       *#
#******************************************************************************#
# オブジェクトサブディレクトリ
OBJ_SUBDIRS = $(sort $(addprefix $(OBJ_DIR)/, $(dir $(SRCS))))

# オブジェクトファイル
OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))

# 依存関係ファイル
DEPS = $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.d))


#******************************************************************************#
#* phonyターゲット                                                            *#
#******************************************************************************#
# コンパイル
.PHONY: all
all: $(OBJ_SUBDIRS) $(RELEASE_DIR)/$(LIB).a Makefile

# 全生成ファイルの削除
.PHONY: clean
clean:
	-rm -rf $(RELEASE_DIR)/$(LIB).a
	-rm -rf $(OBJ_DIR)


#******************************************************************************#
#* 生成規則                                                                   *#
#******************************************************************************#
# 依存関係
-include $(DEPS)

# オブジェクトサブディレクトリ
ifdef OBJ_SUBDIRS
$(OBJ_SUBDIRS):
	mkdir -p $@
endif

# ライブラリ
$(RELEASE_DIR)/$(LIB).a: $(OBJS) Makefile
	$(AR) rcs $(OBJ_DIR)/$(LIB).a $(OBJS)
	ln -srf $(OBJ_DIR)/$(LIB).a $@

# アセンブラファイルコンパイル
$(OBJ_DIR)/%.o: %.s Makefile
	$(AS) -o $@ $<

# Cファイルコンパイル
$(OBJ_DIR)/%.o: %.c Makefile
	$(CC) $(CFLAGS) -o $@ -c $< -MD -MP


#******************************************************************************#
