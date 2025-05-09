/******************************************************************************/
/*                                                                            */
/* src/kernel/Intmng/IntmngIdt.c                                              */
/*                                                                 2024/06/01 */
/* Copyright (C) 2016-2024 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>

/* ライブラリヘッダ */
#include <MLib/MLibUtil.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32Descriptor.h>
#include <hardware/IA32/IA32Instruction.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Intmng.h>

/* 内部モジュールヘッダ */
#include "IntmngIdt.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* モジュールID */
#define _MODULE_ID_ CMN_MODULE_INTMNG_IDT


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** IDT */
static IA32DescriptorGate_t gIdt[ INTMNG_IDT_ENTRY_NUM ];


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       IDT管理初期化
 * @details     IDT管理サブモジュールの初期化を行う。
 */
/******************************************************************************/
void IntmngIdtInit( void )
{
    DEBUG_LOG_TRC( "%s() start.", __func__ );

    /* IDT初期化 */
    MLibUtilSetMemory8( gIdt, 0, sizeof ( gIdt ) );

    /* IDTR設定 */
    IA32InstructionLidt( ( IA32Descriptor_t * ) gIdt, sizeof ( gIdt ) - 1 );

    DEBUG_LOG_TRC( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/**
 * @brief       IDTエントリ設定
 * @details     IDTにエントリを設定する。
 *
 * @param[in]   index    IDTエントリ番号
 *                  - INTMNG_IDT_ENTRY_MIN 割込み番号（最小値）
 *                  - INTMNG_IDT_ENTRY_MAX 割込み番号（最大値）
 * @param[in]   selector セレクタ
 * @param[in]   *pOffset オフセット
 * @param[in]   count    引数コピーカウント
 * @param[in]   type     タイプ
 *                  - IA32_DESCRIPTOR_TYPE_TSS16       16bitTSS
 *                  - IA32_DESCRIPTOR_TYPE_LDT         LDT
 *                  - IA32_DESCRIPTOR_TYPE_TSS16_BUSY  16bitTSS（ビジー）
 *                  - IA32_DESCRIPTOR_TYPE_GATE16_CALL 16bitコールゲート
 *                  - IA32_DESCRIPTOR_TYPE_GATE_TASK   タスクゲート
 *                  - IA32_DESCRIPTOR_TYPE_GATE16_INT  16bit割込みゲート
 *                  - IA32_DESCRIPTOR_TYPE_GATE16_TRAP 16bitトラップゲート
 *                  - IA32_DESCRIPTOR_TYPE_TSS32       32bitTSS
 *                  - IA32_DESCRIPTOR_TYPE_TSS32_BUSY  32bitTSS（ビジー）
 *                  - IA32_DESCRIPTOR_TYPE_GATE32_CALL 32bitコールゲート
 *                  - IA32_DESCRIPTOR_TYPE_GATE32_INT  32bit割込みゲート
 *                  - IA32_DESCRIPTOR_TYPE_GATE32_TRAP 32bitトラップゲート
 * @param[in]   level    特権レベル
 *                  - IA32_DESCRIPTOR_DPL_0 特権レベル0
 *                  - IA32_DESCRIPTOR_DPL_1 特権レベル1
 *                  - IA32_DESCRIPTOR_DPL_2 特権レベル2
 *                  - IA32_DESCRIPTOR_DPL_3 特権レベル3
 */
/******************************************************************************/
void IntmngIdtSet( uint32_t index,
                   uint16_t selector,
                   void     *pOffset,
                   uint8_t  count,
                   uint8_t  type,
                   uint8_t  level )
{
    /* デバッグトレースログ出力 *//*
    DEBUG_LOG_TRC( "%s() start. index=%#x, selector=%u, ",
                   __func__,
                   index,
                   selector );
    DEBUG_LOG_TRC( " pOffset=%010p, count=%u, type=%#x, level=%u",
                   pOffset,
                   count,
                   type,
                   level );*/

    /* ディスクリプタ設定 */
    gIdt[ index ].offset_low  = IA32_DESCRIPTOR_OFFSET_LOW( pOffset );
    gIdt[ index ].selector    = selector;
    gIdt[ index ].count       = count;
    gIdt[ index ].attr_type   = type;
    gIdt[ index ].attr_s      = IA32_DESCRIPTOR_S_SYSTEM;
    gIdt[ index ].attr_dpl    = level;
    gIdt[ index ].attr_p      = IA32_DESCRIPTOR_P_YES;
    gIdt[ index ].offset_high = IA32_DESCRIPTOR_OFFSET_HIGH( pOffset );

    /* デバッグトレースログ出力 *//*
    DEBUG_LOG_TRC( "%s() end.", __func__ );*/

    return;
}


/******************************************************************************/
