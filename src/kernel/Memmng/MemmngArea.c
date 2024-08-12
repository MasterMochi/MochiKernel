/******************************************************************************/
/*                                                                            */
/* src/kernel/Memmng/MemmngArea.c                                             */
/*                                                                 2024/08/11 */
/* Copyright (C) 2017-2024 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>
#include <stdbool.h>

/* ライブラリヘッダ */
#include <MLib/MLibList.h>
#include <MLib/MLibUtil.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Memmng.h>

/* 内部モジュールヘッダ */
#include "MemmngArea.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* モジュールID */
#define _MODULE_ID_ CMN_MODULE_MEMMNG_AREA


/******************************************************************************/
/* ローカル関数プロトタイプ宣言                                               */
/******************************************************************************/
/* メモリ領域追加(内部) */
static void Add( MLibList_t *pAddList,
                 MLibList_t *pUnusedList,
                 AreaInfo_t *pPrev,
                 AreaInfo_t *pNext,
                 void       *pAddr,
                 size_t     size,
                 bool       merge         );

/* メモリ領域割当(内部) */
static void *Alloc( MLibList_t *pAllocList,
                    MLibList_t *pFreeList,
                    MLibList_t *pUnusedList,
                    AreaInfo_t *pFree,
                    size_t     size          );

/* 指定メモリ領域割当(内部) */
static void *AllocSpec( MLibList_t *pAllocList,
                        MLibList_t *pFreeList,
                        MLibList_t *pUnusedList,
                        AreaInfo_t *pFree,
                        void       *pAddr,
                        size_t     size          );


/******************************************************************************/
/* 内部モジュール向けグローバル関数定義                                       */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メモリ領域追加
 * @details     未使用リンクリストからブロック管理情報を取り出して、追加するメ
 *              モリ領域のアドレスとサイズを設定し、追加先リンクリストに追加す
 *              る。リンクリスト内に追加したメモリ領域と隣接するメモリ領域があ
 *              り、結合方法が結合する指定の場合は結合する。
 *
 * @param[in]   *pAddList    追加先リンクリスト
 * @param[in]   *pUnusedList 未使用リンクリスト
 * @param[in]   *pAddr       追加領域アドレス
 * @param[in]   size         追加領域サイズ
 * @param[in]   merge        結合方法
 *                  - true  結合する
 *                  - false 結合しない
 */
/******************************************************************************/
void AreaAdd( MLibList_t *pAddList,
              MLibList_t *pUnusedList,
              void       *pAddr,
              size_t     size,
              bool       merge         )
{
    AreaInfo_t *pPrev;  /* 前エントリ */
    AreaInfo_t *pNext;  /* 後エントリ */

    /* 初期化 */
    pPrev = NULL;
    pNext = NULL;

    do {
        /* 前エントリ更新 */
        pPrev = pNext;

        /* 次エントリ取得 */
        pNext = ( AreaInfo_t * )
                MLibListGetNextNode( pAddList, ( MLibListNode_t * ) pPrev );

        /* 取得結果判定 */
        if ( pNext == NULL ) {
            /* エントリ無し */
            break;
        }

    /* エントリ判定 */
    } while ( !( pAddr < pNext->pAddr ) );

    /* 前後エントリ間に追加 */
    Add( pAddList, pUnusedList, pPrev, pNext, pAddr, size, merge );

    return;
}


/******************************************************************************/
/**
 * @brief       メモリ領域割当
 * @details     未割当リンクリストにて管理されるメモリ領域からsize分のメモリ領
 *              域を割り当てて割当済リンクリストにブロック管理情報を追加する。
 *              新しくブロック管理情報を使用する場合は、未使用リンクリストから
 *              取り出して使用する。
 *
 * @param[in]   *pAllocList  割当済リンクリスト
 * @param[in]   *pFreeList   未割当リンクリスト
 * @param[in]   *pUnusedList 未使用リンクリスト
 * @param[in]   size         割当サイズ
 *
 * @return      割当先メモリ領域アドレスを返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功
 */
