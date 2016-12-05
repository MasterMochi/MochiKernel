/******************************************************************************/
/* src/kernel/MemMng/MemMngGdt.c                                              */
/*                                                                 2016/12/05 */
/* Copyright (C) 2016 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <cpu/IA32/IA32Descriptor.h>
#include <cpu/IA32/IA32Instruction.h>

/* 外部モジュールヘッダ */
#include <MemMng.h>

/* 内部モジュールヘッダ */


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
static IA32Descriptor_t gGdt[ MEMMNG_GDT_ENTRY_MAX ] = {
    /* GDT#0 Nullセグメント */
    { { 0 } },
    /* GDT#1 カーネルコードセグメント */
    { { 0xFFFF,                             /* セグメントリミット(0-15)       */
        0x0000,                             /* セグメントベース(0-15)         */
        0x00,                               /* セグメントベース(16-23)        */
        IA32_DESCRIPTOR_TYPE_CODE_X,        /* セグメントタイプ               */
        IA32_DESCRIPTOR_S_NO,               /* システムフラグ                 */
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
        IA32_DESCRIPTOR_S_NO,               /* システムフラグ                 */
        IA32_DESCRIPTOR_DPL_0,              /* ディスクリプタ特権レベル       */
        IA32_DESCRIPTOR_P_YES,              /* ディスクリプタ存在フラグ       */
        0xF,                                /* セグメントリミット(16-19)      */
        IA32_DESCRIPTOR_AVL_OFF,            /* 未使用                         */
        0x0,                                /* 予約(0)                        */
        IA32_DESCRIPTOR_DB_32,              /* デフォルトオペレーションサイズ */
        IA32_DESCRIPTOR_G_4K,               /* リミット粒度                   */
        0x00                          } },  /* セグメントベース(24-31)        */
    /* GDT#3 ユーザコードセグメント */
    { { 0xFFFF,                             /* セグメントリミット(0-15)       */
        0x0000,                             /* セグメントベース(0-15)         */
        0x00,                               /* セグメントベース(16-23)        */
        IA32_DESCRIPTOR_TYPE_CODE_X,        /* セグメントタイプ               */
        IA32_DESCRIPTOR_S_NO,               /* システムフラグ                 */
        IA32_DESCRIPTOR_DPL_3,              /* ディスクリプタ特権レベル       */
        IA32_DESCRIPTOR_P_YES,              /* ディスクリプタ存在フラグ       */
        0xF,                                /* セグメントリミット(16-19)      */
        IA32_DESCRIPTOR_AVL_OFF,            /* 未使用                         */
        0x0,                                /* 予約(0)                        */
        IA32_DESCRIPTOR_DB_32,              /* デフォルトオペレーションサイズ */
        IA32_DESCRIPTOR_G_4K,               /* リミット粒度                   */
        0x00                          } },  /* セグメントベース(24-31)        */
    /* GDT#4 ユーザデータセグメント */
    { { 0xFFFF,                             /* セグメントリミット(0-15)       */
        0x0000,                             /* セグメントベース(0-15)         */
        0x00,                               /* セグメントベース(16-23)        */
        IA32_DESCRIPTOR_TYPE_DATA_RW,       /* セグメントタイプ               */
        IA32_DESCRIPTOR_S_NO,               /* システムフラグ                 */
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
    /* GDTR設定 */
    IA32InstructionLgdt( gGdt, sizeof ( gGdt ) - 1 );
    
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
 * @param[in]   sysFlg システムフラグ
 * @param[in]   type   セグメントタイプ
 * @param[in]   level  セグメント特権レベル
 * @param[in]   opSize オペレーションサイズ
 * 
 * @retval      MEMMNG_GDT_NULL      失敗（空きエントリ無し）
 * @retval      MEMMNG_GDT_ENTRY_MIN 成功（GDTエントリ番号最小値）
 * @retval      MEMMNG_GDT_ENTRY_MAX 成功（GDTエントリ番号最大値）
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
    uint16_t            selector;       /* セグメントセレクタ       */
    IA32DescriptorSeg_t *pDescriptor;   /* セグメントディスクリプタ */
    
    /* 空きディスクリプタ検索 */
    for ( selector = MEMMNG_GDT_ENTRY_MIN;
          selector < MEMMNG_GDT_ENTRY_MAX;
          selector++                       ) {
        /* 参照変数設定 */
        pDescriptor = &( gGdt[ selector ].seg );
        
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
            
            return selector;
        }
    }
    
    /* 空き無しによる追加失敗 */
    return MEMMNG_GDT_NULL;
}


/******************************************************************************/
