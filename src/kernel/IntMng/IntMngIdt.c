/******************************************************************************/
/* src/kernel/IntMng/IntMngIdt.c                                              */
/*                                                                 2016/12/13 */
/* Copyright (C) 2016 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <string.h>
#include <hardware/IA32/IA32Descriptor.h>
#include <hardware/IA32/IA32Instruction.h>


/* 外部モジュールヘッダ */
#include <IntMng.h>

/* 内部モジュールヘッダ */
#include "IntMngIdt.h"


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/* IDT */
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
void IntMngIdtInit( void )
{
    /* IDT初期化 */
    memset( gIdt, 0, sizeof ( gIdt ) );
    
    /* IDTR設定 */
    IA32InstructionLidt( ( IA32Descriptor_t * ) gIdt, sizeof ( gIdt ) - 1 );
    
    return;
}


/******************************************************************************/
/**
 * @brief       IDTエントリ設定
 * @details     IDTにエントリを設定する。
 * 
 * @param[in]   entryNo  IDTエントリ番号
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
void IntMngIdtSet( uint32_t entryNo,
                   uint16_t selector,
                   void     *pOffset,
                   uint8_t  count,
                   uint8_t  type,
                   uint8_t  level )
{
    /* ディスクリプタ設定 */
    gIdt[ entryNo ].offset_low  = IA32_DESCRIPTOR_OFFSET_LOW( pOffset );
    gIdt[ entryNo ].selector    = selector;
    gIdt[ entryNo ].count       = count;
    gIdt[ entryNo ].attr_type   = type;
    gIdt[ entryNo ].attr_s      = IA32_DESCRIPTOR_S_SYSTEM;
    gIdt[ entryNo ].attr_dpl    = level;
    gIdt[ entryNo ].attr_p      = IA32_DESCRIPTOR_P_YES;
    gIdt[ entryNo ].offset_high = IA32_DESCRIPTOR_OFFSET_HIGH( pOffset );
    
    return;
}


/******************************************************************************/