/******************************************************************************/
void *AreaAlloc( MLibList_t *pAllocList,
                 MLibList_t *pFreeList,
                 MLibList_t *pUnusedList,
                 size_t     size          )
{
    AreaInfo_t *pFree;  /* 未割当エントリ */

    /* 初期化 */
    pFree = NULL;

    /* 先頭未割当エントリ取得 */
    pFree = ( AreaInfo_t * ) MLibListGetNextNode( pFreeList, NULL );

    /* 未割当エントリ有無判定 */
    while ( pFree != NULL ) {

        /* エントリサイズ判定 */
        if ( pFree->size >= size ) {
            /* 未割当エントリの指すメモリ領域が割当サイズを満たす */

            /* 未割当エントリから割当て */
            return Alloc( pAllocList, pFreeList, pUnusedList, pFree, size );
        }

        /* 次未割当エントリ取得 */
        pFree = ( AreaInfo_t * )
                MLibListGetNextNode( pFreeList, &( pFree->node ) );
    }

    return NULL;
}


/******************************************************************************/
/**
 * @brief       指定メモリ領域割当
 * @details     未割当リンクリストにて管理されるメモリ領域からpAddrとsizeのメモ
 *              リ領域を割り当てて、割当済リンクリストにブロック管理情報を追加
 *              する。新しくブロック管理情報を使用する場合は、未使用リンクリス
 *              トから取り出して使用する。
 *
 * @param[in]   *pAllocList  割当済リンクリスト
 * @param[in]   *pFreeList   未割当リンクリスト
 * @param[in]   *pUnusedList 未使用リンクリスト
 * @param[in]   *pAddr       割当アドレス
 * @param[in]   size         割当サイズ
 *
 * @return      割当アドレスを返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功
 */
/******************************************************************************/
void *AreaAllocSpec( MLibList_t *pAllocList,
                     MLibList_t *pFreeList,
                     MLibList_t *pUnusedList,
                     void       *pAddr,
                     size_t     size          )
{
    void       *pEnd;       /* エントリ終端アドレス */
    void       *pEndSpec;   /* 割当領域終端アドレス */
    AreaInfo_t *pFree;      /* 未割当エントリ       */

    /* 初期化 */
    pEnd     = NULL;
    pEndSpec = MLIB_UTIL_ADDR_ADD_SIZE( pAddr, size );
    pFree    = NULL;

    /* 先頭エントリ取得 */
    pFree = ( AreaInfo_t * ) MLibListGetNextNode( pFreeList, NULL );

    while ( pFree != NULL ) {
        /* エントリ終端アドレス計算 */
        pEnd = MLIB_UTIL_ADDR_ADD_SIZE( pFree->pAddr, pFree->size );

        /* 割当て可否判定 */
        if ( ( pFree->pAddr <= pAddr ) &&
             ( pEndSpec     <= pEnd  )    ) {
            /* 可能 */

            /* エントリから割当て */
            return AllocSpec( pAllocList,
                              pFreeList,
                              pUnusedList,
                              pFree,
                              pAddr,
                              size         );
        }

        /* 次エントリ取得 */
        pFree = ( AreaInfo_t * )
                MLibListGetNextNode( pFreeList, &( pFree->node ) );
    }

    return NULL;
}


/******************************************************************************/
/**
 * @brief       メモリ領域解放
 * @details     割当済リンクリスト内からpAddrと等しい先頭アドレスのメモリ領域を
 *              管理するエントリを削除し、未割当リンクリストに追加する。
 *
 * @param[in]   *pAllocList  割当済リンクリスト
 * @param[in]   *pFreeList   未割当リンクリスト
 * @param[in]   *pUnusedList 未使用リンクリスト
 * @param[in]   *pAddr       解放領域アドレス
 *
 * @return      メモリ領域の解放結果を返す。
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
CmnRet_t AreaFree( MLibList_t *pAllocList,
                   MLibList_t *pFreeList,
                   MLibList_t *pUnusedList,
                   void       *pAddr        )
{
    size_t     size;        /* 解放領域サイズ */
    AreaInfo_t *pAlloc;     /* 割当済エントリ */

    /* 初期化 */
    size   = 0;
    pAlloc = NULL;

    /* 先頭割当済エントリ取得 */
    pAlloc = ( AreaInfo_t * ) MLibListGetNextNode( pAllocList, NULL );

    /* 割当済エントリ有無判定 */
    while ( pAlloc != NULL ) {
        /* 解放領域アドレス判定 */
        if ( pAddr == pAlloc->pAddr ) {
            /* 一致 */

            /* 解放領域サイズ取得 */
            size = pAlloc->size;

            /* 割当済エントリ削除 */
            MLibListRemove( pAllocList, &( pAlloc->node ) );

            /* 未使用リンクリスト末尾挿入 */
            MLibListInsertTail( pUnusedList, &( pAlloc->node ) );

            /* 未割当メモリ領域追加 */
            AreaAdd( pFreeList, pUnusedList, pAddr, size, true );

            return CMN_SUCCESS;
        }

        /* 次エントリ取得 */
        pAlloc = ( AreaInfo_t * )
                 MLibListGetNextNode( pAllocList, &( pAlloc->node ) );
    }

    return CMN_FAILURE;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メモリ領域追加(内部)
 * @details     追加先リンクリスト内の前後エントリの間にメモリ領域を追加する。
 *              前後エントリのメモリ領域と隣接し、結合方法を結合するに指定した
 *              場合は結合する。
 *
 * @param[in]   *pAddList    追加先リンクリスト
 * @param[in]   *pUnusedList 未使用リンクリスト
 * @param[in]   *pPrev       前エントリ
 * @param[in]   *pNext       後エントリ
 * @param[in]   *pAddr       追加領域アドレス
 * @param[in]   size         追加領域サイズ
 * @param[in]   merge        結合方法
 *                  - true  結合する
 *                  - false 結合しない
 */
