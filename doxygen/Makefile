#******************************************************************************#
#* doxygen/Makefile                                                           *#
#*                                                                 2018/11/25 *#
#* Copyright (C) 2018 Mochi.                                                  *#
#******************************************************************************#
#******************************************************************************#
#* マクロ設定                                                                 *#
#******************************************************************************#
# Doxygenコンフィグ
BOOTER_CONFIG = ./booter.cnf
KERNEL_CONFIG = ./kernel.cnf
LIBS_CONFIG   = ./libraries.cnf
TOOLS_CONFIG  = ./tools.cnf

# Doxygen出力先
BOOTER_OUTPUT = ./booter
KERNEL_OUTPUT = ./kernel
LIBS_OUTPUT   = ./libraries
TOOLS_OUTPUT  = ./tools

# Doxygenログ
LOG_DIR    = ./log
BOOTER_LOG = $(LOG_DIR)/booter.log
KERNEL_LOG = $(LOG_DIR)/kernel.log
LIBS_LOG   = $(LOG_DIR)/libraries.log
TOOLS_LOG  = $(LOG_DIR)/tools.log


#******************************************************************************#
#* phonyターゲット                                                            *#
#******************************************************************************#
# サブディレクトリも含めたコンパイル
.PHONY: all
all: booter kernel libraries tools

# booter
.PHONY: booter
booter: $(LOG_DIR) $(BOOTER_CONFIG)
	@echo "*** booter ***"
	doxygen $(BOOTER_CONFIG)
	@cat $(BOOTER_LOG)
	@echo ""
	
# kernel
.PHONY: kernel
kernel: $(LOG_DIR) $(KERNEL_CONFIG)
	@echo "*** kernel ***"
	doxygen $(KERNEL_CONFIG)
	@cat $(KERNEL_LOG)
	@echo ""

# libraries
.PHONY: libraries
libraries: $(LOG_DIR) $(LIBS_CONFIG)
	@echo "*** libraries ***"
	doxygen $(LIBS_CONFIG)
	@cat $(LIBS_LOG)
	@echo ""

# tools
.PHONY: tools
tools: $(LOG_DIR) $(TOOLS_CONFIG)
	@echo "*** tools ***"
	doxygen $(TOOLS_CONFIG)
	@cat $(TOOLS_LOG)
	@echo ""

# 全生成ファイルの削除
.PHONY: clean
clean:
	-rm -rf $(LOG_DIR)
	-rm -rf $(BOOTER_OUTPUT)
	-rm -rf $(KERNEL_OUTPUT)
	-rm -rf $(LIBS_OUTPUT)
	-rm -rf $(TOOLS_OUTPUT)
	

#******************************************************************************#
#* 生成規則                                                                   *#
#******************************************************************************#
# log directory
$(LOG_DIR):
	mkdir -p $(LOG_DIR)


#******************************************************************************#
