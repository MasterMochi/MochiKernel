/******************************************************************************/
/*                                                                            */
/* src/kernel/MemMng/MemMngArea.c                                             */
/*                                                                 2019/06/12 */
/* Copyright (C) 2017-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <MemMng.h>

/* 内部モジュールヘッダ */
#include "MemMngArea.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_MEMMNG_AREA, \
                    __LINE__,               \
                    __VA_ARGS__             )
#else
#define DEBUG_LOG( ... )
#endif

/** メモリ領域終端アドレス計算マクロ */
#define END_ADDR( __TOP, __SIZE ) \
    ( ( void * ) ( ( size_t ) ( __TOP ) + ( __SIZE ) ) )

/** メモリ領域情報数 */
#define AREA_INFO_NUM  ( 1000000 )

/** メモリ領域アライメント */
#define AREA_ALIGNMENT ( 0x1000 )

/** メモリ領域管理テーブル構造体 */
typedef struct {
    MLibList_t emptyList;                   /**< 空メモリ領域情報リスト */
    AreaInfo_t areaInfo[ AREA_INFO_NUM ];   /**< メモリ領域情報         */
} AreaTbl_t;


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** メモリ領域管理テーブル */
static AreaTbl_t gAreaTbl;


/******************************************************************************/
/* ローカル関数プロトタイプ宣言                                               */
/******************************************************************************/
/* メモリ領域割当（指定未割当メモリ領域全て） */
static void *AllocAll( MLibList_t *pAllocList,
                       MLibList_t *pFreeList,
                       AreaInfo_t *pFree       );

/* メモリ領域割当（指定未割当メモリ領域後方） */
static void *AllocBack( MLibList_t *pAllocList,
                        AreaInfo_t *pFree,
                        void       *pAddr,
                        size_t     size         );

/* メモリ領域割当（指定未割当メモリ領域中間） */
static void *AllocCenter( MLibList_t *pAllocList,
                          MLibList_t *pFreeList,
                          AreaInfo_t *pFree,
                          void       *pAddr,
                          size_t     size         );

/* メモリ領域割当（指定未割当メモリ領域前方） */
static void *AllocFront( MLibList_t *pAllocList,
                         AreaInfo_t *pFree,
                         void       *pAddr,
                         size_t     size         );

/* メモリ領域解放（指定未割当メモリ領域結合） */
static CmnRet_t FreeMerge( MLibList_t *pAllocList,
                           MLibList_t *pFreeList,
                           AreaInfo_t *pAlloc,
                           AreaInfo_t *pFree       );

/* メモリ領域解放（指定未割当メモリ領域前挿入） */
static CmnRet_t FreePrev( MLibList_t *pAllocList,
                          MLibList_t *pFreeList,
                          AreaInfo_t *pAlloc,
                          AreaInfo_t *pFree       );

/* メモリ領域解放（最後尾挿入） */
static CmnRet_t FreeTail( MLibList_t *pAllocList,
                          MLibList_t *pFreeList,
                          AreaInfo_t *pAlloc      );



/******************************************************************************/
/* 内部モジュール向けグローバル関数定義                                       */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メモリ領域割当
 * @details     メモリ領域
 *
 * @param[in]   *pAllocList 割当中メモリ領域情報リスト
 * @param[in]   *pFreeList  未割当メモリ領域情報リスト
 * @param[in]   size        割当メモリ領域サイズ
 *
 * @return      メモリ領域の割り当て結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功(割当領域へのアドレス)
 */
/******************************************************************************/
void *AreaAlloc( MLibList_t *pAllocList,
                 MLibList_t *pFreeList,
                 size_t     size         )
{
    void       *pRet;   /* 戻り値               */
    AreaInfo_t *pFree;  /* 未割当メモリ領域情報 */

    /* 初期化 */
    pRet  = NULL;
    pFree = NULL;

    /* 未割当メモリ領域情報毎に繰り返し */
    do {
        /* 未割当メモリ領域情報取得 */
        pFree = ( AreaInfo_t * )
            MLibListGetNextNode( pFreeList,
                                 ( MLibListNode_t * ) pFree );

        /* 取得結果判定 */
        if ( pFree == NULL ) {
            /* 未割当メモリ領域情報無し */

            DEBUG_LOG( "%s(): MLibListGetNextNode() error.", __func__ );

            return NULL;
        }

    /* 割当可能判定 */
    } while ( pFree->size < size );

    /* 未割当メモリ領域サイズ比較 */
    if ( pFree->size == size ) {
        /* 一致 */

        /* メモリ領域割当 */
        pRet = AllocAll( pAllocList, pFreeList, pFree );

    } else {
        /* 過大 */

        /* メモリ領域割当 */
        pRet = AllocFront( pAllocList, pFree, pFree->pAddr, size );
    }

    return pRet;
}


