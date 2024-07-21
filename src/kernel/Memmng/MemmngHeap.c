/******************************************************************************/
/*                                                                            */
/* src/kernel/Memmng/MemmngHeap.c                                             */
/*                                                                 2024/07/21 */
/* Copyright (C) 2019-2024 Mcohi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/* ライブラリヘッダ */
#include <MLib/MLibList.h>
#include <MLib/MLibUtil.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Memmng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* モジュールID */
#define _MODULE_ID_ CMN_MODULE_MEMMNG_HEAP

/** 領域情報 */
typedef struct {
    MLibListNode_t nodeInfo;    /**< ノード情報 */
    size_t         size;        /**< 領域サイズ */
    uint8_t        area[];      /**< 領域       */
} areaInfo_t;


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* メモリ領域割当（未使用メモリ領域） */
static void *AllocFromFreeArea( areaInfo_t *pArea,
                                size_t     size    );
/* メモリ領域割当（新ヒープ領域） */
static void *AllocFromNewHeap( size_t size );
/* 未使用メモリ領域リスト挿入 */
static void InsertFreeList( areaInfo_t *pAreaInfo );
/* ブレイクポイント設定 */
static void SetBreakPoint( int quantity );


/******************************************************************************/
/* グローバル変数定義                                                         */
/******************************************************************************/
/** エンドポイント（リンカスクリプト） */
extern void *__end_point;
/** カーネルブレイクポイント */
void *pgBreakPoint;
/** 未使用メモリ領域リスト */
static MLibList_t gFreeList;
/** 使用中メモリ領域リスト */
static MLibList_t gUsedList;


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       カーネルヒープ領域割当
 * @details     割当てサイズ size を未使用メモリ領域から割り当てる。該当する未
 *              使用メモリ領域がなければ新しくヒープ領域を拡張して割り当てる。
 *
 * @param[in]   size 割当サイズ
 *
 * @return      割り当てたヒープ領域へのポインタを返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功（ヒープ領域）
 */
/******************************************************************************/
void *MemmngHeapAlloc( size_t size )
{
    areaInfo_t *pAreaInfo;   /* 領域情報 */

    /* 初期化 */
    pAreaInfo = NULL;

    /* サイズチェック */
    if ( size == 0 ) {
        /* 不正 */

        return NULL;
    }

    /* サイズアライメント */
    size = MLIB_UTIL_ALIGN( size, 4 );

    /* 未使用メモリ領域毎に繰り返す */
    while ( true ) {
        /* 未使用メモリ領域取得 */
        pAreaInfo = ( areaInfo_t * )
                    MLibListGetNextNode( &gFreeList,
                                         ( MLibListNode_t * ) pAreaInfo );

        /* 取得結果判定 */
        if ( pAreaInfo == NULL ) {
            /* 未使用メモリ領域無し */

            /* ヒープ領域を拡張して割当て */
            return AllocFromNewHeap( size );
        }

        /* 未使用メモリ領域サイズ判定 */
        if ( pAreaInfo->size >= size ) {
            /* サイズ不足無し */

            /* 未使用メモリ領域から割当て */
            return AllocFromFreeArea( pAreaInfo, size );
        }
    }
}


/******************************************************************************/
/**
 * @brief       カーネルヒープ領域解放
 * @details     メモリ領域 pAddr を解放する。必要があればブレイクポイントを更新
 *              する。
 *
 * @param[in]   *pAddr ヒープ領域
 */
/******************************************************************************/
void MemmngHeapFree( void *pAddr )
{
    MLibRet_t  retMLib;     /* MLib関数戻り値 */
    areaInfo_t *pAreaInfo;  /* メモリ領域情報 */

    /* アドレスチェック */
    if ( pAddr == NULL ) {
        /* 不正 */

        return;
    }

    /* 初期化 */
    retMLib   = MLIB_RET_FAILURE;
    pAreaInfo = ( areaInfo_t * ) ( pAddr - offsetof( areaInfo_t, area ) );

    /* 使用中メモリ領域リストから削除 */
    retMLib = MLibListRemove( &gUsedList, &( pAreaInfo->nodeInfo ) );

    /* 削除結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        return;
    }

    /* ブレイクポイント変更判定 */
    if ( pgBreakPoint == ( pAddr + pAreaInfo->size ) ) {
        /* ブレイクポイント変更有り */

        /* ブレイクポイント減算 */
        SetBreakPoint( - ( sizeof ( areaInfo_t ) + pAreaInfo->size ) );

    } else {
        /* ブレイクポイント変更無し */

        /* 未使用メモリ領域リスト挿入 */
        InsertFreeList( pAreaInfo );
    }

    return;
}


/******************************************************************************/
/* モジュール内グローバル関数定義                                             */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       ヒープ管理初期化
 * @details     カーネル用ブレイクポイントの初期化とヒープ領域管理リストの初期
 *              化を行う。
 */
