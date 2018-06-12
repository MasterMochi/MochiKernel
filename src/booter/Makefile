#******************************************************************************#
#* src/booter/Makefile                                                        *#
#*                                                                 2018/06/12 *#
#* Copyright (C) 2016-2018 Mochi.                                             *#
#******************************************************************************#
#******************************************************************************#
#* マクロ設定                                                                 *#
#******************************************************************************#
# ベースディレクトリsrc/からの相対パス
CUR_DIR   = booter

# IPL部バイナリ名
IPL_NAME  = booter-ipl.bin
# メイン部バイナリ名
MAIN_NAME = booter-main.bin

# IPL部ソースコード
IPL_SRCS  = Ipl/IplMain.s
# メイン部ソースコード
MAIN_SRCS = InitCtrl/InitCtrlInit16.s \
            InitCtrl/InitCtrlInit32.s \
            IntMng/IntMngInit.c       \
            IntMng/IntMngIdt.c        \
            IntMng/IntMngHdl.c        \
            IntMng/IntMngPic.c        \
            Driver/DriverInit.c       \
            Driver/DriverA20.c        \
            Driver/DriverAta.c        \
            LoadMng/LoadMngInit.c     \
            LoadMng/LoadMngKernel.c   \
            LoadMng/LoadMngProc.c     \
            Debug/DebugInit.c         \
            Debug/DebugLog.c

# ASフラグ
ASFLAGS = --32

# Cフラグ
CFLAGS = -O                 \
         -Wall              \
         -masm=intel        \
         -m32               \
         -fno-pic           \
         -ffreestanding     \
         -Iinclude/         \
         -I../include       \
         -DDEBUG_LOG_ENABLE

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
DIR_LIST  = $(sort $(addprefix $(OBJS_DIR)/, $(dir $(IPL_SRCS) $(MAIN_SRCS))))

# IPL部オブジェクトファイル
IPL_OBJS  = $(addprefix $(OBJS_DIR)/, $(IPL_SRCS:.s=.o))
# メイン部オブジェクトファイル
MAIN_OBJS = $(addprefix $(OBJS_DIR)/,                           \
              $(patsubst %.s,%.o,$(filter %.s, $(MAIN_SRCS)))   \
              $(patsubst %.c,%.o,$(filter %.c, $(MAIN_SRCS))))

# ライブラリディレクトリ
LDFLAGS += -L$(BASE_DIR)/build/objs/libraries


#******************************************************************************#
#* phonyターゲット                                                            *#
#******************************************************************************#
# サブディレクトリも含めたコンパイル
.PHONY: all
all: $(DIR_LIST) $(OBJS_DIR)/$(IPL_NAME) $(OBJS_DIR)/$(MAIN_NAME) Makefile

# 全生成ファイルの削除
.PHONY: clean
clean:
	-rm -rf $(OBJS_DIR)


#******************************************************************************#
#* 生成規則                                                                   *#
#******************************************************************************#
$(DIR_LIST):
	mkdir -p $@

# IPL部バイナリ
$(OBJS_DIR)/$(IPL_NAME): $(IPL_OBJS) Makefile
	$(LD) -T booter-ipl.lds -o $@ $(IPL_OBJS) $(LDFLAGS)

# メイン部バイナリ
$(OBJS_DIR)/$(MAIN_NAME): $(MAIN_OBJS) Makefile
	$(LD) -T booter-main.lds -o $@ $(MAIN_OBJS) $(LDFLAGS)

# アセンブラファイルコンパイル
$(OBJS_DIR)/%.o: %.s Makefile
	$(AS) $(ASFLAGS) -o $@ $<

# Cファイルコンパイル
$(OBJS_DIR)/%.o: %.c Makefile
	$(CC) $(CFLAGS) -o $@ -c $<


#******************************************************************************#
