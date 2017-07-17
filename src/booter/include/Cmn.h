/******************************************************************************/
/* src/booter/include/Cmn.h                                                   */
/*                                                                 2017/07/04 */
/* Copyright (C) 2017 Mochi.                                                  */
/******************************************************************************/
#ifndef CMN_H
#define CMN_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdbool.h>
#include <stdint.h>
#include <hardware/IA32/IA32Instruction.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* 処理結果 */
#define CMN_SUCCESS             (  0 )      /** 成功 */
#define CMN_FAILURE             ( -1 )      /** 失敗 */

/* モジュール・サブモジュール識別子 */
#define CMN_MODULE_INIT_INIT      ( 0x0101 )  /** 初期化制御(初期化)     */
#define CMN_MODULE_INTMNG_INIT    ( 0x0201 )  /** 割込管理(初期化)       */
#define CMN_MODULE_INTMNG_PIC     ( 0x0202 )  /** 割込管理(PIC管理)      */
#define CMN_MODULE_INTMNG_IDT     ( 0x0203 )  /** 割込管理(IDT管理)      */
#define CMN_MODULE_INTMNG_HDL     ( 0x0204 )  /** 割込管理(ハンドラ管理) */
#define CMN_MODULE_DRIVER_INIT    ( 0x0301 )  /** ドライバ(初期化)       */
#define CMN_MODULE_DRIVER_A20     ( 0x0302 )  /** ドライバ(A20)          */
#define CMN_MODULE_DRIVER_ATA     ( 0x0303 )  /** ドライバ(ATA)          */
#define CMN_MODULE_LOADMNG_INIT   ( 0x0401 )  /** 読込管理(初期化)       */
#define CMN_MODULE_LOADMNG_KERNEL ( 0x0402 )  /** 読込管理(カーネル)     */
#define CMN_MODULE_DEBUG_INIT     ( 0x0401 )  /** デバッグ制御(初期化)   */
#define CMN_MODULE_DEBUG_LOG      ( 0x0402 )  /** デバッグ制御(ログ管理) */

/** モジュール・サブモジュール数 */
#define CMN_MODULE_NUM          ( 12 )

/** 処理結果構造体 */
typedef int32_t CmnRet_t;


/******************************************************************************/
/* インライン関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       アボート
 * @details     無限ループする。
 */
/******************************************************************************/
static inline void CmnAbort( void )
{
    /* 割込み禁止 */
    IA32InstructionCli();
    
    /* 無限ループ */
    while ( true ) {
        IA32InstructionHlt();
    }
}


/******************************************************************************/
#endif