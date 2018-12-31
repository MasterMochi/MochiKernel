/******************************************************************************/
/* src/libraries/libc/stdlib/malloc.c                                         */
/*                                                                 2018/12/31 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* ライブラリヘッダ */
#include <MLib/MLibList.h>

/* モジュール外ヘッダ */
#include <kernel/proc.h>

/* モジュール内ヘッダ */
#include "malloc.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/


/******************************************************************************/
/* グローバル変数定義                                                         */
/******************************************************************************/
/** ブレイクポイント */
void *pgBreakPoint = NULL;
/** 未使用メモリ領域リスト */
MLibList_t gFreeList;
/** 使用中メモリ領域リスト */
MLibList_t gUsedList;


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* メモリ領域割当(未使用メモリ領域) */
static void *AllocFromFreeArea( mallocArea_t *pArea,
                                size_t       size    );
/* メモリ領域割当(ヒープ領域) */
static void *AllocFromNewHeap( size_t size );


/******************************************************************************/
/* モジュール外向けグローバル関数定義                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       malloc
 * @details     標準Cライブラリのmalloc関数。指定したサイズのメモリ領域をヒープ
 *              メモリから割り当てる。
 *
 * @param[in]   size サイズ
 *
 * @return      割当結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功(割り当てたメモリアドレス)
 */
/******************************************************************************/
void *malloc( size_t size )
{
    bool         retBool;   /* 初期化結果 */
    mallocArea_t *pArea;    /* メモリ領域 */

    /* 初期化 */
    pArea = NULL;

    /* malloc管理初期化 */
    retBool = MallocInit();

    /* 初期化結果判定 */
    if ( retBool == false ) {
        /* 失敗 */

        return NULL;
    }

    /* サイズチェック */
    if ( size == 0 ) {
        /* サイズ0 */

        return NULL;
    }

    /* サイズアライメント */
    size = MLIB_ALIGN( size, 4 );

    while ( true ) {
        /* 未使用メモリ領域取得 */
        pArea = ( mallocArea_t * ) MLibListGetNextNode( &gFreeList,
                                                        &( pArea->node ) );

        /* 取得結果判定 */
        if ( pArea == NULL ) {
            /* 未使用メモリ領域無し */

            /* ヒープ領域を拡張して割り当て */
            return AllocFromNewHeap( size );
        }

        /* 未使用メモリ領域サイズ判定 */
        if ( pArea->size >= size ) {
            /* */

            /* 未使用メモリ領域から割り当て */
            return AllocFromFreeArea( pArea, size );
        }
    }

    return NULL;
}


/******************************************************************************/
/* モジュール内向けグローバル関数定義                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       malloc初期化
 * @details     mallocの初期化を行う。
 *
 * @return      初期化結果を返す。
 * @retval      false 失敗
 * @retval      true  成功
 */
/******************************************************************************/
bool MallocInit( void )
{
    MLibRet_t retMLib;  /* MLib関数戻り値 */

    /* 初期化判定 */
    if ( pgBreakPoint != NULL ) {
        /* 初期化済み */

        return true;
    }

    /* ブレイクポイント取得 */
    pgBreakPoint = MkProcSetBreakPoint( 0, NULL );

    /* 取得結果判定 */
    if ( pgBreakPoint == NULL ) {
        /* 失敗 */

        return false;
    }

    /* 未使用メモリ領域リスト初期化 */
    retMLib = MLibListInit( &gFreeList );

    /* 初期化結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */

        return false;
    }

    /* 使用中メモリ領域リスト初期化 */
    retMLib = MLibListInit( &gUsedList );

    /* 初期化結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */

        return false;
    }

    return true;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メモリ領域割当(未使用メモリ領域)
 * @details     未使用メモリ領域から必要なサイズのメモリ領域を割り当てる。
 *
 * @param[in]   pArea 未使用メモリ領域
 * @param[in]   size  割当サイズ
 *
 * @return      メモリ領域割当結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功(割当メモリ領域アドレス)
 */
/******************************************************************************/
static void *AllocFromFreeArea( mallocArea_t *pArea,
                                size_t       size    )
{
    mallocArea_t *pFreeArea;    /* 未使用メモリ領域 */

    /* 未使用メモリ領域リストから削除 */
    MLibListRemove( &gFreeList, &( pArea->node ) );

    /* 分割判定 */
    if ( ( pArea->size - size ) >= ( sizeof ( mallocArea_t ) + 4 ) ) {
        /* 要分割 */

        /* 分割 */
        pFreeArea = ( mallocArea_t * ) ( ( ( uint32_t ) pArea ) +
                                         sizeof ( mallocArea_t ) +
                                         size                      );

        /* 分割設定 */
        pFreeArea->size = pArea->size - size - sizeof ( mallocArea_t );
        pArea->size     = size;

        /* 未使用メモリ領域リスト追加 */
        MLibListInsertHead( &gFreeList, &( pFreeArea->node ) );
    }

    /* 使用中メモリ領域リストに挿入 */
    MLibListInsertHead( &gUsedList, &( pArea->node ) );

    return pArea->area;
}


/******************************************************************************/
/**
 * @brief       メモリ領域割当(ヒープ領域)
 * @details     ヒープ領域を拡張してメモリ領域を割り当てる。
 *
 * @param[in]   size 割当サイズ
 *
 * @return      メモリ領域割当結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功(割当メモリ領域アドレス)
 */
/******************************************************************************/
static void *AllocFromNewHeap( size_t size )
{
    void         *pNewBreakPoint;   /* 新ブレイクポイント */
    mallocArea_t *pArea;            /* 割当メモリ領域     */

    /* 初期化 */
    pArea = pgBreakPoint;

    /* ブレイクポイント設定 */
    pNewBreakPoint =
        MkProcSetBreakPoint( sizeof ( mallocArea_t ) + size, NULL );

    /* 設定結果判定 */
    if ( pNewBreakPoint == NULL ) {
        /* 失敗 */

        return NULL;
    }

    /* 使用中メモリ領域リストに挿入 */
    MLibListInsertHead( &gUsedList, &( pArea->node ) );

    /* ブレイクポイント設定 */
    pgBreakPoint = pNewBreakPoint;

    /* メモリ領域サイズ設定 */
    pArea->size = size;

    return pArea->area;
}


/******************************************************************************/