/******************************************************************************/
/**
 * @brief       指定メモリ領域割当
 * @details     メモリ領域
 *
 * @param[in]   *pAllocList 割当中メモリ領域情報リスト
 * @param[in]   *pFreeList  未割当メモリ領域情報リスト
 * @param[in]   *pAddr      割当メモリ領域アドレス
 * @param[in]   size        割当メモリ領域サイズ
 *
 * @return      メモリ領域の割り当て結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功(割当領域へのアドレス)
 */
/******************************************************************************/
void *AreaAllocSpecified( MLibList_t *pAllocList,
                          MLibList_t *pFreeList,
                          void       *pAddr,
                          size_t     size         )
{

    void       *pRet;       /* 戻り値                       */
    void       *pAddr1;     /* 未割当メモリ領域終端アドレス */
    void       *pAddr2;     /* 指定メモリ領域終端アドレス   */
    AreaInfo_t *pFree;      /* 未割当メモリ領域情報         */

    /* 初期化 */
    pRet  = NULL;
    pFree = NULL;

    /* サイズチェック */
    if ( size == 0 ) {
        /* 不正 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=NULL", __func__ );

        return NULL;

    }

    /* 未割当メモリ領域情報取得 */
    pFree = ( AreaInfo_t * ) MLibListGetNextNode( pFreeList, NULL );

    /* 未割当メモリ領域情報毎に繰り返し */
    while ( pFree != NULL ) {
        /* メモリ領域比較用アドレス計算 */
        pAddr1 = END_ADDR( pFree->pAddr, pFree->size );
        pAddr2 = END_ADDR( pAddr,        size        );

        /* 領域チェック */
        if ( ( pAddr < pFree->pAddr                      ) ||
             ( ( pAddr < pAddr1 ) && ( pAddr1 < pAddr2 ) )    ) {
            /* 範囲外 */
            break;
        }

        /* 領域チェック */
        if ( pAddr1 <= pAddr ) {
            /* 次範囲 */

            /* 次メモリ領域情報取得 */
            pFree = ( AreaInfo_t * )
                MLibListGetNextNode( pFreeList,
                                     ( MLibListNode_t * ) pFree );
            continue;
        }

        /* 先頭アドレス比較 */
        if ( pFree->pAddr == pAddr ) {
            /* 一致 */

            /* サイズ比較 */
            if ( pFree->size == size ) {
                /* 一致 */

                /* メモリ領域情報設定 */
                pRet = AllocAll( pAllocList, pFreeList, pFree );

            } else if ( pFree->size > size ) {
                /* 不一致 */

                /* メモリ領域情報設定 */
                pRet = AllocFront( pAllocList, pFree, pAddr, size );
            }

            break;

        } else {
            /* 不一致 */

            /* メモリ領域比較 */
            if ( pAddr1 > pAddr2 ) {
                /* 中間メモリ領域 */

                /* メモリ領域情報設定 */
                pRet = AllocCenter( pAllocList, pFreeList, pFree, pAddr, size );
                break;

            } else if ( pAddr1 == pAddr2 ) {
                /* 後方メモリ領域 */

                /* メモリ領域情報設定 */
                pRet = AllocBack( pAllocList, pFree, pAddr, size );
                break;
            }
        }

        /* 次メモリ領域情報取得 */
        pFree = ( AreaInfo_t * )
            MLibListGetNextNode( pFreeList,
                                 ( MLibListNode_t * ) pFree );
    }

    return pRet;
}


/******************************************************************************/
/**
 * @brief       メモリ領域解放
 * @details     割当中メモリ領域情報リストから指定アドレスに該当するメモリ領域
 *              情報を検索し、割当中メモリ領域情報リストから削除して未割当メモ
 *              リ領域情報リストに挿入する事でメモリ領域を解放する。
 *
 * @param[in]   *pAllocList 割当中メモリ領域情報リスト
 * @param[in]   *pFreeList  未割当メモリ領域情報リスト
 * @param[in]   *pAddr      解放するメモリアドレス
 *
 * @return      メモリ領域の解放結果を返す。
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
CmnRet_t AreaFree( MLibList_t *pAllocList,
                   MLibList_t *pFreeList,
                   void       *pAddr       )
{
    CmnRet_t   ret;         /* 関数戻り値           */
    MLibRet_t  retMLib;     /* MLib関数戻り値       */
    AreaInfo_t *pFree;      /* 未割当メモリ領域情報 */
    AreaInfo_t *pInfo;      /* 割当中メモリ領域情報 */

    /* 初期化 */
    ret      = CMN_FAILURE;
    retMLib  = MLIB_FAILURE;
    pFree    = NULL;
    pInfo    = NULL;

    /* 該当メモリ領域情報検索 */
    do {
        /* 割当中メモリ領域情報取得 */
        pInfo = ( AreaInfo_t * )
            MLibListGetPrevNode( pAllocList,
                                 ( MLibListNode_t * ) pInfo );

        /* 取得結果判定 */
        if ( pInfo == NULL ) {
            /* メモリ領域情報無 */

            return CMN_FAILURE;
        }

        /* 先頭アドレス比較 */
    } while ( pInfo->pAddr != pAddr );

    /* 割当中メモリ領域リストから削除 */
    retMLib = MLibListRemove( pAllocList,
                              ( MLibListNode_t * ) pInfo );

    /* 削除結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */

        return CMN_FAILURE;
    }

    while ( true ) {
        /* 未割当メモリ領域情報取得 */
        pFree = ( AreaInfo_t * )
            MLibListGetNextNode( pFreeList,
                                 ( MLibListNode_t * ) pFree );

        /* メモリ領域位置関係比較 */
        if ( pFree == NULL ) {
            /* メモリ領域情報無 */

            /* 未割当メモリ領域リストの最後尾に挿入 */
            ret = FreeTail( pAllocList, pFreeList, pInfo );

        } else if ( ( pInfo->pAddr + pInfo->size ) < pFree->pAddr ) {
            /* 未割当メモリ領域より前 */

            /* 前に挿入 */
            ret = FreePrev( pAllocList, pFreeList, pFree, pInfo );

        } else if ( ( pFree->pAddr + pFree->size ) < pInfo->pAddr ) {
            /* 未割当メモリ領域より後 */

            /* 次の未割当メモリ領域 */
            continue;

        } else {
            /* 未割当メモリ領域に隣接 */

            /* 結合 */
            ret = FreeMerge( pAllocList, pFreeList, pFree, pInfo );
        }

        break;
    }

    return ret;
}


