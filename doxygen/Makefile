#******************************************************************************#
#*                                                                            *#
#* doxygen/Makefile                                                           *#
#*                                                                 2019/04/04 *#
#* Copyright (C) 2018-2019 Mochi.                                             *#
#*                                                                            *#
#******************************************************************************#
#******************************************************************************#
#* マクロ設定                                                                 *#
#******************************************************************************#
# Doxygenコンフィグ
BOOTER_CONFIG = ./booter.cnf
KERNEL_CONFIG = ./kernel.cnf
LIB_CONFIG    = ./lib.cnf
TOOL_CONFIG   = ./tool.cnf

# Doxygen出力先
BOOTER_OUTPUT = ./booter
KERNEL_OUTPUT = ./kernel
LIB_OUTPUT    = ./lib
TOOL_OUTPUT   = ./tool

# Doxygenログ
LOG_DIR    = ./log
BOOTER_LOG = $(LOG_DIR)/booter.log
KERNEL_LOG = $(LOG_DIR)/kernel.log
LIB_LOG    = $(LOG_DIR)/lib.log
TOOL_LOG   = $(LOG_DIR)/tool.log


#******************************************************************************#
#* phonyターゲット                                                            *#
#******************************************************************************#
# サブディレクトリも含めたコンパイル
.PHONY: all
all: booter kernel lib tool

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

# lib
.PHONY: lib
lib: $(LOG_DIR) $(LIB_CONFIG)
	@echo "*** lib ***"
	doxygen $(LIB_CONFIG)
	@cat $(LIB_LOG)
	@echo ""

# tool
.PHONY: tool
tool: $(LOG_DIR) $(TOOL_CONFIG)
	@echo "*** tool ***"
	doxygen $(TOOL_CONFIG)
	@cat $(TOOL_LOG)
	@echo ""

# 全生成ファイルの削除
.PHONY: clean
clean:
	-rm -rf $(LOG_DIR)
	-rm -rf $(BOOTER_OUTPUT)
	-rm -rf $(KERNEL_OUTPUT)
	-rm -rf $(LIB_OUTPUT)
	-rm -rf $(TOOL_OUTPUT)
	

#******************************************************************************#
#* 生成規則                                                                   *#
#******************************************************************************#
# log directory
$(LOG_DIR):
	mkdir -p $(LOG_DIR)


#******************************************************************************#
