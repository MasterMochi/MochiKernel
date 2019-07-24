/******************************************************************************/
/*                                                                            */
/* src/kernel/MemMng/MemMngMap.c                                              */
/*                                                                 2019/07/22 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>

/* 内部モジュールヘッダ */
#include "MemMngMap.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_MEMMNG_MAP,  \
                    __LINE__,               \
                    __VA_ARGS__            )
#else
#define DEBUG_LOG( ... )
#endif

/** メモリマップ管理テーブル構造体 */
typedef struct {
    BiosE820Entry_t *pBiosE820;     /**< BIOS-E820メモリマップ           */
    size_t          biosE820Num;    /**< BIOS-E820メモリマップエントリ数 */
    MkMemMapEntry_t *pMemMap;       /**< メモリマップ                    */
    size_t          memMapNum;      /**< メモリマップエントリ数          */
} MapTbl_t;


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** メモリマップ管理テーブル */
static MapTbl_t gMapTbl;


/******************************************************************************/
/* 外部モジュール向けグローバル関数定義                                       */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メモリマップ領域情報取得
 * @details     メモリマップから指定タイプのメモリマップ領域情報を取得する。
 *
 * @param[out]  *pInfo メモリマップ領域情報
 * @param[in]   type   メモリマップ領域種別
 *                  - MK_MEM_TYPE_KERNEL  カーネル領域
 *                  - MK_MEM_TYPE_PROCIMG プロセスイメージ領域
 *
 * @return      取得結果を返す。
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
CmnRet_t MemMngMapGetInfo( MkMemMapEntry_t *pInfo,
                           uint32_t        type    )
{
    uint32_t        index;      /* インデックス         */
    MkMemMapEntry_t *pEntry;    /* メモリマップエントリ */

    /* メモリマップ全エントリ毎に繰り返し */
    for ( index = 0; index < gMapTbl.memMapNum; index++ ) {
        /* メモリマップ参照変数設定 */
        pEntry = &( gMapTbl.pMemMap[ index ] );

        /* メモリ領域タイプ判定 */
        if ( pEntry->type == type ) {
            /* 一致 */

            /* メモリマップ領域情報設定 */
            *pInfo = *pEntry;

            return CMN_SUCCESS;
        }
    }

    return CMN_FAILURE;
}


/******************************************************************************/
/* 内部モジュール向けグローバル関数定義                                       */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メモリマップ管理初期化
 * @details     メモリマップ管理テーブルを初期化する。
 *
 * @param[in]   *pBiosE820  BIOS-E820メモリマップ
 * @param[in]   biosE820Num BIOS-E820メモリマップエントリ数
 * @param[in]   *pMemMap    メモリマップ
 * @param[in]   memMapNum   メモリマップエントリ数
 */
/******************************************************************************/
void MapInit( BiosE820Entry_t *pBiosE820,
              size_t          biosE820Num,
              MkMemMapEntry_t *pMemMap,
              size_t          memMapNum    )
{
    /* メモリマップ管理テーブル */
    gMapTbl.pBiosE820   = pBiosE820;
    gMapTbl.biosE820Num = biosE820Num;
    gMapTbl.pMemMap     = pMemMap;
    gMapTbl.memMapNum   = memMapNum;

    return;
}


/******************************************************************************/
