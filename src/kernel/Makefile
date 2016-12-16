#******************************************************************************#
#* src/kernel/Makefile                                                        *#
#*                                                                 2016/12/16 *#
#* Copyright (C) 2016 Mochi.                                                  *#
#******************************************************************************#
#******************************************************************************#
#* マクロ設定                                                                 *#
#******************************************************************************#
# ベースディレクトリsrc/からの相対パス
CUR_DIR      = kernel

# バイナリ名
KERNEL_NAME  = kernel

# ソースコード
SRCS = InitCtrl/InitCtrlInit.c \
       MemMng/MemMngInit.c     \
       MemMng/MemMngGdt.c      \
       IntMng/IntMngInit.c     \
       IntMng/IntMngIdt.c      \
       IntMng/IntMngHdl.c      \
       IntMng/IntMngPic.c      \
       TimerMng/TimerMngInit.c \
       TimerMng/TimerMngPit.c

# Cフラグ
CFLAGS = -O             \
         -Wall          \
         -masm=intel    \
         -m32           \
         -ffreestanding \
         -Iinclude/     \
         -I../include

# LDフラグ
LDFLAGS = -melf_i386 \
          -lc


#******************************************************************************#
#* 自動設定マクロ                                                             *#
#******************************************************************************#
# ベースディレクトリパス
BASE_DIR  = $(shell pwd | sed -e 's/\/src\/$(subst /,\/,$(CUR_DIR))//')
# 中間ファイル格納先ディレクトリパス
OBJS_DIR  = $(BASE_DIR)/build/objs/$(CUR_DIR)
# ディレクトリリスト
DIR_LIST  = $(sort $(addprefix $(OBJS_DIR)/, $(dir $(SRCS))))

# オブジェクトファイル
OBJS  = $(addprefix $(OBJS_DIR)/, $(SRCS:.c=.o))

# ライブラリディレクトリ
LDFLAGS += -L$(BASE_DIR)/build/objs/libraries


#******************************************************************************#
#* phonyターゲット                                                            *#
#******************************************************************************#
# サブディレクトリも含めたコンパイル
.PHONY: all
all: $(DIR_LIST) $(OBJS_DIR)/$(KERNEL_NAME) Makefile

# 全生成ファイルの削除
.PHONY: clean
clean:
	-rm -rf $(OBJS_DIR)


#******************************************************************************#
#* 生成規則                                                                   *#
#******************************************************************************#
ifdef DIR_LIST
$(DIR_LIST):
	mkdir -p $@
endif

# バイナリ
$(OBJS_DIR)/$(KERNEL_NAME): $(OBJS) Makefile
	$(LD) -T kernel.lds -o $@ $(OBJS) $(LDFLAGS)

# アセンブラファイルコンパイル
$(OBJS_DIR)/%.o: %.s Makefile
	$(AS) -o $@ $<

# Cファイルコンパイル
$(OBJS_DIR)/%.o: %.c Makefile
	$(CC) $(CFLAGS) -o $@ -c $<


#******************************************************************************#
