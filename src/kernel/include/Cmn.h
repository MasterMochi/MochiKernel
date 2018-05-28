/******************************************************************************/
/* src/kernel/include/Cmn.h                                                   */
/*                                                                 2018/05/28 */
/* Copyright (C) 2017-2018 Mochi.                                             */
/******************************************************************************/
#ifndef CMN_H
#define CMN_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdint.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* 処理結果 */
#define CMN_SUCCESS              (  0 )     /** 成功 */
#define CMN_FAILURE              ( -1 )     /** 失敗 */

/* 使用フラグ */
#define CMN_UNUSED               ( 0 )      /** 未使用 */
#define CMN_USED                 ( 1 )      /** 使用中 */

/* モジュール・サブモジュール識別子 */
#define CMN_MODULE_INIT_INIT     ( 0x0101 ) /** 初期化制御(初期化)               */
#define CMN_MODULE_DEBUG_INIT    ( 0x0201 ) /** デバッグ制御(初期化)             */
#define CMN_MODULE_DEBUG_LOG     ( 0x0202 ) /** デバッグ制御(ログ管理)           */
#define CMN_MODULE_MEMMNG_INIT   ( 0x0301 ) /** メモリ管理(初期化)               */
#define CMN_MODULE_MEMMNG_GDT    ( 0x0302 ) /** メモリ管理(GDT管理)              */
#define CMN_MODULE_MEMMNG_AREA   ( 0x0303 ) /** メモリ管理(メモリ領域管理)       */
#define CMN_MODULE_MEMMNG_PAGE   ( 0x0304 ) /** メモリ管理(ページ管理)           */
#define CMN_MODULE_MEMMNG_CTRL   ( 0x0305 ) /** メモリ管理(メモリ制御)           */
#define CMN_MODULE_TASKMNG_INIT  ( 0x0401 ) /** タスク管理(初期化)               */
#define CMN_MODULE_TASKMNG_TSS   ( 0x0402 ) /** タスク管理(TSS管理)              */
#define CMN_MODULE_TASKMNG_SCHED ( 0x0403 ) /** タスク管理(スケジューラ)         */
#define CMN_MODULE_TASKMNG_TASK  ( 0x0404 ) /** タスク管理(タスク管理)           */
#define CMN_MODULE_TASKMNG_ELF   ( 0x0405 ) /** タスク管理(ELFローダ)            */
#define CMN_MODULE_TASKMNG_PROC  ( 0x0406 ) /** タスク管理(プロセス管理)         */
#define CMN_MODULE_INTMNG_INIT   ( 0x0501 ) /** 割込み管理(初期化)               */
#define CMN_MODULE_INTMNG_PIC    ( 0x0502 ) /** 割込み管理(PIC管理)              */
#define CMN_MODULE_INTMNG_IDT    ( 0x0503 ) /** 割込み管理(IDT管理)              */
#define CMN_MODULE_INTMNG_HDL    ( 0x0504 ) /** 割込み管理(ハンドラ管理)         */
#define CMN_MODULE_TIMERMNG_INIT ( 0x0601 ) /** タイマ管理(初期化)               */
#define CMN_MODULE_TIMERMNG_PIT  ( 0x0602 ) /** タイマ管理(PIT管理)              */
#define CMN_MODULE_ITCCTRL_INIT  ( 0x0701 ) /** タスク間通信制御(初期化)         */
#define CMN_MODULE_ITCCTRL_MSG   ( 0x0702 ) /** タスク間通信制御(メッセージ制御) */
#define CMN_MODULE_IOCTRL_INIT   ( 0x0801 ) /** 入出力制御(初期化)               */
#define CMN_MODULE_IOCTRL_PORT   ( 0x0802 ) /** 入出力制御(I/Oポート制御)        */

/** モジュール・サブモジュール数 */
#define CMN_MODULE_NUM           ( 24 )

/* 処理結果構造体 */
typedef int32_t CmnRet_t;


/******************************************************************************/
#endif
