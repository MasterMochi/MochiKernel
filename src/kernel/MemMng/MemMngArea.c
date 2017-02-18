/******************************************************************************/
/* src/kernel/MemMng/MemMngArea.c                                             */
/*                                                                 2017/02/18 */
/* Copyright (C) 2017 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdbool.h>
#include <string.h>
#include <MLib/MLib.h>
#include <MLib/Basic/MLibBasic.h>
#include <MLib/Basic/MLibBasicList.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>

/* 内部モジュールヘッダ */


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** メモリ領域情報数 */
#define AREA_INFO_NUM  ( 1000000 )

/** メモリ領域アライメント */
#define AREA_ALIGNMENT ( 0x1000 )

/* メモリ領域情報使用フラグ */
#define AREA_INFO_UNUSED ( 0 )  /** メモリ領域情報使用中 */
#define AREA_INFO_USED   ( 1 )  /** メモリ領域情報未使用 */

/** メモリ領域情報構造体 */
typedef struct {
    MLibBasicListNode_t node;   /**< 連結リストノード情報 */
    uint32_t            used;   /**< 使用フラグ           */
    void                *pAddr; /**< 先頭アドレス         */
    size_t              size;   /**< サイズ               */
} AreaInfo_t;

/** メモリ領域管理テーブル構造体 */
typedef struct {
    MLibBasicList_t usedList;                   /**< 使用中メモリ領域リスト */
    MLibBasicList_t freeList;                   /**< 未使用メモリ領域リスト */
    MLibBasicList_t emptyList;                  /**< 空メモリ領域情報リスト */
    AreaInfo_t      areaInfo[ AREA_INFO_NUM ];  /**< メモリ領域情報         */
} AreaTbl_t;


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** メモリ領域管理テーブル */
static AreaTbl_t gAreaTbl;


/******************************************************************************/
/* ローカル関数プロトタイプ宣言                                               */
/******************************************************************************/
/* 指定メモリ領域割当 */
static void *AreaAlloc( AreaInfo_t *pFree );

/* 指定メモリ領域割当（一部） */
static void *AreaAllocPartially( AreaInfo_t *pFree,
                                 size_t     size    );

/* 指定メモリ領域解放（結合） */
static CmnRet_t AreaFree( AreaInfo_t *pFree,
                          AreaInfo_t *pUsed  );

/* 指定メモリ領域解放（最後尾挿入） */
static CmnRet_t AreaFreeAfterTail( AreaInfo_t *pUsed );

/* 指定メモリ領域解放（前挿入） */
static CmnRet_t AreaFreeBefore( AreaInfo_t *pFree,
                                AreaInfo_t *pUsed  );


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メモリ領域割当
 * @details     指定したサイズを満たすメモリ領域を割り当てる。
 * 
 * @param[in]   size 割当サイズ
 * 
 * @retval      NULL     失敗
 * @retval      NULL以外 割り当てたメモリ領域の先頭アドレス
 * 
 * @note        割当サイズが4Kバイトアライメントでない場合は、割当サイズが4Kバ
 *              イトアライメントに合うよう加算して、メモリ領域を割り当てる。
 */
/******************************************************************************/
void *MemMngAreaAlloc( size_t size )
{
    void       *pAddr;  /* 割当メモリ領域先頭アドレス */
    AreaInfo_t *pFree;  /* 未使用メモリ領域情報       */
    
    /* 初期化 */
    pAddr = NULL;
    pFree = NULL;
    
    /* サイズチェック */
    if ( size == 0 ) {
        /* 不正 */
        
        return NULL;
        
    } else {
        /* 正常 */
        
        /* アライメント計算 */
        size = MLIB_BASIC_ALIGN( size, AREA_ALIGNMENT );
    }
    
    /* 割当可能な未使用メモリ領域検索 */
    do {
        /* 未使用メモリ領域情報取得 */
        pFree = ( AreaInfo_t * )
            MLibBasicListGetNextNode( &( gAreaTbl.freeList ),
                                      ( MLibBasicListNode_t * ) pFree );
        
        /* 取得結果判定 */
        if ( pFree == NULL ) {
            /* メモリ領域情報無 */
            
            return NULL;
        }
        
        /* 割当可能判定 */
    } while ( pFree->size < size );
    
    /* 未使用メモリ領域サイズ比較 */
    if ( pFree->size == size ) {
        /* 必要サイズと一致 */
        
        /* 同サイズの未使用メモリ領域からメモリ割当 */
        pAddr = AreaAlloc( pFree );
        
    } else {
        /* 必要サイズを超過 */
        
        /* 超過サイズの未使用メモリ領域からメモリ割当 */
        pAddr = AreaAllocPartially( pFree, size );
    }
    
    return pAddr;
}