/******************************************************************************/
/**
 * @brief       メモリ領域管理初期化
 * @details     空メモリ領域情報リストを初期化する。
 */
/******************************************************************************/
void AreaInit( void )
{
    uint32_t       index;   /* メモリ領域情報リストインデックス */
    MLibRet_t      retMLib; /* MLIB関数戻り値                   */
    MLibListNode_t *pEmpty; /* 空メモリ領域情報ノード           */

    /* 空メモリ領域情報リスト初期化 */
    retMLib = MLibListInit( &( gAreaTbl.emptyList ) );

    /* 初期化結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */

        /* [TODO] */
    }

    /* メモリ領域情報初期化 */
    memset( gAreaTbl.areaInfo, 0, sizeof ( gAreaTbl.areaInfo ) );

    for ( index = 0; index < AREA_INFO_NUM; index++ ) {
        /* メモリ領域情報設定 */
        gAreaTbl.areaInfo[ index ].taskId = MK_TASKID_NULL;

        /* 空メモリ領域情報参照変数設定 */
        pEmpty = ( MLibListNode_t * ) &( gAreaTbl.areaInfo[ index ] );

        /* 空メモリ領域情報リスト挿入 */
        retMLib = MLibListInsertTail( &( gAreaTbl.emptyList ),
                                           pEmpty );

        /* 挿入結果判定 */
        if ( retMLib != MLIB_SUCCESS ) {
            /* 失敗 */

            /* [TODO] */
        }
    }

    return;
}


