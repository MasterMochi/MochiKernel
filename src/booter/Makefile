#******************************************************************************#
#*                                                                            *#
#* src/booter/Makefile                                                        *#
#*                                                                 2019/04/21 *#
#* Copyright (C) 2016-2019 Mochi.                                             *#
#*                                                                            *#
#******************************************************************************#
#******************************************************************************#
#* 設定                                                                       *#
#******************************************************************************#
# IPL部バイナリ名
IPL_NAME = booter-ipl.bin
# IPL部ソースコード
IPL_SRCS = Ipl/IplMain.s

# メイン部バイナリ名
MAIN_NAME = booter-main.bin
# メイン部ソースコード
MAIN_SRCS  = InitCtrl/InitCtrlInit16.s
MAIN_SRCS += InitCtrl/InitCtrlInit32.s
MAIN_SRCS += IntMng/IntMngInit.c
MAIN_SRCS += IntMng/IntMngIdt.c
MAIN_SRCS += IntMng/IntMngHdl.c
MAIN_SRCS += IntMng/IntMngPic.c
MAIN_SRCS += MemMng/MemMngInit.c
MAIN_SRCS += MemMng/MemMngMap.c
MAIN_SRCS += Driver/DriverInit.c
MAIN_SRCS += Driver/DriverA20.c
MAIN_SRCS += Driver/DriverAta.c
MAIN_SRCS += LoadMng/LoadMngInit.c
MAIN_SRCS += LoadMng/LoadMngKernel.c
MAIN_SRCS += LoadMng/LoadMngProc.c
MAIN_SRCS += Debug/DebugInit.c
MAIN_SRCS += Debug/DebugLog.c

# リリースディレクトリ
RELEASE_DIR = ../../release/booter
# ビルドディレクトリ
BUILD_DIR   = ../../build
# オブジェクトディレクトリ
OBJ_DIR     = $(BUILD_DIR)/obj/booter

# ASフラグ
ASFLAGS  = --32
# Cフラグ
CFLAGS   = -O
CFLAGS  += -Wall
CFLAGS  += -masm=intel
CFLAGS  += -m32
CFLAGS  += -fno-pic
CFLAGS  += -ffreestanding
CFLAGS  += -Iinclude
CFLAGS  += -I../include
CFLAGS  += -I../../release/kernel/include
CFLAGS  += -I$(BUILD_DIR)/include
CFLAGS  += -DDEBUG_LOG_ENABLE
# LDフラグ
LDFLAGS  = -melf_i386
LDFLAGS += -L$(BUILD_DIR)/lib
# LDライブラリフラグ
LDLIBS   = -lc
LDLIBS  += -lMLib


#******************************************************************************#
#* 定義                                                                       *#
#******************************************************************************#
# オブジェクトサブディレクトリ
OBJ_SUBDIRS = $(sort $(addprefix $(OBJ_DIR)/, $(dir $(IPL_SRCS) $(MAIN_SRCS))))

# IPL部オブジェクトファイル
IPL_OBJS  = $(addprefix $(OBJ_DIR)/, $(IPL_SRCS:.s=.o))
# メイン部オブジェクトファイル
MAIN_OBJS = $(addprefix $(OBJ_DIR)/,                                     \
                        $(patsubst %.s,%.o,$(filter %.s, $(MAIN_SRCS)))  \
                        $(patsubst %.c,%.o,$(filter %.c, $(MAIN_SRCS))))

# メイン部依存関係ファイル
MAIN_DEPS = $(addprefix $(OBJ_DIR)/, $(MAIN_SRCS:.c=.d))


#******************************************************************************#
#* phonyターゲット                                                            *#
#******************************************************************************#
# コンパイル
.PHONY: all
all: $(OBJ_SUBDIRS) $(RELEASE_DIR)/$(IPL_NAME) $(RELEASE_DIR)/$(MAIN_NAME) Makefile

# 全生成ファイルの削除
.PHONY: clean
clean:
	-rm -rf $(RELEASE_DIR)/$(IPL_NAME)
	-rm -rf $(RELEASE_DIR)/$(MAIN_NAME)
	-rm -rf $(OBJ_DIR)


#******************************************************************************#
#* 生成規則                                                                   *#
#******************************************************************************#
# メイン部依存関係
-include $(MAIN_DEPS)

# オブジェクトサブディレクトリ
ifdef OBJ_SUBDIRS
$(OBJ_SUBDIRS):
	mkdir -p $@
endif

# IPL部バイナリ
$(RELEASE_DIR)/$(IPL_NAME): $(IPL_OBJS) Makefile
	$(LD) -T booter-ipl.lds $(LDFLAGS) -o $(OBJ_DIR)/$(IPL_NAME) $(IPL_OBJS) $(LDLIBS)
	ln -sfr $(OBJ_DIR)/$(IPL_NAME) $@

# メイン部バイナリ
$(RELEASE_DIR)/$(MAIN_NAME): $(MAIN_OBJS) Makefile
	$(LD) -T booter-main.lds $(LDFLAGS) -o $(OBJ_DIR)/$(MAIN_NAME) $(MAIN_OBJS) $(LDLIBS)
	ln -sfr $(OBJ_DIR)/$(MAIN_NAME) $@

# アセンブラファイルコンパイル
$(OBJ_DIR)/%.o: %.s Makefile
	$(AS) $(ASFLAGS) -o $@ $<

# Cファイルコンパイル
$(OBJ_DIR)/%.o: %.c Makefile
	$(CC) $(CFLAGS) -o $@ -c $< -MD -MP


#******************************************************************************#
