/******************************************************************************/
/*                                                                            */
/* kernel/config.h                                                            */
/*                                                                 2019/06/12 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef _MK_CONFIG_H_
#define _MK_CONFIG_H_
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* カーネルヘッダ */
#include "kernel.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/*--------*/
/* ID定義 */
/*--------*/
#define MK_CONFIG_PID_MAX  ( 1024 ) /** プロセスID最大値 */
#define MK_CONFIG_TID_MAX  ( 4 )    /** スレッドID最大値 */

/*------*/
/* tick */
/*------*/
/** tick間隔(hz) */
#define MK_CONFIG_TICK_HZ ( 100 )

/*----------------------*/
/* 仮想メモリマップ定義 */
/*----------------------*/
/** ブートデータ領域先頭アドレス */
#define MK_CONFIG_ADDR_BOOTDATA     ( NULL )
/** ブートデータ領域サイズ */
#define MK_CONFIG_SIZE_BOOTDATA     ( 0x00100000 )
/** カーネル領域先頭アドレス */
#define MK_CONFIG_ADDR_KERNEL_START ( MK_ADDR_ENTRY )
/** カーネル用スタック領域先頭アドレス */
#define MK_CONFIG_ADDR_KERNEL_STACK ( 0x3EFFE000 )
/** カーネル用スタック領域サイズ */
#define MK_CONFIG_SIZE_KERNEL_STACK ( 0x00002000 )
/** メモリ制御用領域1先頭アドレス */
#define MK_CONFIG_ADDR_KERNEL_MAP1  ( 0x3F000000 )
/** メモリ制御用領域1サイズ */
#define MK_CONFIG_SIZE_KERNEL_MAP1  ( 0x00800000 )
/** メモリ制御用領域2先頭アドレス */
#define MK_CONFIG_ADDR_KERNEL_MAP2  ( 0x3F800000 )
/** メモリ制御用領域2サイズ */
#define MK_CONFIG_SIZE_KERNEL_MAP2  ( 0x00800000 )
/** メモリ制御用領域全サイズ */
#define MK_CONFIG_SIZE_KERNEL_MAP   ( 0x01000000 )
/** 全カーネル領域サイズ */
#define MK_CONFIG_SIZE_KERNEL       ( 0x3FF00000 )
/** アプリ領域先頭アドレス */
#define MK_CONFIG_ADDR_APL_START    ( 0x40000000 )
/** アプリ用スタック領域先頭アドレス */
#define MK_CONFIG_ADDR_APL_STACK    ( 0xBFFFE000 )
/** アプリ用スタック領域サイズ */
#define MK_CONFIG_SIZE_APL_STACK    ( 0x00008000 )
/** 全アプリ領域サイズ */
#define MK_CONFIG_SIZE_APL          ( 0x40000000 )

/*------------*/
/* 割込み番号 */
/*------------*/
/** メッセージパッシング割込み番号 */
#define MK_CONFIG_INTNO_MESSAGE   ( 0x30 )
/** I/Oポート制御割込み番号 */
#define MK_CONFIG_INTNO_IOPORT    ( 0x31 )
/** I/Oメモリ制御割込み番号 */
#define MK_CONFIG_INTNO_IOMEM     ( 0x32 )
/** ハードウェア割込み制御割込み番号 */
#define MK_CONFIG_INTNO_INTERRUPT ( 0x33 )
/** タイマ割込み番号 */
#define MK_CONFIG_INTNO_TIMER     ( 0x34 )
/** プロセス管理割込み番号 */
#define MK_CONFIG_INTNO_PROC      ( 0x35 )
/** タスク名管理割込み番号　*/
#define MK_CONFIG_INTNO_TASKNAME  ( 0x36 )

/*--------------*/
/* タスク名管理 */
/*--------------*/
/** タスク名文字列長(NULL文字含まない) */
#define MK_CONFIG_TASKNAME_LENMAX ( 63 )
/** タスク名管理数　*/
#define MK_CONFIG_TASKNAME_NUM    ( 64 )


/******************************************************************************/
#endif