/******************************************************************************/
/**
 * @brief       メモリ領域情報リスト設定
 * @details     メモリ領域情報を空メモリ領域情報リストから取得して情報を設定し
 *              指定メモリ領域情報リストに挿入する。
 *
 * @param[in]   *pFreeList 未割当メモリ領域情報リスト
 * @param[in]   *pAddr     メモリ領域先頭アドレス
 * @param[in]   size       メモリ領域サイズ
 *
 * @return      メモリ領域情報リスト設定結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功（メモリ領域情報の先頭アドレス）
 */
/******************************************************************************/
AreaInfo_t *AreaSet( MLibList_t *pFreeList,
                     void       *pAddr,
                     size_t     size        )
{
    MLibRet_t  retMLib; /* MLIB関数戻り値 */
    AreaInfo_t *pRet;   /* 戻り値         */

    /* 空メモリ領域情報リストから取得 */
    pRet = ( AreaInfo_t * ) MLibListRemoveTail( &( gAreaTbl.emptyList ) );

    /* 取得結果判定 */
    if ( pRet != NULL ) {
        /* 成功 */

        /* メモリ領域情報設定 */
        pRet->pAddr = pAddr;
        pRet->size  = size;

        /* 未割当メモリ領域情報リストの最後尾に挿入 */
        retMLib = MLibListInsertTail( pFreeList,
                                      ( MLibListNode_t * ) pRet );

        /* 挿入結果判定 */
        if ( retMLib != MLIB_SUCCESS ) {
            /* 失敗 */

            return NULL;
        }
    }

    return pRet;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メモリ領域割当（指定未割当メモリ領域全て）
 * @details     指定した未割当メモリ領域情報を未割当メモリ領域情報リストから割
 *              当中メモリ領域情報リストに入れ替えてメモリ領域を割り当てる。
 *
 * @param[in]   *pAllocList 割当中メモリ領域情報リスト
 * @param[in]   *pFreeList  未割当メモリ領域情報リスト
 * @param[in]   *pFree      未割当メモリ領域情報
 *
 * @return      割当結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 割り当てたメモリ領域の先頭アドレス
 */
/******************************************************************************/
static void *AllocAll( MLibList_t *pAllocList,
                       MLibList_t *pFreeList,
                       AreaInfo_t *pFree       )
{
    MLibRet_t retMLib;  /* 関数戻り値 */

    /* メモリ領域情報リスト削除 */
    retMLib = MLibListRemove( pFreeList,
                              ( MLibListNode_t * ) pFree );

    /* 削除結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */

        DEBUG_LOG( "%s(): MLibListRemove() error.", __func__ );

        return NULL;
    }

    /* メモリ領域情報リスト挿入 */
    retMLib = MLibListInsertTail( pAllocList,
                                  ( MLibListNode_t * ) pFree );

    /* 挿入結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */

        DEBUG_LOG( "%s(): MLibListInsertTail() error.", __func__ );

        return NULL;
    }

    return pFree->pAddr;
}


/******************************************************************************/
/**
 * @brief       メモリ領域割当（指定未割当メモリ領域後方）
 * @details     指定した未割当メモリ領域の後方を分割して割当中メモリ領域情報リ
 *              ストに挿入しメモリ領域を割り当てる。
 *
 * @param[in]   *pAllocList 割当中メモリ領域情報リスト
 * @param[in]   *pFree      未割当メモリ領域情報
 * @param[in]   *pAddr      メモリ領域先頭アドレス
 * @param[in]   size        メモリ領域サイズ
 *
 * @return      割当結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 割り当てたメモリ領域の先頭アドレス
 */
