/******************************************************************************/
/*                                                                            */
/* kernel/config.h                                                            */
/*                                                                 2019/08/11 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef __KERNEL_CONFIG_H__
#define __KERNEL_CONFIG_H__
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
#define MK_CONFIG_PID_MASK  ( 0x3FF )   /** プロセスIDマスク(最大値) */
#define MK_CONFIG_TID_MASK  ( 0x3   )   /** スレッドIDマスク(最大値) */

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
/** ユーザ空間領域先頭アドレス */
#define MK_CONFIG_ADDR_USER_START   ( 0x40000000 )
/** ユーザ空間スタック領域先頭アドレス */
#define MK_CONFIG_ADDR_USER_STACK   ( 0xBFFFE000 )
/** ユーザ空間スタック領域サイズ */
#define MK_CONFIG_SIZE_USER_STACK   ( 0x00008000 )
/** 全ユーザ空間領域サイズ */
#define MK_CONFIG_SIZE_USER         ( 0x80000000 )

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
/** タスク名管理割込み番号 */
#define MK_CONFIG_INTNO_TASKNAME  ( 0x36 )
/** スレッド管理割込み番号 */
#define MK_CONFIG_INTNO_THREAD    ( 0x37 )
/** タスク管理割込み番号 */
#define MK_CONFIG_INTNO_TASK      ( 0x38 )

/*--------------*/
/* タスク名管理 */
/*--------------*/
/** タスク名文字列長(NULL文字含まない) */
#define MK_CONFIG_TASKNAME_LENMAX ( 63 )
/** タスク名管理数　*/
#define MK_CONFIG_TASKNAME_NUM    ( 64 )


/******************************************************************************/
#endif
