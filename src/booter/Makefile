#******************************************************************************#
#* src/booter/Makefile                                                        *#
#*                                                                 2016/12/04 *#
#* Copyright (C) 2016 Mochi.                                                  *#
#******************************************************************************#
#******************************************************************************#
#* マクロ設定                                                                 *#
#******************************************************************************#
# ベースディレクトリsrc/からの相対パス
CUR_DIR   = booter

# IPL部バイナリ名
IPL_NAME  = booter-ipl
# メイン部バイナリ名
MAIN_NAME = booter-main

# IPL部ソースコード
IPL_SRCS  = Ipl/IplMain.s
# メイン部ソースコード
MAIN_SRCS = Initctrl/InitctrlMain.s \
            Initctrl/InitctrlA20.s  \
            Initctrl/InitctrlCpu.s  \
            Loader/LoaderLoad.s

# ASフラグ
ASFLAGS = -32

# LDフラグ
LDFLAGS = -melf_i386


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
MAIN_OBJS = $(addprefix $(OBJS_DIR)/, $(MAIN_SRCS:.s=.o))


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