/******************************************************************************/
/**
 * @brief       メモリ領域解放
 * @details     指定したメモリアドレスが先頭アドレスのメモリ領域を解放する。
 * 
 * @param[in]   *pAddr 先頭アドレス
 * 
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
CmnRet_t MemMngAreaFree( void *pAddr )
{
    CmnRet_t   ret;     /* 関数戻り値           */
    MLibRet_t  retMLib; /* MLib関数戻り値       */
    AreaInfo_t *pFree;  /* 未使用メモリ領域情報 */
    AreaInfo_t *pInfo;  /* 使用中メモリ領域情報 */
    
    /* 初期化 */
    ret     = CMN_FAILURE;
    retMLib = MLIB_FAILURE;
    pFree   = NULL;
    pInfo   = NULL;
    
    /* 引数チェック */
    if ( pAddr == NULL ) {
        /* 不正 */
        
        return CMN_FAILURE;
    }
    
    /* 該当メモリ領域情報検索 */
    do {
        /* 使用中メモリ領域情報取得 */
        pInfo = ( AreaInfo_t * )
            MLibBasicListGetPrevNode( &( gAreaTbl.usedList ),
                                      ( MLibBasicListNode_t * ) pInfo );
        
        /* 取得結果判定 */
        if ( pInfo == NULL ) {
            /* メモリ領域情報無 */
            
            return CMN_FAILURE;
        }
        
        /* 先頭アドレス比較 */
    } while ( pInfo->pAddr != pAddr );
    
    /* 使用中メモリ領域リストから削除 */
    retMLib = MLibBasicListRemove( &( gAreaTbl.usedList ),
                                   ( MLibBasicListNode_t * ) pInfo );
    
    /* 削除結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        return CMN_FAILURE;
    }
    
    while ( true ) {
        /* 未使用メモリ領域情報取得 */
        pFree = ( AreaInfo_t * )
            MLibBasicListGetNextNode( &( gAreaTbl.freeList ),
                                      ( MLibBasicListNode_t * ) pFree );
        
        /* メモリ領域位置関係比較 */
        if ( pFree == NULL ) {
            /* メモリ領域情報無 */
            
            /* 未使用メモリ領域リストの最後尾に挿入 */
            ret = AreaFreeAfterTail( pInfo );
            
        } else if ( ( pInfo->pAddr + pInfo->size ) < pFree->pAddr ) {
            /* 未使用メモリ領域より前 */
            
            /* 前に挿入 */
            ret = AreaFreeBefore( pFree, pInfo );
            
        } else if ( ( pFree->pAddr + pFree->size ) < pInfo->pAddr ) {
            /* 未使用メモリ領域より後 */
            
            /* 次の未使用メモリ領域 */
            continue;
            
        } else {
            /* 未使用メモリ領域に隣接 */
            
            /* 結合 */
            ret = AreaFree( pFree, pInfo );
        }
        
        break;
    }
    
    return ret;
}


/******************************************************************************/
/**
 * @brief       メモリ領域管理初期化
 * @details     メモリ領域管理テーブルを初期化する。
 */
