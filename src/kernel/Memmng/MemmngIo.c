/******************************************************************************/
/*                                                                            */
/* src/kernel/Memmng/MemmngIo.c                                               */
/*                                                                 2024/06/02 */
/* Copyright (C) 2018-2024 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>

/* ライブラリヘッダ */
#include <MLib/MLibDynamicArray.h>
#include <MLib/MLibList.h>

/* 共通ヘッダ */
#include <kernel/kernel.h>
#include <hardware/IA32/IA32Paging.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>

/* 内部モジュールヘッダ */
#include "MemmngArea.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* モジュールID */
#define _MODULE_ID_ CMN_MODULE_MEMMNG_IO

/** 動的配列エントリ内ブロック管理情報数 */
#define AREAINFO_NUM             ( 256 )
/** 動的配列エントリサイズ */
#define DYNAMIC_ARRAY_ENTRY_SIZE ( sizeof ( AreaInfo_t ) * AREAINFO_NUM )
/** 動的配列最大エントリ数 */
#define DYNAMIC_ARRAY_ENTRY_MAX  ( 4096 )

/** I/Oメモリ領域管理テーブル */
typedef struct {
    MLibList_t         allocList;   /**< 割当済リンクリスト */
    MLibList_t         freeList;    /**< 未割当リンクリスト */
    MLibList_t         unusedList;  /**< 未使用リンクリスト */
    MLibDynamicArray_t array;       /**< 動的配列ハンドル   */
} IoTbl_t;


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* 未使用ブロック管理情報割当て */
static void AllocUnusedAreaInfo( void );


/******************************************************************************/
/* グローバル変数定義                                                         */
/******************************************************************************/
/** I/Oメモリ領域管理テーブル */
static IoTbl_t gIoTbl;


/******************************************************************************/
/* 外部モジュール向けグローバル関数定義                                       */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       I/Oメモリ領域割当
 * @details     指定I/Oメモリ領域を割り当てる。
 *
 * @param[in]   *pAddr I/Oメモリ領域先頭アドレス
 * @param[in]   size   I/Oメモリ領域サイズ
 *
 * @return      割当先のI/Oメモリアドレスを返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功
 *
 * @note        先頭アドレスと割当サイズが4Kバイトアライメントでない場合は、割
 *              り当てを行わない。
 */
/******************************************************************************/
void *MemmngIoAlloc( void   *pAddr,
                     size_t size    )
{
    void *pRet; /* 戻り値 */

    /* 初期化 */
    pRet = NULL;

    /* サイズチェック */
    if ( size == 0 ) {
        /* 不正 */

        return NULL;
    }

    /* アライメントチェック */
    if ( ( ( ( uint32_t ) pAddr % IA32_PAGING_PAGE_SIZE ) != 0 ) ||
         ( (              size  % IA32_PAGING_PAGE_SIZE ) != 0 )    ) {
        /* 4KBアライメント不正 */

        return NULL;
    }

    /* 未使用リンクリストエントリ数判定 */
    if ( gIoTbl.unusedList.size < 5 ) {
        /* 5未満 */

        /* 未使用ブロック管理情報割当て */
        AllocUnusedAreaInfo();
    }

    /* メモリ領域割当 */
    pRet = AreaAllocSpec( &( gIoTbl.allocList  ),
                          &( gIoTbl.freeList   ),
                          &( gIoTbl.unusedList ),
                          pAddr,
                          size                    );

    return pRet;
}


/******************************************************************************/
/**
 * @brief       I/Oメモリ領域解放
 * @details     割当中のI/Oメモリ領域を解放する。
 *
 * @param[in]   *pAddr 解放するメモリアドレス
 *
 * @return      解放結果を返す。
 * @retval      CMN_FAILURE 失敗
 * @retval      CMN_SUCCESS 成功
 */
/******************************************************************************/
CmnRet_t MemmngIoFree( void *pAddr )
{
    CmnRet_t ret;   /* 戻り値 */

    /* メモリ領域解放 */
    ret = AreaFree( &( gIoTbl.allocList  ),
                    &( gIoTbl.freeList   ),
                    &( gIoTbl.unusedList ),
                    pAddr                   );

    return ret;
}


/******************************************************************************/
/* 内部モジュール向けグローバル関数定義                                       */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       I/Oメモリ領域管理初期化
 * @details     I/Oメモリ領域管理テーブルを初期化する。
 *
 * @param[in]   *pMemMap メモリマップ
 * @param[in]   entryNum メモリマップエントリ数
 */
/******************************************************************************/
void IoInit( MkMemMapEntry_t *pMemMap,
             size_t          entryNum  )
{
    uint32_t  idx;      /* インデックス */
    MLibRet_t retMLib;  /* MLib戻り値   */

    /* 初期化 */
    idx       = 0;
    retMLib   = MLIB_RET_FAILURE;

    /* 割当済リンクリスト初期化 */
    MLibListInit( &( gIoTbl.allocList ) );

    /* 未割当リンクリスト初期化 */
    MLibListInit( &( gIoTbl.freeList ) );

    /* 未使用リンクリスト初期化 */
    MLibListInit( &( gIoTbl.unusedList ) );

    /* 動的配列初期化 */
    retMLib = MLibDynamicArrayInit(
                  &( gIoTbl.array ),
                  1,
                  DYNAMIC_ARRAY_ENTRY_SIZE,
                  DYNAMIC_ARRAY_ENTRY_MAX,
                  NULL                      );

    /* 配列初期化結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        /* [TODO]アボート */
    }

    /* 未使用ブロック管理情報割当て */
    AllocUnusedAreaInfo();

    /* メモリマップエントリ毎に繰り返し */
    for ( idx = 0; idx < entryNum; idx++ ) {
        /* メモリ領域タイプ判定 */
        if ( pMemMap[ idx ].type == MK_MEM_TYPE_RESERVED ) {
            /* 予約済み領域 */

            /* 未割当メモリ領域追加 */
            AreaAdd( &( gIoTbl.freeList   ),
                     &( gIoTbl.unusedList ),
                     pMemMap[ idx ].pAddr,
                     pMemMap[ idx ].size,
                     true                    );
        }
    }

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       未使用ブロック管理情報割当て
 * @details     動的配列エントリ割当てを行い、ブロック管理情報をヒープ領域から
 *              割り当てる。割り当てたブロック管理情報は未使用リンクリストに追
 *              加する。
 */
/******************************************************************************/
static void AllocUnusedAreaInfo( void )
{
    uint_t     dummy;       /* ダミー変数       */
    uint32_t   idx;         /* インデックス     */
    MLibRet_t  retMLib;     /* MLib戻り値       */
    AreaInfo_t *pAreaInfo;  /* ブロック管理情報 */

    /* 初期化 */
    idx       = 0;
    retMLib   = MLIB_RET_FAILURE;
    pAreaInfo = NULL;

    /* ブロック管理情報割当て */
    retMLib = MLibDynamicArrayAlloc(
                  &( gIoTbl.array ),
                  &dummy,
                  ( void ** ) &pAreaInfo,
                  NULL                    );

    /* 割当て結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        return;
    }

    /* ブロック管理情報毎に繰り返す */
    for ( idx = 0; idx < AREAINFO_NUM; idx++ ) {
        /* 未使用リンクリスト追加 */
        MLibListInsertTail( &( gIoTbl.unusedList ),
                            &( pAreaInfo[ idx ].node ) );
    }

    return;
}


/******************************************************************************/
