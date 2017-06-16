/******************************************************************************/
/* src/kernel/include/Cmn.h                                                   */
/*                                                                 2017/06/16 */
/* Copyright (C) 2017 Mochi.                                                  */
/******************************************************************************/
#ifndef CMN_H
#define CMN_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
#include <stdint.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* 処理結果 */
#define CMN_SUCCESS (  0 )  /** 成功 */
#define CMN_FAILURE ( -1 )  /** 失敗 */

/* モジュール・サブモジュール識別子 */
#define CMN_MODULE_INIT_INIT     ( 0x0101 ) /** 初期化制御(初期化)         */
#define CMN_MODULE_MEMMNG_INIT   ( 0x0201 ) /** メモリ管理(初期化)         */
#define CMN_MODULE_MEMMNG_GDT    ( 0x0202 ) /** メモリ管理(GDT管理)        */
#define CMN_MODULE_MEMMNG_AREA   ( 0x0203 ) /** メモリ管理(メモリ領域管理) */
#define CMN_MODULE_MEMMNG_PAGE   ( 0x0204 ) /** メモリ管理(ページ管理)     */
#define CMN_MODULE_MEMMNG_CTRL   ( 0x0205 ) /** メモリ管理(メモリ制御)     */
#define CMN_MODULE_INTMNG_INIT   ( 0x0301 ) /** 割込管理(初期化)           */
#define CMN_MODULE_INTMNG_PIC    ( 0x0302 ) /** 割込管理(PIC管理)          */
#define CMN_MODULE_INTMNG_IDT    ( 0x0303 ) /** 割込管理(IDT管理)          */
#define CMN_MODULE_INTMNG_HDL    ( 0x0304 ) /** 割込管理(ハンドラ管理)     */
#define CMN_MODULE_TIMERMNG_INIT ( 0x0401 ) /** タイマ管理(初期化)         */
#define CMN_MODULE_TIMERMNG_PIT  ( 0x0402 ) /** タイマ管理(PIT管理)        */
#define CMN_MODULE_PROCMNG_INIT  ( 0x0501 ) /** プロセス管理(初期化)       */
#define CMN_MODULE_PROCMNG_TSS   ( 0x0502 ) /** プロセス管理(TSS管理)      */
#define CMN_MODULE_PROCMNG_SCHED ( 0x0503 ) /** プロセス管理(スケジューラ) */
#define CMN_MODULE_PROCMNG_TASK  ( 0x0504 ) /** プロセス管理(タスク管理)   */
#define CMN_MODULE_PROCMNG_ELF   ( 0x0505 ) /** プロセス管理(ELFローダ)    */
#define CMN_MODULE_DEBUG_INIT    ( 0x0601 ) /** デバッグ制御(初期化)       */
#define CMN_MODULE_DEBUG_LOG     ( 0x0602 ) /** デバッグ制御(ログ管理)     */

/** モジュール・サブモジュール数 */
#define CMN_MODULE_NUM           ( 19 )

/* 処理結果構造体 */
typedef int32_t CmnRet_t;


/******************************************************************************/
#endif
