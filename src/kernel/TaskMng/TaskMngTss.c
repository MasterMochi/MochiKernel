/******************************************************************************/
/*                                                                            */
/* src/kernel/TaskMng/TaskMngTss.c                                            */
/*                                                                 2019/07/21 */
/* Copyright (C) 2017-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>
#include <string.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32.h>
#include <hardware/IA32/IA32Descriptor.h>
#include <hardware/IA32/IA32Instruction.h>
#include <hardware/IA32/IA32Tss.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <MemMng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_TASKMNG_TSS, \
                    __LINE__,               \
                    __VA_ARGS__             )
#else
#define DEBUG_LOG( ... )
#endif


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** TSS */
static IA32Tss_t gTss;


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       TSS管理初期化
 * @details     TSS(Task State Segmeng)を初期化、GDT(Global Descriptor Table)に
 *              登録し、そのセグメントセレクタをTRレジスタにロードする。
 */
/******************************************************************************/
void TaskMngTssInit( void )
{
    uint16_t index;     /* GDTエントリ番号    */
    uint16_t selector;  /* セグメントセレクタ */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* TSS初期化 */
    memset( &gTss, 0, sizeof ( IA32Tss_t ) );

    /* TSS設定 */
    gTss.ss0 = MEMMNG_SEGSEL_KERNEL_DATA;

    /* TSSディスクリプタ設定 */
    index = MemMngGdtAdd(
                &gTss,                          /* セグメントベース     */
                sizeof ( IA32Tss_t ) - 1,       /* セグメントサイズ     */
                IA32_DESCRIPTOR_G_BYTE,         /* リミット粒度         */
                IA32_DESCRIPTOR_S_SYSTEM,       /* システムフラグ       */
                IA32_DESCRIPTOR_TYPE_TSS32,     /* セグメントタイプ     */
                IA32_DESCRIPTOR_DPL_0,          /* セグメント特権レベル */
                IA32_DESCRIPTOR_DB_UNUSED   );  /* オペレーションサイズ */

    /* セグメントセレクタ設定 */
    selector = IA32_SEGMENT_SELECTOR( index,            /* インデックス */
                                      IA32_TI_GDT,      /* TI           */
                                      IA32_RPL_0   );   /* RPL          */

    /* TR設定 */
    IA32InstructionLtr( selector );

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/**
 * @brief       ESP0設定
 * @details     TSSのESP0フィールドを設定する。
 *
 * @param[in]   esp0 ESP0設定値
 */
/******************************************************************************/
void TaskMngTssSetEsp0( uint32_t esp0 )
{
    /* esp0設定 */
    gTss.esp0 = esp0;

    return;
}


/******************************************************************************/
