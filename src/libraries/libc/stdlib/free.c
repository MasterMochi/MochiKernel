/******************************************************************************/
/* src/libraries/libc/stdlib/free.c                                           */
/*                                                                 2018/12/31 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stddef.h>
#include <stdint.h>

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
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* 未使用メモリ領域リスト挿入 */
static void InsertFreeList( mallocArea_t *pArea );


/******************************************************************************/
/* モジュール外向けグローバル関数定義                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       free
 * @details     標準Cライブラリのfree関数。malloc関数によって割り当てたメモリ領
 *              域を解放する。
 *
 * @param[in]   *ptr メモリ解放アドレス
 */
/******************************************************************************/
void free( void *ptr )
{
    mallocArea_t *pArea;    /* メモリ領域 */

    /* アドレスチェック */
    if ( ptr == NULL ) {
        /* 不正 */

        return;
    }

    /* 初期化 */
    pArea = ( mallocArea_t * ) ( ptr - offsetof( mallocArea_t, area ) );

    /* 使用中メモリ領域リストから削除 */
    MLibListRemove( &gUsedList, &( pArea->node ) );

    /* ブレイクポイント変更判定 */
    if ( pgBreakPoint == ( ptr + pArea->size ) ) {
        /* ブレイクポイント変更有り */

        /* ブレイクポイント減算 */
        pgBreakPoint = MkProcSetBreakPoint(
                           - ( sizeof ( mallocArea_t ) + pArea->size ), NULL );

    } else {
        /* ブレイクポイント変更無し */

        /* 未使用メモリ領域リストに挿入 */
        InsertFreeList( pArea );
    }

    return;
}


/******************************************************************************/
/* モジュール内向けグローバル関数定義                                         */
/******************************************************************************/


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       未使用メモリ領域リスト挿入
 * @details     未使用メモリ領域を未使用メモリ領域リストの適切な位置に挿入する。
 *              必要があれば前後の未使用メモリ領域と結合する。
 *
 * @param[in]   pArea 未使用メモリ領域
 */
/******************************************************************************/
static void InsertFreeList( mallocArea_t *pArea )
{
    mallocArea_t *pPrevArea;    /* 前メモリ領域 */
    mallocArea_t *pNextArea;    /* 次メモリ領域 */

    /* 初期化 */
    pPrevArea = NULL;
    pNextArea = NULL;

    /* 未使用メモリ領域リスト先頭取得 */
    pPrevArea = ( mallocArea_t * )
                MLibListGetNextNode( &gFreeList, NULL );

    /* 取得結果判定 */
    if ( pPrevArea == NULL ) {
        /* メモリ領域無し */

        /* 最後尾に挿入 */
        MLibListInsertTail( &gFreeList, &( pArea->node ) );
        return;
    }

    while ( true ) {
        /* 次メモリ領域取得 */
        pNextArea = ( mallocArea_t * )
                    MLibListGetNextNode( &gFreeList, &( pPrevArea->node ) );

        /* 後メモリ領域結合判定 */
        if ( ( ( uint32_t ) pArea->area + pArea->size ) ==
             ( ( uint32_t ) pNextArea                 )    ) {
            /* 後方結合 */

            /* 結合 */
            pArea->size += sizeof ( mallocArea_t ) + pNextArea->size;

            /* 後メモリ領域をリストから削除 */
            MLibListRemove( &gFreeList, &( pNextArea->node ) );
        }

        /* 前メモリ領域結合判定 */
        if ( ( ( uint32_t ) pPrevArea->area + pPrevArea->size ) ==
             ( ( uint32_t ) pArea                             )    ) {
            /* 前方結合 */

            /* 結合 */
            pPrevArea->size += sizeof ( mallocArea_t ) + pArea->size;
            break;
        }

        /* 後メモリ領域有無判定 */
        if ( pNextArea == NULL ) {
            /* 無し */

            /* 最後尾に挿入 */
            MLibListInsertTail( &gFreeList, &( pArea->node ) );
            break;

        }

        /* 前後メモリ領域位置判定 */
        if ( ( pPrevArea < pArea ) && ( pArea < pNextArea ) ) {
            /* 中間 */

            /* 挿入 */
            MLibListInsertNext( &gFreeList,
                                &( pPrevArea->node ),
                                &( pArea->node )      );
            break;
        }

        /* 次のメモリ領域 */
        pPrevArea = pNextArea;
    }

    return;
}


/******************************************************************************/