/******************************************************************************/
void MemMngAreaInit( void )
{
    uint32_t            index;      /* メモリ領域情報インデックス */
    MLibRet_t           retMLib;    /* MLib関数戻り値             */
    MLibBasicListNode_t *pEmpty;    /* 空メモリ領域情報           */
    
    /* 初期化 */
    index   = 0;
    retMLib = MLIB_FAILURE;
    pEmpty  = NULL;
    
    /* 使用中メモリ領域リスト初期化 */
    retMLib = MLibBasicListInit( &( gAreaTbl.usedList ) );
    
    /* 初期化結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        /* [TODO]カーネルパニック */
    }
    
    /* 未使用メモリ領域リスト初期化 */
    retMLib = MLibBasicListInit( &( gAreaTbl.freeList ) );
    
    /* 初期化結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        /* [TODO]カーネルパニック */
    }
    
    /* 空メモリ領域情報リスト初期化 */
    retMLib = MLibBasicListInit( &( gAreaTbl.emptyList ) );
    
    /* 初期化結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        /* [TODO]カーネルパニック */
    }
    
    /* メモリ領域情報初期化 */
    memset( gAreaTbl.areaInfo, 0, sizeof ( gAreaTbl.areaInfo ) );
    
    for ( index = 0; index < AREA_INFO_NUM; index++ ) {
        /* 空メモリ領域情報参照変数設定 */
        pEmpty = ( MLibBasicListNode_t * ) &( gAreaTbl.areaInfo[ index ] );
        
        /* 空メモリ領域情報リスト挿入 */
        retMLib = MLibBasicListInsertTail( &( gAreaTbl.emptyList ),
                                           pEmpty );
        
        /* 挿入結果判定 */
        if ( retMLib != MLIB_SUCCESS ) {
            /* 失敗 */
            
            /* [TODO]カーネルパニック */
            
        }
    }
    
    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       指定メモリ領域割当
 * @details     指定した未使用メモリ領域を割り当てる。
 * 
 * @param[in]   *pFree 未使用メモリ領域情報
 * 
 * @retval      NULL     失敗
 * @retval      NULL以外 割当メモリ領域の先頭アドレス
 */
/******************************************************************************/
static void *AreaAlloc( AreaInfo_t *pFree )
{
    MLibRet_t retMLib;  /* MLib関数戻り値 */
    
    /* 未使用メモリ領域リストから削除 */
    retMLib = MLibBasicListRemove( &( gAreaTbl.freeList ),
                                   ( MLibBasicListNode_t * ) pFree );
    
    /* 削除結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        return NULL;
    }
    
    /* 使用中メモリ領域リストに追加 */
    retMLib = MLibBasicListInsertTail( &( gAreaTbl.usedList ),
                                       ( MLibBasicListNode_t * ) pFree );
    
    /* 追加結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        return NULL;
    }
    
    /* メモリ領域先頭アドレス返却 */
    return pFree->pAddr;
}


/******************************************************************************/
/**
 * @brief       指定メモリ領域割当（一部）
 * @details     指定した未使用メモリ領域から指定した領域サイズのメモリ領域を割
 *              り当てる。
 * 
 * @param[in]   *pFree 未使用メモリ領域情報
 * @param[in]   size   割当てサイズ
 * 
 * @retval      NULL     失敗
 * @retval      NULL以外 割当メモリ領域の先頭アドレス
 */
/******************************************************************************/
static void *AreaAllocPartially( AreaInfo_t *pFree,
                                 size_t     size    )
{
    MLibRet_t  retMLib;     /* MLib関数戻り値   */
    AreaInfo_t *pEmpty;     /* 空メモリ領域情報 */
    
    /* 初期化 */
    retMLib = MLIB_FAILURE;
    
    /* 空メモリ領域情報リストからメモリ領域情報取得 */
    pEmpty = ( AreaInfo_t * )
        MLibBasicListRemoveTail( &( gAreaTbl.emptyList ) );
    
    /* 取得結果判定 */
    if ( pEmpty == NULL ) {
        /* メモリ領域情報無 */
        
        return NULL;
    }
    
    /* 空メモリ領域情報設定 */
    pEmpty->used  = AREA_INFO_USED;     /* 使用フラグ   */
    pEmpty->pAddr = pFree->pAddr;       /* 先頭アドレス */
    pEmpty->size  = size;               /* サイズ       */
    
    /* 空メモリ領域情報を使用中メモリ領域リストに追加 */
    retMLib = MLibBasicListInsertTail( &( gAreaTbl.usedList ),
                                       ( MLibBasicListNode_t * ) pEmpty );
    
    /* 追加結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        return NULL;
    }
    
    /* 未使用メモリ領域情報設定 */
    pFree->pAddr += size;               /* 先頭アドレス */
    pFree->size  -= size;               /* サイズ       */
    
    /* メモリ領域先頭アドレス返却 */
    return pEmpty->pAddr;
}


