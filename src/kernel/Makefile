#******************************************************************************#
#*                                                                            *#
#* src/kernel/Makefile                                                        *#
#*                                                                 2020/11/03 *#
#* Copyright (C) 2016-2020 Mochi.                                             *#
#*                                                                            *#
#******************************************************************************#
#******************************************************************************#
#* 設定                                                                       *#
#******************************************************************************#
# プログラム名
PROG = kernel

# ソースコード
SRCS  = InitCtrl/InitCtrl.c
SRCS += Debug/Debug.c
SRCS += Debug/DebugLog.c
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
SRCS += TaskMng/TaskMng.c
SRCS += TaskMng/TaskMngElf.c
SRCS += TaskMng/TaskMngName.c
SRCS += TaskMng/TaskMngProc.c
SRCS += TaskMng/TaskMngSched.c
SRCS += TaskMng/TaskMngTask.c
SRCS += TaskMng/TaskMngTbl.c
SRCS += TaskMng/TaskMngThread.c
SRCS += TaskMng/TaskMngTss.c
SRCS += IntMng/IntMng.c
SRCS += IntMng/IntMngIdt.c
SRCS += IntMng/IntMngHdl.c
SRCS += IntMng/IntMngPic.c
SRCS += IntMng/IntMngCtrl.c
SRCS += TimerMng/TimerMng.c
SRCS += TimerMng/TimerMngCtrl.c
SRCS += TimerMng/TimerMngPit.c
SRCS += ItcCtrl/ItcCtrl.c
SRCS += ItcCtrl/ItcCtrlMsg.c
SRCS += IoCtrl/IoCtrl.c
SRCS += IoCtrl/IoCtrlPort.c
SRCS += IoCtrl/IoCtrlMem.c

# ビルドディレクトリ
BUILD_DIR   = ../../build
# リリースディレクトリ
RELEASE_DIR = $(BUILD_DIR)/release
# オブジェクトディレクトリ
OBJ_DIR     = $(BUILD_DIR)/obj/$(PROG)

# Cフラグ
CFLAGS   = -O
CFLAGS  += -Wall
CFLAGS  += -masm=intel
CFLAGS  += -m32
CFLAGS  += -fno-pic
CFLAGS  += -ffreestanding
#CFLAGS  += -DDEBUG_LOG_ENABLE
CFLAGS  += -Iinclude
CFLAGS  += -I../include
CFLAGS  += -I$(RELEASE_DIR)/include
CFLAGS  += -I$(BUILD_DIR)/include
ifdef BASE_CFLAGS
CFLAGS  += $(BASE_CFLAGS)
endif
# LDフラグ
LDFLAGS  = -T kernel.lds
LDFLAGS += -melf_i386
LDFLAGS += -L$(BUILD_DIR)/lib
ifdef BASE_LDFLAGS
LDFLAGS += $(BASE_LDFLAGS)
endif
# LDライブラリフラグ
LIBS     = -lc
LIBS    += -lMLib


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
