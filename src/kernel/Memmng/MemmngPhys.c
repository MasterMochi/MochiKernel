/******************************************************************************/
/*                                                                            */
/* src/kernel/Memmng/MemmngPhys.c                                             */
/*                                                                 2021/01/09 */
/* Copyright (C) 2018-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>

/* ライブラリヘッダ */
#include <MLib/MLib.h>
#include <MLib/MLibList.h>
#include <MLib/MLibUtil.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32Paging.h>
#include <kernel/kernel.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>

/* 内部モジュールヘッダ */
#include "MemmngArea.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_MEMMNG_PHYS, \
                    __LINE__,               \
                    __VA_ARGS__             )
#else
#define DEBUG_LOG( ... )
#endif

/** ブロック管理情報数 */
#define AREAINFO_NUM ( 1000 )

/** 物理メモリ領域管理テーブル */
typedef struct {
    MLibList_t allocList;                   /**< 割当済リンクリスト */
    MLibList_t freeList;                    /**< 未割当リンクリスト */
    MLibList_t unusedList;                  /**< 未使用リンクリスト */
    AreaInfo_t areaInfo[ AREAINFO_NUM ];    /**< ブロック管理情報   */
} PhysTbl_t;


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** 物理メモリ領域管理テーブル */
static PhysTbl_t gPhysTbl;


/******************************************************************************/
/* 外部モジュール向けグローバル関数定義                                       */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       物理メモリ領域割当
 * @details     指定サイズを満たす物理メモリ領域を割り当てる。
 *
 * @param[in]   size 割当サイズ
 *
 * @return      割当先の物理メモリアドレスを返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功
 *
 * @note        割当サイズが4Kバイトアライメントでない場合は、割当サイズが4Kバ
 *              イトアライメントに合うよう加算して、物理メモリ領域を割り当てる。
 */
/******************************************************************************/
void *MemmngPhysAlloc( size_t size )
{
    void *pRet; /* 戻り値 */

    /* 初期化 */
    pRet = NULL;

    /* サイズチェック */
    if ( size == 0 ) {
        /* 不正 */

        return NULL;

    } else {
        /* 正常 */

        /* アライメント計算 */
        size = MLIB_UTIL_ALIGN( size, IA32_PAGING_PAGE_SIZE );
    }

    /* メモリ領域割当 */
    pRet = AreaAlloc( &( gPhysTbl.allocList  ),
                      &( gPhysTbl.freeList   ),
                      &( gPhysTbl.unusedList ),
                      size                      );

    return pRet;
}


/******************************************************************************/
/**
 * @brief       物理メモリ領域解放
 * @details     割当中の物理メモリ領域を解放する。
 *
 * @param[in]   *pAddr 解放するメモリアドレス
 *
 * @return      解放結果を返す。
 * @retval      CMN_FAILURE 成功
 * @retval      CMN_SUCCESS 失敗
 */
/******************************************************************************/
CmnRet_t MemmngPhysFree( void *pAddr )
{
    CmnRet_t ret;   /* 戻り値 */

    /* メモリ領域解放 */
    ret = AreaFree( &( gPhysTbl.allocList  ),
                    &( gPhysTbl.freeList   ),
                    &( gPhysTbl.unusedList ),
                    pAddr                     );

    return ret;
}


/******************************************************************************/
/* 内部モジュール向けグローバル関数定義                                       */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       物理メモリ領域管理初期化
 * @details     物理メモリ領域管理テーブルを初期化する。
 *
 * @param[in]   *pMemMap メモリマップ
 * @param[in]   entryNum メモリマップエントリ数
 */
/******************************************************************************/
void PhysInit( MkMemMapEntry_t *pMemMap,
               size_t          entryNum  )
{
    uint32_t idx;   /* インデックス */

    /* 初期化 */
    idx = 0;

    /* 割当済リンクリスト初期化 */
    MLibListInit( &( gPhysTbl.allocList ) );

    /* 未割当リンクリスト初期化 */
    MLibListInit( &( gPhysTbl.freeList ) );

    /* 未使用リンクリスト初期化 */
    MLibListInit( &( gPhysTbl.unusedList ) );

    /* ブロック管理情報毎に繰り返す */
    for ( idx = 0; idx < AREAINFO_NUM; idx++ ) {
        /* 未使用リンクリスト追加 */
        MLibListInsertTail( &( gPhysTbl.unusedList           ),
                            &( gPhysTbl.areaInfo[ idx ].node )  );
    }

    /* メモリマップエントリ毎に繰り返し */
    for ( idx = 0; idx < entryNum; idx++ ) {
        /* メモリ領域タイプ判定 */
        if ( pMemMap[ idx ].type == MK_MEM_TYPE_AVAILABLE ) {
            /* 使用可能メモリ領域 */

            /* 未割当メモリ領域追加 */
            AreaAdd( &( gPhysTbl.freeList   ),
                     &( gPhysTbl.unusedList ),
                     pMemMap[ idx ].pAddr,
                     pMemMap[ idx ].size,
                     true                      );
        }
    }

    return;
}


/******************************************************************************/
