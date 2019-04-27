#******************************************************************************#
#*                                                                            *#
#* src/tool/makeimg/Makefile                                                  *#
#*                                                                 2019/04/27 *#
#* Copyright (C) 2017-2019 Mochi.                                             *#
#*                                                                            *#
#******************************************************************************#
#******************************************************************************#
#* 設定                                                                       *#
#******************************************************************************#
# 実行ファイル
PROG = makeimg

# ソースコード
SRCS = makeimg.c

# ビルドディレクトリ
BUILD_DIR   = ../../../build
# リリースディレクトリ
RELEASE_DIR = $(BUILD_DIR)/release/tool
# ツールディレクトリ
OBJ_DIR     = $(BUILD_DIR)/obj/tool

# Cフラグ
CFLAGS   = -m32
CFLAGS  += -I$(BUILD_DIR)/release/include/kernel
CFLAGS  += -I$(BUILD_DIR)/include
ifdef BASE_CFLAGS
CFLAGS  += $(BASE_CFLAGS)
endif
# LDフラグ
LDFLAGS  = -L$(BUILD_DIR)/lib
ifdef BASE_LDFLAGS
LDFLAGS += $(BASE_LDFLAGS)
endif
# LDライブラリフラグ
LIBS     = -lMLib


#******************************************************************************#
#* phonyターゲット                                                            *#
#******************************************************************************#
# コンパイル
.PHONY: all
all: $(OBJ_DIR) $(RELEASE_DIR)/$(PROG) Makefile

# 全生成ファイルの削除
.PHONY: clean
clean:
	-rm -rf $(RELEASE_DIR)/$(PROG)
	-rm -rf $(OBJ_DIR)/$(PROG)


#******************************************************************************#
#* 生成規則                                                                   *#
#******************************************************************************#
# ディレクトリ
$(OBJ_DIR):
	mkdir -p $@

# 実行ファイル
$(RELEASE_DIR)/$(PROG): $(SRCS) Makefile
	$(CC) $(CFLAGS) -o $(OBJ_DIR)/$(PROG) $(SRCS) $(LDFLAGS) $(LIBS)
	ln -sfr $(OBJ_DIR)/$(PROG) $@


#******************************************************************************#