/******************************************************************************/
void HeapInit( void )
{
    /* ブレイクポイント初期化 */
//    pgBreakPoint = &__end_point;
    pgBreakPoint = ( void * ) 0x10000000;

    /* 未使用メモリ領域リスト初期化 */
    MLibListInit( &gFreeList );

    /* 使用中メモリ領域リスト初期化 */
    MLibListInit( &gUsedList );

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メモリ領域割当（未使用メモリ領域）
 * @details     未使用メモリ領域情報 pAreaInfo から必要なサイズ size の領域を割
 *              り当てる。
 *
 * @param[in]   *pAreaInfo 未使用メモリ領域情報
 * @param[in]   size       割当てサイズ
 *
 * @return      割り当てたメモリ領域へのポインタを返す。
 */
/******************************************************************************/
static void *AllocFromFreeArea( areaInfo_t *pAreaInfo,
                                size_t     size        )
{
    areaInfo_t *pFreeAreaInfo;  /* 未使用メモリ領域情報 */

    /* 分割判定 */
    if ( ( pAreaInfo->size - size ) >= ( sizeof ( areaInfo_t ) + 4 ) ) {
        /* 分割可能 */

        /* 分割 */
        pAreaInfo->size -= sizeof ( areaInfo_t ) + size;

        /* 未使用メモリ領域情報設定 */
        pFreeAreaInfo       = ( areaInfo_t * )
                              ( ( ( uint32_t ) pAreaInfo->area ) +
                                pAreaInfo->size                    );
        pFreeAreaInfo->size = size;

    } else {
        /* 分割不可 */

        /* 未使用メモリ領域設定 */
        pFreeAreaInfo = pAreaInfo;

        /* 未使用メモリ領域リストから削除 */
        MLibListRemove( &gFreeList, &( pAreaInfo->nodeInfo ) );
    }

    /* 使用中メモリ領域リスト追加 */
    MLibListInsertHead( &gUsedList, &( pFreeAreaInfo->nodeInfo ) );

    return pFreeAreaInfo->area;
}


/******************************************************************************/
/**
 * @brief       メモリ領域割当（新ヒープ領域）
 * @details     必要なサイズ size 分のヒープ領域を拡張してメモリ領域を割り当て
 *              る。
 *
 * @param[in]   size 割当てサイズ
 *
 * @return      割り当てたメモリ領域へのポインタを返す。
 */
/******************************************************************************/
static void *AllocFromNewHeap( size_t size )
{
    areaInfo_t *pAreaInfo;  /* 割当てメモリ領域情報 */

    /* 初期化 */
    pAreaInfo = pgBreakPoint;

    /* ブレイクポイント設定 */
    SetBreakPoint( sizeof ( areaInfo_t ) + size );

    /* メモリ領域サイズ設定 */
    pAreaInfo->size = size;

    /* 使用中メモリ領域リスト追加 */
    MLibListInsertHead( &gUsedList, &( pAreaInfo->nodeInfo ) );

    return pAreaInfo->area;
}


/******************************************************************************/
/**
 * @brief       未使用メモリ領域リスト挿入
 * @details     未使用メモリ領域情報 pAreaInfo を未使用メモリ領域リストの適切な
 *              位置に挿入する。必要があれば前後の未使用メモリ領域と結合する。
 *
 * @param[in]   pAreaInfo 未使用メモリ領域情報
 */
/******************************************************************************/
static void InsertFreeList( areaInfo_t *pAreaInfo )
{
    areaInfo_t *pPrevAreaInfo;  /* 前メモリ領域 */
    areaInfo_t *pNextAreaInfo;  /* 次メモリ領域 */

    /* 初期化 */
    pPrevAreaInfo = NULL;
    pNextAreaInfo = NULL;

    do {
        /* シフト */
        pPrevAreaInfo = pNextAreaInfo;

        /* 次未使用メモリ領域情報取得 */
        pNextAreaInfo =
            ( areaInfo_t * )
            MLibListGetNextNode( &gFreeList,
                                 ( MLibListNode_t * ) pPrevAreaInfo );

        /* 取得結果判定 */
        if ( pNextAreaInfo == NULL ) {
            /* 無し */

            break;
        }

    /* 挿入位置判定 */
    } while ( !( ( pPrevAreaInfo < pAreaInfo                 ) &&
                 (                 pAreaInfo < pNextAreaInfo )    ) );

    /* 次ノード有無判定 */
    if ( pNextAreaInfo != NULL ) {
        /* 有り */

        /* 次ノード隣接判定 */
        if ( ( ( uint32_t ) pAreaInfo->area + pAreaInfo->size ) ==
             ( ( uint32_t ) pNextAreaInfo                     )    ) {
            /* 隣接 */

            /* 次ノード結合 */
            pAreaInfo->size += sizeof ( areaInfo_t ) + pNextAreaInfo->size;

            /* 次ノード削除 */
            MLibListRemove( &gFreeList, &( pNextAreaInfo->nodeInfo ) );
        }
    }

    /* 前ノード有無判定 */
    if ( pPrevAreaInfo != NULL ) {
        /* 有り */

        /* 前ノード隣接判定 */
        if ( ( ( uint32_t ) pPrevAreaInfo->area + pPrevAreaInfo->size ) ==
             ( ( uint32_t ) pAreaInfo                                 )    ) {
            /* 隣接 */

            /* 前ノード結合 */
            pPrevAreaInfo->size += sizeof ( areaInfo_t ) + pAreaInfo->size;

            return;
        }

        /* 挿入 */
        MLibListInsertNext( &gFreeList,
                            &( pPrevAreaInfo->nodeInfo ),
                            &( pAreaInfo->nodeInfo     )  );

    } else {
        /* 無し */

        /* 挿入 */
        MLibListInsertHead( &gFreeList, &( pAreaInfo->nodeInfo ) );
    }

    return;
}


/******************************************************************************/
/**
 * @brief       ブレイクポイント設定
 * @details     プレイクポイントを設定する。必要があれば、仮想メモリの割当てま
 *              たは解放を行う。
 *
 * @param[in]   quantity 増減量
 */
/******************************************************************************/
static void SetBreakPoint( int quantity )
{
    int      once;          /* ページサイズ以内増減量 */
    void     *pPhysAddr;    /* 物理アドレス           */
    void     *pVirtAddr;    /* 仮想アドレス           */
    uint32_t oldPageNum;    /* 増減前ページ数         */
    uint32_t newPageNum;    /* 増減後ページ数         */
    CmnRet_t ret;           /* 関数戻り値             */

    DEBUG_LOG_TRC( "%s(): quantity=%d", __func__, quantity );

    /* 初期化 */
    once       = 0;
    pPhysAddr  = NULL;
    pVirtAddr  = NULL;
    oldPageNum = 0;
    newPageNum = 0;

    /* 最大ページサイズ分の増減毎に繰り返す */
    while ( quantity != 0 ) {
        /* 残増減量比較 */
        if ( ( ( -IA32_PAGING_PAGE_SIZE ) <= quantity              ) &&
             ( quantity                   <= IA32_PAGING_PAGE_SIZE )    ) {
            /* ページサイズ以内の増減量 */

            once     = quantity;
            quantity = 0;

        } else if ( quantity < ( - IA32_PAGING_PAGE_SIZE ) ) {
            /* ページサイズを超えた減量 */

            once      = ( - IA32_PAGING_PAGE_SIZE );
            quantity += IA32_PAGING_PAGE_SIZE;

        } else {
            /* ページサイズを超えた増量 */

            once      = IA32_PAGING_PAGE_SIZE;
            quantity -= IA32_PAGING_PAGE_SIZE;
        }

        /* ブレイクポイント増減前後のページ数計算 */
        oldPageNum = ( ( int ) ( pgBreakPoint ) - 1        ) /
                     IA32_PAGING_PAGE_SIZE;
        newPageNum = ( ( int ) ( pgBreakPoint ) - 1 + once ) /
                     IA32_PAGING_PAGE_SIZE;

        /* 増減前後ページ数比較 */
        if ( oldPageNum < newPageNum ) {
            /* ページ数増加 */

            /* 物理メモリ領域割当て */
            pPhysAddr = MemmngPhysAlloc( IA32_PAGING_PAGE_SIZE );

            /* 割当て結果判定 */
            if ( pPhysAddr == NULL ) {
                /* 失敗 */

                return;
            }

            /* 0初期化 */
            MemmngCtrlSet( pPhysAddr, 0, IA32_PAGING_PAGE_SIZE );

            /* ページ先頭アドレス計算 */
            pVirtAddr = ( void * )
                        MLIB_UTIL_ALIGN( ( int ) ( pgBreakPoint ) - 1,
                                         IA32_PAGING_PAGE_SIZE         );

            /* ページマッピング設定 */
            ret = MemmngPageSet( MEMMNG_PAGE_DIR_ID_IDLE,
                                 pVirtAddr,
                                 pPhysAddr,
                                 IA32_PAGING_PAGE_SIZE,
                                 IA32_PAGING_G_YES,
                                 IA32_PAGING_US_SV,
                                 IA32_PAGING_RW_RW        );

            /* 設定結果判定 */
            if ( ret != CMN_SUCCESS ) {
                /* 失敗 */

                /* 物理メモリ領域解放 */
                MemmngPhysFree( pPhysAddr );

                return;
            }

        } else if ( oldPageNum > newPageNum ) {
            /* ページ数減少 */

            /* ページ先頭アドレス計算 */
            pVirtAddr = ( void * )
                        MLIB_UTIL_ALIGN( ( int ) ( pgBreakPoint ) - 1 + once,
                                         IA32_PAGING_PAGE_SIZE                );

            /* ページマッピング解除 */
            MemmngPageUnset( MEMMNG_PAGE_DIR_ID_IDLE,
                             pVirtAddr,
                             IA32_PAGING_PAGE_SIZE    );
        }

        /* ブレイクポイント更新 */
        pgBreakPoint += once;
    }

    return;
}


/******************************************************************************/
