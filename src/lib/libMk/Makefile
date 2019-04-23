#******************************************************************************#
#*                                                                            *#
#* src/libraries/libMk/Makefile                                               *#
#*                                                                 2019/04/23 *#
#* Copyright (C) 2018-2019 Mochi.                                             *#
#*                                                                            *#
#******************************************************************************#
#******************************************************************************#
#* 設定                                                                       *#
#******************************************************************************#
# ライブラリ
LIB = libMk

# ソースコード
SRCS  = MkMsg.c
SRCS += MkInt.c
SRCS += MkIoMem.c
SRCS += MkIoPort.c
SRCS += MkProc.c
SRCS += MkTimer.c
SRCS += MkTaskName.c

# ビルドディレクトリ
BUILD_DIR   = ../../../build
# リリースディレクトリ
RELEASE_DIR = $(BUILD_DIR)/release/lib
# オブジェクトディレクトリ
OBJ_DIR     = $(BUILD_DIR)/obj/lib/$(LIB)

# Cフラグ
CFLAGS  = -O
CFLAGS += -Wall
CFLAGS += -masm=intel
CFLAGS += -m32
CFLAGS += -static
CFLAGS += -ffreestanding
CFLAGS += -I../../include
CFLAGS += -I$(RELEASE_DIR)/include
CFLAGS += -I$(BUILD_DIR)/release/kernel/include
CFLAGS += -I$(BUILD_DIR)/include
ifdef BASE_CFLAGS
CFLAGS += BASE_CFLAGS
endif


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
