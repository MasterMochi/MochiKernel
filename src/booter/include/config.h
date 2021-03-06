/******************************************************************************/
/*                                                                            */
/* src/booter/include/Config.h                                                */
/*                                                                 2019/07/23 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef CONFIG_H
#define CONFIG_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdbool.h>
#include <stdint.h>

/* 共通ヘッダ */
#include <firmware/bios/e820.h>
#include <hardware/IA32/IA32Instruction.h>
#include <kernel/kernel.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* メモリアドレス */
#define MB_CONFIG_ADDR_KERNEL ( 0x00010000 )    /**< カーネルバッファアドレス */


/******************************************************************************/
#endif