/******************************************************************************/
static void Add( MLibList_t *pAddList,
                 MLibList_t *pUnusedList,
                 AreaInfo_t *pPrev,
                 AreaInfo_t *pNext,
                 void       *pAddr,
                 size_t     size,
                 bool       merge         )
{
    void       *pTmp;   /* 一時的アドレス */
    AreaInfo_t *pEdit;  /* 編集エントリ   */

    /* 初期化 */
    pTmp  = NULL;
    pEdit = NULL;

    /* 前エントリ有無判定 */
    if ( pPrev != NULL ) {
        /* 前エントリ有り */

        /* 前エントリ最終アドレス計算 */
        pTmp = MLIB_UTIL_ADDR_ADD_SIZE( pPrev->pAddr, pPrev->size );
    }

    /* 前エントリ隣接判定 */
    if ( ( pPrev != NULL  ) &&
         ( pTmp  == pAddr ) &&
         ( merge != false )    ) {
        /* 隣接かつ結合指定 */

        /* 前エントリ編集 */
        pEdit        = pPrev;
        pEdit->size += size;

    } else {
        /* 隣接しないまたは結合しない */

        /* 未使用エントリ取得 */
        pEdit = ( AreaInfo_t * ) MLibListRemoveTail( pUnusedList );

        /* 未使用エントリ編集 */
        pEdit->pAddr = pAddr;
        pEdit->size  = size;

        /* 後エントリ前挿入 */
        MLibListInsertPrev( pAddList, ( MLibListNode_t * ) pNext, &( pEdit->node ) );
    }

    /* 後エントリ有無かつ結合指定判定 */
    if ( ( pNext != NULL ) && ( merge != false ) ) {
        /* 有りかつ結合指定 */

        /* 追加領域最終アドレス計算 */
        pTmp = MLIB_UTIL_ADDR_ADD_SIZE( pAddr, size );

        /* 後エントリ隣接判定 */
        if ( pTmp == pNext->pAddr ) {
            /* 隣接 */

            /* ブロック管理情報編集 */
            pEdit->size += pNext->size;

            /* 後エントリ削除 */
            MLibListRemove( pAddList, &( pNext->node ) );

            /* 未使用リンクリスト末尾挿入 */
            MLibListInsertTail( pUnusedList, &( pNext->node ) );
        }
    }

    return;
}


/******************************************************************************/
/**
 * @brief       メモリ領域割当(内部)
 * @details     未割当リンクリスト内のエントリからsize分のメモリ領域を割当てて
 *              割当済リンクリストにブロック管理情報を追加する。新しくブロック
 *              管理情報を使用する場合は、未使用リンクリストから取り出して使用
 *              する。
 *
 * @param[in]   *pAllocList  割当済リンクリスト
 * @param[in]   *pFreeList   未割当リンクリスト
 * @param[in]   *pUnusedList 未使用リンクリスト
 * @param[in]   *pFree       未割当リンクリストエントリ
 * @param[in]   size         割当サイズ
 *
 * @return      割当先メモリ領域アドレスを返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功
 *
 * @note        未割当リンクリストエントリは、割当サイズ以上のメモリ領域である
 *              こと。
 */
