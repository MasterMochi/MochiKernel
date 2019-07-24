/******************************************************************************/
/*                                                                            */
/* src/kernel/MemMng/MemMngGdt.c                                              */
/*                                                                 2019/07/22 */
/* Copyright (C) 2016-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32Descriptor.h>
#include <hardware/IA32/IA32Instruction.h>

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
    DebugLogOutput( CMN_MODULE_MEMMNG_GDT,  \
                    __LINE__,               \
                    __VA_ARGS__            )
#else
#define DEBUG_LOG( ... )
#endif


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** GDT */
static IA32Descriptor_t gGdt[ MEMMNG_GDT_ENTRY_NUM ] = {
    /* GDT#0 Nullセグメント */
    { { 0 } },
    /* GDT#1 カーネルコードセグメント */
    { { 0xFFFF,                             /* セグメントリミット(0-15)       */
        0x0000,                             /* セグメントベース(0-15)         */
        0x00,                               /* セグメントベース(16-23)        */
        IA32_DESCRIPTOR_TYPE_CODE_X,        /* セグメントタイプ               */
        IA32_DESCRIPTOR_S_CODEDATA,         /* システムフラグ                 */
        IA32_DESCRIPTOR_DPL_0,              /* ディスクリプタ特権レベル       */
        IA32_DESCRIPTOR_P_YES,              /* ディスクリプタ存在フラグ       */
        0xF,                                /* セグメントリミット(16-19)      */
        IA32_DESCRIPTOR_AVL_OFF,            /* 未使用                         */
        0x0,                                /* 予約(0)                        */
        IA32_DESCRIPTOR_DB_32,              /* デフォルトオペレーションサイズ */
        IA32_DESCRIPTOR_G_4K,               /* リミット粒度                   */
        0x00                          } },  /* セグメントベース(24-31)        */
    /* GDT#2 カーネルデータセグメント */
    { { 0xFFFF,                             /* セグメントリミット(0-15)       */
        0x0000,                             /* セグメントベース(0-15)         */
        0x00,                               /* セグメントベース(16-23)        */
        IA32_DESCRIPTOR_TYPE_DATA_RW,       /* セグメントタイプ               */
        IA32_DESCRIPTOR_S_CODEDATA,         /* システムフラグ                 */
        IA32_DESCRIPTOR_DPL_0,              /* ディスクリプタ特権レベル       */
        IA32_DESCRIPTOR_P_YES,              /* ディスクリプタ存在フラグ       */
        0xF,                                /* セグメントリミット(16-19)      */
        IA32_DESCRIPTOR_AVL_OFF,            /* 未使用                         */
        0x0,                                /* 予約(0)                        */
        IA32_DESCRIPTOR_DB_32,              /* デフォルトオペレーションサイズ */
        IA32_DESCRIPTOR_G_4K,               /* リミット粒度                   */
        0x00                          } },  /* セグメントベース(24-31)        */
    /* GDT#3 アプリコードセグメント */
    { { 0xFFFF,                             /* セグメントリミット(0-15)       */
        0x0000,                             /* セグメントベース(0-15)         */
        0x00,                               /* セグメントベース(16-23)        */
        IA32_DESCRIPTOR_TYPE_CODE_X,        /* セグメントタイプ               */
        IA32_DESCRIPTOR_S_CODEDATA,         /* システムフラグ                 */
        IA32_DESCRIPTOR_DPL_3,              /* ディスクリプタ特権レベル       */
        IA32_DESCRIPTOR_P_YES,              /* ディスクリプタ存在フラグ       */
        0xF,                                /* セグメントリミット(16-19)      */
        IA32_DESCRIPTOR_AVL_OFF,            /* 未使用                         */
        0x0,                                /* 予約(0)                        */
        IA32_DESCRIPTOR_DB_32,              /* デフォルトオペレーションサイズ */
        IA32_DESCRIPTOR_G_4K,               /* リミット粒度                   */
        0x00                          } },  /* セグメントベース(24-31)        */
    /* GDT#4 アプリデータセグメント */
    { { 0xFFFF,                             /* セグメントリミット(0-15)       */
        0x0000,                             /* セグメントベース(0-15)         */
        0x00,                               /* セグメントベース(16-23)        */
        IA32_DESCRIPTOR_TYPE_DATA_RW,       /* セグメントタイプ               */
        IA32_DESCRIPTOR_S_CODEDATA,         /* システムフラグ                 */
        IA32_DESCRIPTOR_DPL_3,              /* ディスクリプタ特権レベル       */
        IA32_DESCRIPTOR_P_YES,              /* ディスクリプタ存在フラグ       */
        0xF,                                /* セグメントリミット(16-19)      */
        IA32_DESCRIPTOR_AVL_OFF,            /* 未使用                         */
        0x0,                                /* 予約(0)                        */
        IA32_DESCRIPTOR_DB_32,              /* デフォルトオペレーションサイズ */
        IA32_DESCRIPTOR_G_4K,               /* リミット粒度                   */
        0x00                          } },  /* セグメントベース(24-31)        */
    /* GDT#5以降 空ディスクリプタ */
    { { 0 } }, };


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       GDT初期化
 * @details     GDTを初期化しCPUのGDTRへGDTを設定する。
 */
