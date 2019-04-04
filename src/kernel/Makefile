#******************************************************************************#
#*                                                                            *#
#* src/kernel/Makefile                                                        *#
#*                                                                 2019/04/03 *#
#* Copyright (C) 2016-2019 Mochi.                                             *#
#*                                                                            *#
#******************************************************************************#
#******************************************************************************#
#* 設定                                                                       *#
#******************************************************************************#
# プログラム名
PROG = kernel

# ソースコード
SRCS = InitCtrl/InitCtrlInit.c \
       Debug/DebugInit.c       \
       Debug/DebugLog.c        \
       MemMng/MemMngInit.c     \
       MemMng/MemMngGdt.c      \
       MemMng/MemMngArea.c     \
       MemMng/MemMngPage.c     \
       MemMng/MemMngCtrl.c     \
       MemMng/MemMngMap.c      \
       MemMng/MemMngPhys.c     \
       MemMng/MemMngIo.c       \
       MemMng/MemMngVirt.c     \
       TaskMng/TaskMngInit.c   \
       TaskMng/TaskMngTss.c    \
       TaskMng/TaskMngTask.c   \
       TaskMng/TaskMngSched.c  \
       TaskMng/TaskMngElf.c    \
       TaskMng/TaskMngProc.c   \
       TaskMng/TaskMngName.c   \
       IntMng/IntMngInit.c     \
       IntMng/IntMngIdt.c      \
       IntMng/IntMngHdl.c      \
       IntMng/IntMngPic.c      \
       IntMng/IntMngCtrl.c     \
       TimerMng/TimerMngInit.c \
       TimerMng/TimerMngCtrl.c \
       TimerMng/TimerMngPit.c  \
       ItcCtrl/ItcCtrlInit.c   \
       ItcCtrl/ItcCtrlMsg.c    \
       IoCtrl/IoCtrlInit.c     \
       IoCtrl/IoCtrlPort.c     \
       IoCtrl/IoCtrlMem.c

# リリースディレクトリ
RELEASE_DIR = ../../release/kernel
# ビルドディレクトリ
BUILD_DIR   = ../../build
# オブジェクトディレクトリ
OBJ_DIR     = $(BUILD_DIR)/obj/$(PROG)

# Cフラグ
CFLAGS  = -O                     \
          -Wall                  \
          -masm=intel            \
          -m32                   \
          -fno-pic               \
          -ffreestanding         \
          -Iinclude              \
          -I../include           \
          -I$(BUILD_DIR)/include
#          -DDEBUG_LOG_ENABLE

# LDフラグ
LDFLAGS = -T kernel.lds      \
          -melf_i386         \
          -L$(BUILD_DIR)/lib

# LDライブラリフラグ
LDLIBS  = -lc    \
          -lMLib


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
	$(LD) $(LDFLAGS) -o $(OBJ_DIR)/$(PROG) $(OBJS) $(LDLIBS)
	ln -sfr $(OBJ_DIR)/$(PROG) $@

# アセンブラファイルコンパイル
$(OBJ_DIR)/%.o: %.s Makefile
	$(AS) -o $@ $<

# Cファイルコンパイル
$(OBJ_DIR)/%.o: %.c Makefile
	$(CC) $(CFLAGS) -o $@ -c $< -MD -MP


#******************************************************************************#
