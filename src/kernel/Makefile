#******************************************************************************#
#*                                                                            *#
#* src/kernel/Makefile                                                        *#
#*                                                                 2024/06/02 *#
#* Copyright (C) 2016-2024 Mochi.                                             *#
#*                                                                            *#
#******************************************************************************#
#******************************************************************************#
#* 設定                                                                       *#
#******************************************************************************#
# プログラム名
PROG = kernel

# ソースコード
SRCS  = Initctrl/Initctrl.c
SRCS += Debug/Debug.c
SRCS += Debug/DebugMem.c
SRCS += Debug/DebugVram.c
SRCS += Memmng/Memmng.c
SRCS += Memmng/MemmngSgmt.c
SRCS += Memmng/MemmngArea.c
SRCS += Memmng/MemmngPage.c
SRCS += Memmng/MemmngCtrl.c
SRCS += Memmng/MemmngHeap.c
SRCS += Memmng/MemmngMap.c
SRCS += Memmng/MemmngPhys.c
SRCS += Memmng/MemmngIo.c
SRCS += Memmng/MemmngVirt.c
SRCS += Taskmng/Taskmng.c
SRCS += Taskmng/TaskmngElf.c
SRCS += Taskmng/TaskmngName.c
SRCS += Taskmng/TaskmngProc.c
SRCS += Taskmng/TaskmngSched.c
SRCS += Taskmng/TaskmngTask.c
SRCS += Taskmng/TaskmngThread.c
SRCS += Taskmng/TaskmngTss.c
SRCS += Intmng/Intmng.c
SRCS += Intmng/IntmngIdt.c
SRCS += Intmng/IntmngHdl.c
SRCS += Intmng/IntmngPic.c
SRCS += Intmng/IntmngCtrl.c
SRCS += Timermng/Timermng.c
SRCS += Timermng/TimermngCtrl.c
SRCS += Timermng/TimermngPit.c
SRCS += Itcctrl/Itcctrl.c
SRCS += Itcctrl/ItcctrlMsg.c
SRCS += Ioctrl/Ioctrl.c
SRCS += Ioctrl/IoctrlPort.c
SRCS += Ioctrl/IoctrlMem.c

# ビルドディレクトリ
BUILD_DIR   = ../../build
# リリースディレクトリ
RELEASE_DIR = $(BUILD_DIR)/release
# オブジェクトディレクトリ
OBJ_DIR     = $(BUILD_DIR)/obj/$(PROG)

# Cフラグ
CFLAGS   = -O
CFLAGS  += -g
CFLAGS  += -Wall
CFLAGS  += -masm=intel
CFLAGS  += -m32
CFLAGS  += -fno-pic
CFLAGS  += -ffreestanding
CFLAGS  += -nostdlib
CFLAGS  += -DDEBUG_VRAM_ENABLE
CFLAGS  += -DDEBUG_MEM_ENABLE
CFLAGS  += -DDEBUG_ABORT_ENABLE
CFLAGS  += -DDEBUG_ERROR_ENABLE
CFLAGS  += -DDEBUG_WARNING_ENABLE
CFLAGS  += -DDEBUG_INFO_ENABLE
CFLAGS  += -DDEBUG_TRACE_ENABLE
CFLAGS  += -DDEBUG_TEMP_ENABLE
CFLAGS  += -Iinclude
CFLAGS  += -I../include
CFLAGS  += -I$(RELEASE_DIR)/include
CFLAGS  += -I$(BUILD_DIR)/include
ifdef BASE_CFLAGS
CFLAGS  += $(BASE_CFLAGS)
endif

LD = gcc

# LDフラグ
LDFLAGS  = -T kernel.lds
LDFLAGS += -m32
LDFLAGS += -ffreestanding
LDFLAGS += -nostdlib
LDFLAGS += -L$(BUILD_DIR)/lib
LDFLAGS += -L$(BUILD_DIR)/release
ifdef BASE_LDFLAGS
LDFLAGS += $(BASE_LDFLAGS)
endif

# LDライブラリフラグ
LIBS  = -lc
LIBS += -lmk
LIBS += -lMLib_fe
LIBS += -lgcc


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
all: $(OBJ_SUBDIRS) $(OBJS) $(RELEASE_DIR)/$(PROG) Makefile

# 全生成ファイルの削除
.PHONY: clean
clean:
	-rm -rf $(RELEASE_DIR)/$(PROG)
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

# 実行ファイル
$(RELEASE_DIR)/$(PROG): $(OBJS) Makefile
	$(LD) $(LDFLAGS) -o $(OBJ_DIR)/$(PROG) $(OBJS) $(LIBS)
	ln -sfr $(OBJ_DIR)/$(PROG) $@

# アセンブラファイルコンパイル
$(OBJ_DIR)/%.o: %.s Makefile
	$(AS) -o $@ $<

# Cファイルコンパイル
$(OBJ_DIR)/%.o: %.c Makefile
	$(CC) $(CFLAGS) -o $@ -c $< -MD -MP


#******************************************************************************#