/******************************************************************************/
void MemMngGdtInit( void )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* GDTR設定 */
    IA32InstructionLgdt( gGdt, sizeof ( gGdt ) - 1 );

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/**
 * @brief       GDTエントリ追加
 * @details     GDTの空きエントリを検索し、エントリをGDTに追加する。
 *
 * @param[in]   *pBase セグメントベース
 * @param[in]   limit  セグメントサイズ
 * @param[in]   limitG リミット粒度
 *                  - IA32_DESCRIPTOR_G_BYTE 1byte単位
 *                  - IA32_DESCRIPTOR_G_4K   4,095byte単位
 * @param[in]   sysFlg システムフラグ
 *                  - IA32_DESCRIPTOR_S_SYSTEM   システムディスクリプタ
 *                  - IA32_DESCRIPTOR_S_CODEDATA コード/データディスクリプタ
 * @param[in]   type   セグメントタイプ
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
 *                  - IA32_DESCRIPTOR_TYPE_DATA_R      データR
 *                  - IA32_DESCRIPTOR_TYPE_DATA_RA     データRA
 *                  - IA32_DESCRIPTOR_TYPE_DATA_RW     データRW
 *                  - IA32_DESCRIPTOR_TYPE_DATA_RWA    データRWA
 *                  - IA32_DESCRIPTOR_TYPE_DATA_ER     データER
 *                  - IA32_DESCRIPTOR_TYPE_DATA_ERA    データERA
 *                  - IA32_DESCRIPTOR_TYPE_DATA_ERW    データER
 *                  - IA32_DESCRIPTOR_TYPE_DATA_ERWA   データERWA
 *                  - IA32_DESCRIPTOR_TYPE_CODE_X      コードX
 *                  - IA32_DESCRIPTOR_TYPE_CODE_XA     コードXA
 *                  - IA32_DESCRIPTOR_TYPE_CODE_XR     コードXR
 *                  - IA32_DESCRIPTOR_TYPE_CODE_XRA    コードXRA
 *                  - IA32_DESCRIPTOR_TYPE_CODE_CX     コードCX
 *                  - IA32_DESCRIPTOR_TYPE_CODE_CXA    コードCXA
 *                  - IA32_DESCRIPTOR_TYPE_CODE_CXR    コードCXR
 *                  - IA32_DESCRIPTOR_TYPE_CODE_CXRA   コードCXRA
 * @param[in]   level  セグメント特権レベル
 *                  - IA32_DESCRIPTOR_DPL_0 特権レベル0
 *                  - IA32_DESCRIPTOR_DPL_1 特権レベル1
 *                  - IA32_DESCRIPTOR_DPL_2 特権レベル2
 *                  - IA32_DESCRIPTOR_DPL_3 特権レベル3
 * @param[in]   opSize オペレーションサイズ
 *                  - IA32_DESCRIPTOR_DB_16 16bit
 *                  - IA32_DESCRIPTOR_DB_32 32bit
 *
 * @return      追加時に割り当てたGDTエントリ番号を返す。
 * @retval      MEMMNG_GDT_ENTRY_FULL 失敗（空きエントリ空き無し）
 * @retval      MEMMNG_GDT_ENTRY_MIN  成功（GDTエントリ番号最小値）
 * @retval      MEMMNG_GDT_ENTRY_MAX  成功（GDTエントリ番号最大値）
 */
/******************************************************************************/
uint16_t MemMngGdtAdd( void    *pBase,
                       size_t  limit,
                       uint8_t limitG,
                       uint8_t sysFlg,
                       uint8_t type,
                       uint8_t level,
                       uint8_t opSize  )
{
    uint16_t            index;          /* GDTエントリ番号          */
    IA32DescriptorSeg_t *pDescriptor;   /* セグメントディスクリプタ */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. pBase=%010p, limit=%u, ",
               __func__,
               pBase,
               limit );
    DEBUG_LOG( " limitG=%u, sysFlg=%u, type=%u, level=%u, opSize=%u",
               limitG,
               sysFlg,
               type,
               level,
               opSize );

    /* 空きディスクリプタ検索 */
    for ( index =  MEMMNG_GDT_ENTRY_MIN;
          index <= MEMMNG_GDT_ENTRY_MAX;
          index++                       ) {
        /* 参照変数設定 */
        pDescriptor = &( gGdt[ index ].seg );

        /* 空きディスクリプタ判定 */
        if ( pDescriptor->attr_p == IA32_DESCRIPTOR_P_NO ) {
            /* 空き */

            /* ディスクリプタ追加 */
            pDescriptor->limit_low     = IA32_DESCRIPTOR_LIMIT_LOW( limit );
            pDescriptor->base_low      = IA32_DESCRIPTOR_BASE_LOW( pBase );
            pDescriptor->base_middle   = IA32_DESCRIPTOR_BASE_MIDDLE( pBase );
            pDescriptor->attr_type     = type;
            pDescriptor->attr_s        = sysFlg;
            pDescriptor->attr_dpl      = level;
            pDescriptor->attr_p        = IA32_DESCRIPTOR_P_YES;
            pDescriptor->limit_high    = IA32_DESCRIPTOR_LIMIT_HIGH( limit );
            pDescriptor->attr_avl      = IA32_DESCRIPTOR_AVL_OFF;
            pDescriptor->attr_reserved = 0;
            pDescriptor->attr_opSize   = opSize;
            pDescriptor->attr_g        = limitG;
            pDescriptor->base_high     = IA32_DESCRIPTOR_BASE_HIGH( pBase );

            /* デバッグトレースログ出力 */
            DEBUG_LOG( "%s() end. ret=%#x", __func__, index );

            return index;
        }
    }

    /* デバッグトレースログ */
    DEBUG_LOG( "%s() end. ret=%#x", __func__, MEMMNG_GDT_ENTRY_FULL );

    /* 空き無しによる追加失敗 */
    return MEMMNG_GDT_ENTRY_FULL;
}


/******************************************************************************/
