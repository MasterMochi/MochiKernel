/******************************************************************************/
/* src/kernel/include/Config.h                                                */
/*                                                                 2017/06/04 */
/* Copyright (C) 2017 Mochi.                                                  */
/******************************************************************************/
#ifndef CONFIG_H
#define CONFIG_H
/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* 仮想メモリマップ定義 */
#define CONFIG_MEM_KERNEL_START_ADDR ( 0x00100000 ) /** カーネル領域先頭アドレス           */
#define CONFIG_MEM_KERNEL_STACK_ADDR ( 0x3EFFE000 ) /** カーネル用スタック領域先頭アドレス */
#define CONFIG_MEM_KERNEL_STACK_SIZE ( 0x00002000 ) /** カーネル用スタック領域サイズ       */
#define CONFIG_MEM_KERNEL_MAP_ADDR1  ( 0x3F000000 ) /** メモリ制御用領域1先頭アドレス      */
#define CONFIG_MEM_KERNEL_MAP_SIZE1  ( 0x00800000 ) /** メモリ制御用領域1サイズ            */
#define CONFIG_MEM_KERNEL_MAP_ADDR2  ( 0x3F800000 ) /** メモリ制御用領域2先頭アドレス      */
#define CONFIG_MEM_KERNEL_MAP_SIZE2  ( 0x00800000 ) /** メモリ制御用領域2サイズ            */
#define CONFIG_MEM_KERNEL_MAP_SIZE   ( 0x01000000 ) /** メモリ制御用領域全サイズ           */
#define CONFIG_MEM_TASK_START_ADDR   ( 0x40000000 ) /** タスク領域先頭アドレス             */
#define CONFIG_MEM_TASK_STACK_ADDR   ( 0xFFFFE000 ) /** タスク用スタック領域先頭アドレス   */
#define CONFIG_MEM_TASK_STACK_SIZE   ( 0x00002000 ) /** タスク用スタック領域サイズ         */


/******************************************************************************/
#endif
