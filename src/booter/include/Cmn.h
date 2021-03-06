/******************************************************************************/
/*                                                                            */
/* src/booter/include/Cmn.h                                                   */
/*                                                                 2019/07/24 */
/* Copyright (C) 2017-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef CMN_H
#define CMN_H
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
/* 処理結果 */
#define CMN_SUCCESS               (  0 )        /**< 成功 */
#define CMN_FAILURE               ( -1 )        /**< 失敗 */

/* モジュール・サブモジュール識別子 */
#define CMN_MODULE_INIT_MAIN      ( 0x0101 )    /**< 初期化制御(メイン)     */
#define CMN_MODULE_INTMNG_MAIN    ( 0x0201 )    /**< 割込管理(メイン)       */
#define CMN_MODULE_INTMNG_PIC     ( 0x0202 )    /**< 割込管理(PIC管理)      */
#define CMN_MODULE_INTMNG_IDT     ( 0x0203 )    /**< 割込管理(IDT管理)      */
#define CMN_MODULE_INTMNG_HDL     ( 0x0204 )    /**< 割込管理(ハンドラ管理) */
#define CMN_MODULE_MEMMNG_MAIN    ( 0x0301 )    /**< メモリ管理(メイン)     */
#define CMN_MODULE_MEMMNG_MAP     ( 0x0302 )    /**< メモリ管理(マップ管理) */
#define CMN_MODULE_DRIVER_MAIN    ( 0x0401 )    /**< ドライバ(メイン)       */
#define CMN_MODULE_DRIVER_A20     ( 0x0402 )    /**< ドライバ(A20)          */
#define CMN_MODULE_DRIVER_ATA     ( 0x0403 )    /**< ドライバ(ATA)          */
#define CMN_MODULE_LOADMNG_MAIN   ( 0x0501 )    /**< 読込管理(メイン)       */
#define CMN_MODULE_LOADMNG_KERNEL ( 0x0502 )    /**< 読込管理(カーネル)     */
#define CMN_MODULE_LOADMNG_PROC   ( 0x0503 )    /**< 読込管理(プロセス)     */
#define CMN_MODULE_DEBUG_MAIN     ( 0x0601 )    /**< デバッグ制御(メイン)   */
#define CMN_MODULE_DEBUG_LOG      ( 0x0602 )    /**< デバッグ制御(ログ管理) */

/** モジュール・サブモジュール数 */
#define CMN_MODULE_NUM            ( 15 )

/** 処理結果構造体 */
typedef int32_t CmnRet_t;


/******************************************************************************/
/* 変数宣言                                                                   */
/******************************************************************************/
/*-----------------------*/
/* BIOS-E820メモリマップ */
/*-----------------------*/
/** BIOS-E820メモリマップエントリ数 */
extern size_t          gBiosE820EntryNum;
/** BIOS-E820メモリマップ */
extern BiosE820Entry_t gBiosE820[];

/*--------------*/
/* メモリマップ */
/*--------------*/
/** メモリマップエントリ数 */
extern size_t          gMemMapEntryNum;
/** メモリマップ */
extern MkMemMapEntry_t gMemMap[];


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
