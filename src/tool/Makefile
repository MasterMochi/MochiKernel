#******************************************************************************#
#*                                                                            *#
#* src/tool/Makefile                                                          *#
#*                                                                 2019/04/21 *#
#* Copyright (C) 2017-2019 Mochi.                                             *#
#*                                                                            *#
#******************************************************************************#
#******************************************************************************#
#* 設定                                                                       *#
#******************************************************************************#
# ビルドディレクトリ
BUILD_DIR = ../../build
# オブジェクトディレクトリ
OBJ_DIR   = $(BUILD_DIR)/obj/tool
# サブディレクトリ
SUB_DIRS  = makedisk
SUB_DIRS += makeimg


#******************************************************************************#
#* phonyターゲット                                                            *#
#******************************************************************************#
# サブディレクトリのmake実行
.PHONY: all
all:
ifdef SUB_DIRS
	@for subdir in $(SUB_DIRS); \
	do \
	    $(MAKE) -C $$subdir all; \
	done
endif

# 全生成ファイルの削除
.PHONY: clean
clean:
ifdef SUB_DIRS
	@for subdir in $(SUB_DIRS); \
	do \
	    $(MAKE) -C $$subdir clean; \
	done
endif
	-rm -rf $(OBJ_DIR)


#******************************************************************************#