/******************************************************************************/
static void *AllocBack( MLibList_t *pAllocList,
                        AreaInfo_t *pFree,
                        void       *pAddr,
                        size_t     size         )
{
    MLibRet_t  retMLib; /* MLib関数戻り値   */
    AreaInfo_t *pEmpty; /* 空メモリ領域情報 */

    /* 空メモリ領域情報取得 */
    pEmpty = ( AreaInfo_t * )
        MLibListRemoveTail( &( gAreaTbl.emptyList ) );

    /* 取得結果判定 */
    if ( pEmpty == NULL ) {
        /* 失敗 */

        DEBUG_LOG( "%s(): MLibListRemoveTail() error.", __func__ );

        return NULL;
    }

    /* メモリ領域分割 */
    pFree->size   -= size;
    pEmpty->pAddr  = pAddr;
    pEmpty->size   = size;

    /* 割当中メモリ領域情報挿入 */
    retMLib = MLibListInsertTail( pAllocList,
                                  ( MLibListNode_t * ) pEmpty );

    /* 挿入結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */

        DEBUG_LOG( "%s(): MLibListInsertTail() error.", __func__ );

        return NULL;
    }

    return pAddr;
}


/******************************************************************************/
/**
 * @brief       メモリ領域割当（指定未割当メモリ領域中間）
 * @details     指定した未割当メモリ領域の中間を分割して割当中メモリ領域情報リ
 *              ストに挿入し、メモリ領域を割り当てる。
 *
 * @param[in]   *pAllocList 割当中メモリ領域情報リスト
 * @param[in]   *pFreeList  未割当メモリ領域情報リスト
 * @param[in]   *pFree      未割当メモリ領域情報
 * @param[in]   *pAddr      メモリ領域先頭アドレス
 * @param[in]   size        メモリ領域サイズ
 *
 * @return      割当結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 割り当てたメモリ領域の先頭アドレス
 */
/******************************************************************************/
static void *AllocCenter( MLibList_t *pAllocList,
                          MLibList_t *pFreeList,
                          AreaInfo_t *pFree,
                          void       *pAddr,
                          size_t     size         )
{
    size_t     allSize; /* 未割当メモリ領域サイズ */
    MLibRet_t  retMLib; /* MLib関数戻り値         */
    AreaInfo_t *pPrev;  /* 前未割当メモリ領域情報 */
    AreaInfo_t *pAlloc; /* 割当メモリ領域情報     */
    AreaInfo_t *pNext;  /* 後未割当メモリ領域情報 */

    /* 初期化 */
    pPrev   = pFree;
    allSize = pPrev->size;

    /* 割当メモリ領域情報取得 */
    pAlloc = ( AreaInfo_t * )
        MLibListRemoveTail( &( gAreaTbl.emptyList ) );

    /* 取得結果判定 */
    if ( pPrev == NULL ) {
        /* 失敗 */

        DEBUG_LOG( "%s(): MLibListRemoveTail() error.", __func__ );

        return NULL;
    }

    /* 後未割当メモリ領域情報取得 */
    pNext = ( AreaInfo_t * )
        MLibListRemoveTail( &( gAreaTbl.emptyList ) );

    /* 取得結果判定 */
    if ( pPrev == NULL ) {
        /* 失敗 */

        DEBUG_LOG( "%s(): MLibListRemoveTail() error.", __func__ );

        return NULL;
    }

    /* 前未割当メモリ領域情報設定 */
    pPrev->size = ( size_t ) pAddr - ( size_t ) pPrev->pAddr;

    /* 割当メモリ領域情報設定 */
    pAlloc->pAddr = pAddr;
    pAlloc->size  = size;

    /* 後未割当メモリ領域情報設定 */
    pNext->pAddr = END_ADDR( pAddr, size );
    pNext->size  = allSize - pPrev->size - pAlloc->size;

    /* 後未割当メモリ領域情報を前未割当メモリ領域情報の後に挿入 */
    retMLib = MLibListInsertNext( pFreeList,
                                  ( MLibListNode_t * ) pPrev,
                                  ( MLibListNode_t * ) pNext  );

    /* 挿入結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */

        /* [TODO] */
        DEBUG_LOG( "%s(): MLibListInsertNext() error.", __func__ );
    }

    /* 割当中メモリ領域情報リスト挿入 */
    retMLib = MLibListInsertTail( pAllocList,
                                  ( MLibListNode_t * ) pAlloc );

    /* 挿入結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */

        DEBUG_LOG( "%s(): MLibListInsertTail() error.", __func__ );

        return NULL;
    }

    return pAddr;
}


