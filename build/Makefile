#******************************************************************************#
#* build/Makefile                                                             *#
#*                                                                 2016/12/05 *#
#* Copyright (C) 2016 Mochi                                                   *#
#******************************************************************************#
#******************************************************************************#
#* マクロ設定                                                                 *#
#******************************************************************************#
# ディスクイメージファイル
IMG_NAME    = mochi.img

# パーティションテーブル
PTABLE      = ptable.bin

# ループデバイス
LOOP_DEV    = /dev/loop0
LOOP_DEV_P1 = /dev/loop0p1
LOOP_DEV_P2 = /dev/loop0p2

# booter
BOOTER_IPL  = objs/booter/booter-ipl
BOOTER_MAIN = objs/booter/booter-main

# kernel
KERNEL      = objs/kernel/kernel


#******************************************************************************#
#* phonyターゲット                                                            *#
#******************************************************************************#
# サブディレクトリも含めたコンパイル
.PHONY: all
all:
	make -C ../src/ all

# 全生成ファイルの削除
.PHONY: clean
clean:
	make -C ../src/ clean
	-rm -f $(IMG_NAME)

# ディスクイメージの作成
.PHONY: image
image: $(IMG_NAME)


#******************************************************************************#
#* イメージファイル作成                                                       *#
#******************************************************************************#
$(IMG_NAME): all
	sync
	dd if=/dev/zero of=$@ count=10080 conv=fsync
	dd if=$(BOOTER_IPL) of=$@ conv=notrunc,fsync
	dd if=$(PTABLE) of=$@ ibs=1 obs=1 seek=446 conv=notrunc,fsync
	sudo losetup $(LOOP_DEV) $@
	-sudo partx -a $(LOOP_DEV)
	-sudo dd if=$(BOOTER_MAIN) of=$(LOOP_DEV_P1) conv=fsync
	-sudo dd if=$(KERNEL) of=$(LOOP_DEV_P2) conv=fsync
	sync
	-sudo partx -d $(LOOP_DEV)
	sudo losetup -d $(LOOP_DEV)


#******************************************************************************#
