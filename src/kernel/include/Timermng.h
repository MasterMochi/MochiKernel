/******************************************************************************/
/*                                                                            */
/* src/kernel/include/Timermng.h                                              */
/*                                                                 2021/11/27 */
/* Copyright (C) 2018-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef TIMERMNG_H
#define TIMERMNG_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdint.h>

/* カーネルヘッダ */
#include <kernel/types.h>

/* 外部モジュールヘッダ */
#include <Intmng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* タイマID */
#define TIMERMNG_TIMERID_MIN  (    0 )                      /**< タイマID最小値 */
#define TIMERMNG_TIMERID_MAX  ( 1023 )                      /**< タイマID最大値 */
#define TIMERMNG_TIMERID_NUM  ( TIMERMNG_TIMERID_MAX + 1 )  /**< タイマID数     */
#define TIMERMNG_TIMERID_NULL ( TIMERMNG_TIMERID_NUM     )  /**< 無効タイマID   */

/* タイマ種別 */
#define TIMERMNG_TYPE_ONESHOT ( 0 )     /**< ワンショットタイマ種別 */
#define TIMERMNG_TYPE_REPEAT  ( 1 )     /**< 繰り返しタイマ種別     */

/** タイマコールバック関数型 */
typedef void ( *TimermngFunc_t )( uint32_t timerId, void *pArg );


/******************************************************************************/
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
/*------------*/
/* Timermng.c */
/*------------*/
/* タイマ管理初期化 */
extern void TimermngInit( void );


/*----------------*/
/* TimermngCtrl.c */
/*----------------*/
/* タスクID取得 */
extern MkTaskId_t TimermngCtrlGetTaskId( uint32_t timerId );

/* タイマ設定 */
extern uint32_t TimermngCtrlSet( uint32_t       usec,
                                 uint32_t       type,
                                 TimermngFunc_t pFunc,
                                 void           *pArg  );

/* タイマ解除 */
extern void TimermngCtrlUnset( uint32_t timerId );


/*---------------*/
/* TimermngPit.c */
/*---------------*/
/* PIT割込みハンドラ */
extern void TimermngPitHdlInt( uint32_t        intNo,
                               IntmngContext_t context );


/******************************************************************************/
#endif