/******************************************************************************/
/**
 * @brief       メモリ領域割当（指定未割当メモリ領域前方）
 * @details     指定した未割当メモリ領域の前方を分割して割当中メモリ領域情報リ
 *              ストに挿入し、メモリ領域を割り当てる。
 *
 * @param[in]   *pAllocList 割当中メモリ領域情報リスト
 * @param[in]   *pFree      未割当メモリ領域情報
 * @param[in]   *pAddr      メモリ領域先頭アドレス
 * @param[in]   size        メモリ領域サイズ
 *
 * @return      割当結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 割り当てたメモリ領域の先頭アドレス
 */
/******************************************************************************/
static void *AllocFront( MLibList_t *pAllocList,
                         AreaInfo_t *pFree,
                         void       *pAddr,
                         size_t     size         )
{
    MLibRet_t  retMLib;     /* MLib関数戻り値   */
    AreaInfo_t *pEmpty;     /* 空メモリ領域情報 */

    /* 空メモリ領域情報取得 */
    pEmpty = ( AreaInfo_t * )
        MLibListRemoveTail( &( gAreaTbl.emptyList ) );

    /* 取得結果判定 */
    if ( pEmpty == NULL ) {
        /* 失敗 */

        DEBUG_LOG( "%s(): MLibListRemoveTail() error.", __func__ );

        return NULL;
    }

    /* メモリ領域分割 */
    pEmpty->pAddr  = pAddr;
    pEmpty->size   = size;
    pFree->pAddr   = END_ADDR( pAddr, size );
    pFree->size   -= size;

    /* 割当中メモリ領域情報リスト挿入 */
    retMLib = MLibListInsertTail( pAllocList,
                                  ( MLibListNode_t * ) pEmpty );

    /* 挿入結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */

        DEBUG_LOG( "%s(): MLibListInsertTail() error.", __func__ );

        return NULL;
    }

    return pAddr;
}


