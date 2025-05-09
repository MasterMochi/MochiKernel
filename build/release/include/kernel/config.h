/******************************************************************************/
/*                                                                            */
/* kernel/config.h                                                            */
/*                                                                 2023/01/04 */
/* Copyright (C) 2018-2023 Mochi.                                             */
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