/******************************************************************************/
static void *Alloc( MLibList_t *pAllocList,
                    MLibList_t *pFreeList,
                    MLibList_t *pUnusedList,
                    AreaInfo_t *pFree,
                    size_t     size          )
{
    void *pAddr;    /* 割当てアドレス */

    /* 初期化 */
    pAddr = pFree->pAddr;

    /* エントリサイズ判定 */
    if ( size < pFree->size ) {
        /* エントリが指すメモリ領域より小さい */

        /* エントリ編集 */
        pFree->pAddr  = MLIB_UTIL_ADDR_ADD_SIZE( pFree->pAddr, size );
        pFree->size  -= size;

    } else {
        /* エントリが指すメモリ領域と等しい */

        /* 未割当リンクリストから削除 */
        MLibListRemove( pFreeList, &( pFree->node ) );

        /* 未使用リンクリスト末尾挿入 */
        MLibListInsertTail( pUnusedList, &( pFree->node ) );
    }

    /* メモリ領域追加 */
    AreaAdd( pAllocList, pUnusedList, pAddr, size, false );

    return pAddr;
}


/******************************************************************************/
/**
 * @brief       指定メモリ領域割当(内部)
 * @details     未割当リンクリスト内のエントリからpAddrとsizeのメモリ領域を割当
 *              てて、割当済リンクリストにブロック管理情報を追加する。新しくブ
 *              ロック管理情報を使用する場合は、未使用リンクリストから取り出し
 *              て使用する。
 *
 * @param[in]   *pAllocList  割当済リンクリスト
 * @param[in]   *pFreeList   未割当リンクリスト
 * @param[in]   *pUnusedList 未使用リンクリスト
 * @param[in]   *pFree       未割当エントリ
 * @param[in]   *pAddr       割当アドレス
 * @param[in]   size         割当サイズ
 *
 * @return      割当アドレスを返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功
 *
 * @note        割当メモリ領域が未割当エントリのメモリ領域内に含まれること。
 */
/******************************************************************************/
static void *AllocSpec( MLibList_t *pAllocList,
                        MLibList_t *pFreeList,
                        MLibList_t *pUnusedList,
                        AreaInfo_t *pFree,
                        void       *pAddr,
                        size_t     size          )
{
    void       *pEnd;       /* 未割当エントリ終端アドレス */
    void       *pEndSpec;   /* 割当領域終端アドレス       */
    AreaInfo_t *pUnused;    /* 未使用エントリ             */

    /* 初期化 */
    pEnd     = MLIB_UTIL_ADDR_ADD_SIZE( pFree->pAddr, pFree->size );
    pEndSpec = MLIB_UTIL_ADDR_ADD_SIZE( pAddr       , size        );
    pUnused  = NULL;

    /* 割当メモリ領域位置判定 */
    if ( pFree->size == size ) {
        /* 未割当エントリのメモリ領域と割当メモリ領域が等しい */

        /* エントリ削除 */
        MLibListRemove( pFreeList, &( pFree->node ) );

        /* 未使用リンクリスト末尾挿入 */
        MLibListInsertTail( pUnusedList, &( pFree->node ) );

    } else if ( pFree->pAddr == pAddr ) {
        /* 未割当エントリのメモリ領域前方に割当メモリ領域が含まれる */

        /* 未割当エントリ編集 */
        pFree->pAddr  = pEndSpec;
        pFree->size  -= size;

    } else if ( pEnd == pEndSpec ) {
        /* 未割当エントリのメモリ領域後方に割当メモリ領域が含まれる */

        /* 未割当エントリ編集 */
        pFree->size -= size;

    } else {
        /* 未割当エントリのメモリ領域中間に割当メモリ領域が含まれる */

       /* 未使用エントリ取得 */
        pUnused = ( AreaInfo_t * ) MLibListRemoveTail( pUnusedList );

        /* 後方の未割当エントリ編集 */
        pUnused->pAddr = pEndSpec;
        pUnused->size  = ( size_t ) pEnd - ( size_t ) pEndSpec;

        /* 前方の未割当エントリ編集 */
        pFree->size = ( size_t ) pAddr - ( size_t ) pFree->pAddr;

        /* 未割当エントリ次挿入 */
        MLibListInsertNext( pFreeList, &( pFree->node ), &( pUnused->node ) );
    }

    /* メモリ領域追加 */
    AreaAdd( pAllocList, pUnusedList, pAddr, size, false );

    return pAddr;
 }


/******************************************************************************/