/******************************************************************************/
/**
 * @brief       メモリ領域解放（指定未割当メモリ領域結合）
 * @details     指定した未割当メモリ領域に割当中メモリ領域を結合し割当中メモリ
 *              領域情報リストから削除してメモリ領域を解放する。なお、削除した
 *              割当中メモリ領域情報は空メモリ領域情報リストに挿入し、結合した
 *              未割当メモリ領域情報が更に前後のメモリ領域情報と結合可能か判定
 *              し必要あれば結合する。
 *
 * @param[in]   *pAllocList 割当中メモリ領域情報リスト
 * @param[in]   *pFreeList  未割当メモリ領域情報リスト
 * @param[in]   *pAlloc     割当中メモリ領域情報
 * @param[in]   *pFree      未割当メモリ領域情報
 *
 * @return      解放結果を返す。
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
static CmnRet_t FreeMerge( MLibList_t *pAllocList,
                           MLibList_t *pFreeList,
                           AreaInfo_t *pAlloc,
                           AreaInfo_t *pFree       )
{
    MLibRet_t  retMLib; /* MLib関数戻り値                     */
    AreaInfo_t *pNext;  /* 結合先メモリ領域の次メモリ領域情報 */

    /* 初期化 */
    retMLib = MLIB_FAILURE;
    pNext   = NULL;

    /* メモリ領域位置関係比較 */
    if ( pAlloc->pAddr < pFree->pAddr ) {
        /* 割当中メモリ領域が未割当メモリ領域の前 */

        /* 未割当メモリ領域先頭アドレス設定 */
        pFree->pAddr = pAlloc->pAddr;
    }

    /* 未割当メモリ領域サイズ設定 */
    pFree->size += pAlloc->size;

    /* 割当中メモリ領域情報初期化 */
    memset( pAlloc, 0, sizeof ( AreaInfo_t ) );

    /* 空メモリ領域情報リストに挿入 */
    retMLib = MLibListInsertTail( &( gAreaTbl.emptyList ),
                                  ( MLibListNode_t * ) pAlloc );

    /* 挿入結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */

        /* [TODO]トレース情報 */
    }

    /* 次未割当メモリ領域情報取得 */
    pNext = ( AreaInfo_t * )
        MLibListGetNextNode( pFreeList, ( MLibListNode_t * ) pFree );

    /* 取得結果判定 */
    if ( pNext == NULL ) {
        /* メモリ領域情報無 */

        return CMN_SUCCESS;
    }

    /* メモリ領域結合可能判定 */
    if ( END_ADDR( pFree->pAddr, pFree->size ) == pNext->pAddr ) {
        /* 結合可能 */

        /* メモリ領域結合 */
        pFree->size += pNext->size;

        /* 未割当メモリ領域情報リストから削除 */
        retMLib = MLibListRemove( pFreeList,
                                  ( MLibListNode_t * ) pNext );

        /* 削除結果判定 */
        if ( retMLib != MLIB_SUCCESS ) {
            /* 失敗 */

            return CMN_FAILURE;
        }

        /* 次メモリ領域情報初期化 */
        memset( pNext, 0, sizeof ( AreaInfo_t ) );

        /* 空メモリ領域情報リストに挿入 */
        retMLib = MLibListInsertTail( &( gAreaTbl.emptyList ),
                                      ( MLibListNode_t * ) pNext );

        /* 挿入結果判定 */
        if ( retMLib != MLIB_SUCCESS ) {
            /* 失敗 */

            /* [TODO]トレース情報 */
        }
    }

    return CMN_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       メモリ領域解放（指定未割当メモリ領域前挿入）
 * @details     割当中メモリ領域を割当中メモリ領域情報リストから削除して指定し
 *              た未割当メモリ領域の前に割当中メモリ領域を挿入し、メモリ領域を
 *              解放する。
 *
 * @param[in]   *pAllocList 割当中メモリ領域情報リスト
 * @param[in]   *pFreeList  未割当メモリ領域情報リスト
 * @param[in]   *pAlloc     割当中メモリ領域情報
 * @param[in]   *pFree      未割当メモリ領域情報
 *
 * @return      解放結果を返す。
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
static CmnRet_t FreePrev( MLibList_t *pAllocList,
                          MLibList_t *pFreeList,
                          AreaInfo_t *pAlloc,
                          AreaInfo_t *pFree       )
{
    MLibRet_t retMLib;  /* MLib関数戻り値 */

    /* 割当中メモリ領域情報リストから削除 */
    retMLib = MLibListRemove( pAllocList,
                              ( MLibListNode_t * ) pAlloc );

    /* 削除結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */

        return CMN_FAILURE;
    }

    /* 未割当メモリ領域情報リスト挿入 */
    retMLib = MLibListInsertPrev( pAllocList,
                                  ( MLibListNode_t * ) pFree,
                                  ( MLibListNode_t * ) pAlloc );

    /* 挿入結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */

        return CMN_FAILURE;
    }

    return CMN_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       メモリ領域解放（最後尾挿入）
 * @details     割当中メモリ領域を割当中メモリ領域情報リストから削除して未割当
 *              メモリ領域情報リストの最後尾に挿入し、メモリ領域を解放する。
 *
 * @param[in]   *pAllocList 割当中メモリ領域情報リスト
 * @param[in]   *pFreeList  未割当メモリ領域情報リスト
 * @param[in]   *pAlloc     割当中メモリ領域情報
 *
 * @return      解放結果を返す。
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
static CmnRet_t FreeTail( MLibList_t *pAllocList,
                          MLibList_t *pFreeList,
                          AreaInfo_t *pAlloc      )
{
    MLibRet_t retMLib;  /* MLib関数戻り値 */

    /* 割当中メモリ領域情報リストから削除 */
    retMLib = MLibListRemove( pAllocList,
                              ( MLibListNode_t * ) pAlloc );

    /* 削除結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */

        return CMN_FAILURE;
    }

    /* 未割当メモリ領域情報リスト最後尾挿入 */
    retMLib = MLibListInsertTail( pFreeList,
                                  ( MLibListNode_t * ) pAlloc );

    /* 挿入結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */

        return CMN_FAILURE;
    }

    return CMN_SUCCESS;
}


/******************************************************************************/