/******************************************************************************/
/**
 * @brief       指定メモリ領域解放（結合）
 * @details     指定した使用中メモリ領域を指定した未使用メモリ領域に結合する。
 * 
 * @param[in]   *pFree 解放先未使用メモリ領域情報
 * @param[in]   *pUsed 使用中メモリ領域情報
 * 
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
static CmnRet_t AreaFree( AreaInfo_t *pFree,
                          AreaInfo_t *pUsed  )
{
    size_t     size;    /* 未使用メモリ領域サイズ               */
    MLibRet_t  retMLib; /* MLib関数戻り値                       */
    AreaInfo_t *pNext;  /* 未使用メモリ領域の次のメモリ領域情報 */
    
    /* 初期化 */
    size    = pFree->size;
    retMLib = MLIB_FAILURE;
    pNext   = NULL;
    
    /* メモリ領域位置関係比較 */
    if ( pUsed->pAddr > pFree->pAddr ) {
        /* 使用中メモリ領域が未使用メモリ領域の前 */
        
        /* 未使用メモリ領域先頭アドレス設定 */
        pFree->pAddr = pUsed->pAddr;
    }
    
    /* 未使用メモリ領域サイズ設定 */
    pFree->size += pUsed->size;
    
    /* 使用中メモリ領域情報初期化 */
    memset( pUsed, 0, sizeof ( AreaInfo_t ) );
    
    /* 使用中メモリ領域情報を空メモリ領域情報リストの最後尾に挿入 */
    retMLib = MLibBasicListInsertTail( &( gAreaTbl.emptyList ),
                                       ( MLibBasicListNode_t * ) pUsed );
    
    /* 挿入結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        /* [TODO]トレース情報 */
    }
    
    /* 未使用メモリ領域の次のメモリ領域情報取得 */
    pNext = ( AreaInfo_t * )
        MLibBasicListGetNextNode( &( gAreaTbl.freeList ),
                                  ( MLibBasicListNode_t * ) pFree );
    
    /* 取得結果判定 */
    if ( pNext == NULL ) {
        /* メモリ領域情報無 */
        
        return CMN_SUCCESS;
    }
    
    /* メモリ領域隣接判定 */
    if ( ( pFree->pAddr + pFree->size ) == pNext->pAddr ) {
        /* 隣接する */
        
        /* 次メモリ領域情報を未使用メモリ領域リストから削除 */
        retMLib = MLibBasicListRemove( &( gAreaTbl.freeList ),
                                       ( MLibBasicListNode_t * ) pNext );
        
        /* 削除結果判定 */
        if ( retMLib != MLIB_SUCCESS ) {
            /* 失敗 */
            
            /* 未使用メモリ領域サイズ復元 */
            pFree->size = size;
            
            return CMN_FAILURE;
        }
        
        /* 未使用メモリ領域サイズ設定 */
        pFree->size += pNext->size;
        
        /* 次メモリ領域情報初期化 */
        memset( pNext, 0, sizeof ( AreaInfo_t ) );
        
        /* 次メモリ領域情報を空メモリ領域情報リストの最後尾に挿入 */
        retMLib = MLibBasicListInsertTail( &( gAreaTbl.emptyList ),
                                           ( MLibBasicListNode_t * ) pUsed );
        
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
 * @brief       指定メモリ領域解放（最後尾挿入）
 * @details     指定した使用中メモリ領域を未使用メモリ領域リストの最後尾に挿入
 *              する。
 * 
 * @param[in]   *pUsed 使用中メモリ領域情報
 * 
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
static CmnRet_t AreaFreeAfterTail( AreaInfo_t *pUsed )
{
    MLibRet_t retMLib;  /* MLib関数戻り値 */
    
    /* 最後尾挿入 */
    retMLib = MLibBasicListInsertTail( &( gAreaTbl.freeList ),
                                       ( MLibBasicListNode_t * ) pUsed );
    
    /* 挿入結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        return CMN_FAILURE;
    }
    
    return CMN_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       指定メモリ領域解放（前挿入）
 * @details     指定した使用中メモリ領域を指定した未使用メモリ領域の前に挿入す
 *              る。
 * 
 * @param[in]   *pFree 未使用メモリ領域情報
 * @param[in]   *pUsed 使用中メモリ領域情報
 * 
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
static CmnRet_t AreaFreeBefore( AreaInfo_t *pFree,
                                AreaInfo_t *pUsed  )
{
    MLibRet_t retMLib;  /* MLib関数戻り値 */
    
    /* 指定した未使用メモリ領域の前に挿入 */
    retMLib = MLibBasicListInsertPrev( &( gAreaTbl.freeList ),
                                       ( MLibBasicListNode_t * ) pFree,
                                       ( MLibBasicListNode_t * ) pUsed  );
    
    /* 挿入結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        return CMN_FAILURE;
    }
    
    return CMN_SUCCESS;
}


/******************************************************************************/
