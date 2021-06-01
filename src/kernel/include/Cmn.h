/******************************************************************************/
/*                                                                            */
/* src/kernel/include/Cmn.h                                                   */
/*                                                                 2021/05/29 */
/* Copyright (C) 2017-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef CMN_H
#define CMN_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdint.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* 処理結果 */
#define CMN_SUCCESS              (  0 )     /**< 成功 */
#define CMN_FAILURE              ( -1 )     /**< 失敗 */

/* 使用フラグ */
#define CMN_UNUSED               ( 0 )      /**< 未使用 */
#define CMN_USED                 ( 1 )      /**< 使用中 */

/* モジュール・サブモジュール識別子 */
#define CMN_MODULE_INIT_MAIN      ( 0x0101 )/**< 初期化制御(メイン)           */
#define CMN_MODULE_DEBUG_MAIN     ( 0x0201 )/**< デバッグ制御(メイン)         */
#define CMN_MODULE_DEBUG_LOG      ( 0x0202 )/**< デバッグ制御(ログ管理)       */
#define CMN_MODULE_MEMMNG_MAIN    ( 0x0301 )/**< メモリ管理(メイン)           */
#define CMN_MODULE_MEMMNG_SGMT    ( 0x0302 )/**< メモリ管理(セグメント管理)   */
#define CMN_MODULE_MEMMNG_AREA    ( 0x0303 )/**< メモリ管理(領域)             */
#define CMN_MODULE_MEMMNG_PAGE    ( 0x0304 )/**< メモリ管理(ページ)           */
#define CMN_MODULE_MEMMNG_CTRL    ( 0x0305 )/**< メモリ管理(制御)             */
#define CMN_MODULE_MEMMNG_MAP     ( 0x0306 )/**< メモリ管理(マップ)           */
#define CMN_MODULE_MEMMNG_PHYS    ( 0x0307 )/**< メモリ管理(物理)             */
#define CMN_MODULE_MEMMNG_IO      ( 0x0308 )/**< メモリ管理(I/O)              */
#define CMN_MODULE_MEMMNG_VIRT    ( 0x0309 )/**< メモリ管理(仮想)             */
#define CMN_MODULE_MEMMNG_HEAP    ( 0x030A )/**< メモリ管理(ヒープ)           */
#define CMN_MODULE_TASKMNG_MAIN   ( 0x0401 )/**< タスク管理(メイン)           */
#define CMN_MODULE_TASKMNG_TSS    ( 0x0402 )/**< タスク管理(TSS)              */
#define CMN_MODULE_TASKMNG_SCHED  ( 0x0403 )/**< タスク管理(スケジューラ)     */
#define CMN_MODULE_TASKMNG_TASK   ( 0x0404 )/**< タスク管理(タスク)           */
#define CMN_MODULE_TASKMNG_ELF    ( 0x0405 )/**< タスク管理(ELFローダ)        */
#define CMN_MODULE_TASKMNG_PROC   ( 0x0406 )/**< タスク管理(プロセス)         */
#define CMN_MODULE_TASKMNG_NAME   ( 0x0407 )/**< タスク管理(名前管理)         */
#define CMN_MODULE_TASKMNG_THREAD ( 0x0408 )/**< タスク管理(スレッド)         */
#define CMN_MODULE_INTMNG_MAIN    ( 0x0501 )/**< 割込み管理(メイン)           */
#define CMN_MODULE_INTMNG_PIC     ( 0x0502 )/**< 割込み管理(PIC)              */
#define CMN_MODULE_INTMNG_IDT     ( 0x0503 )/**< 割込み管理(IDT)              */
#define CMN_MODULE_INTMNG_HDL     ( 0x0504 )/**< 割込み管理(ハンドラ)         */
#define CMN_MODULE_INTMNG_CTRL    ( 0x0505 )/**< 割込み管理(ハードウェア)     */
#define CMN_MODULE_TIMERMNG_MAIN  ( 0x0601 )/**< タイマ管理(メイン)           */
#define CMN_MODULE_TIMERMNG_CTRL  ( 0x0602 )/**< タイマ管理(制御)             */
#define CMN_MODULE_TIMERMNG_PIT   ( 0x0603 )/**< タイマ管理(PIT)              */
#define CMN_MODULE_ITCCTRL_MAIN   ( 0x0701 )/**< タスク間通信制御(メイン)     */
#define CMN_MODULE_ITCCTRL_MSG    ( 0x0702 )/**< タスク間通信制御(メッセージ) */
#define CMN_MODULE_IOCTRL_MAIN    ( 0x0801 )/**< 入出力制御(メイン)           */
#define CMN_MODULE_IOCTRL_PORT    ( 0x0802 )/**< 入出力制御(ポート)           */
#define CMN_MODULE_IOCTRL_MEM     ( 0x0803 )/**< 入出力制御(I/Oメモリ)        */

/** モジュール・サブモジュール数 */
#define CMN_MODULE_NUM           ( 35 )

/** 処理結果構造体 */
typedef int32_t CmnRet_t;


/******************************************************************************/
#endif
