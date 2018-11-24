/******************************************************************************/
/* src/kernel/MemMng/MemMngIo  .c                                             */
/*                                                                 2018/09/16 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdarg.h>
#include <kernel/kernel.h>
#include <MLib/Basic/MLibBasic.h>
#include <MLib/Basic/MLibBasicList.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>

/* 内部モジュールヘッダ */
#include "MemMngArea.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_MEMMNG_IO,   \
                    __LINE__,               \
                    __VA_ARGS__ )
#else
#define DEBUG_LOG( ... )
#endif

/** I/Oメモリ領域管理テーブル構造体 */
typedef struct {
    MLibBasicList_t allocList;  /**< 割当中I/Oメモリ領域情報リスト */
    MLibBasicList_t freeList;   /**< 未割当I/Oメモリ領域情報リスト */
} IoTbl_t;


/******************************************************************************/
/* 変数定義                                                                   */
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
 * @return      割当結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 割り当てたI/Oメモリ領域の先頭アドレス
 * 
 * @note        先頭アドレスと割当サイズが4Kバイトアライメントでない場合は、割
 *              り当てを行わない。
 */
/******************************************************************************/
void *MemMngIoAlloc( void   *pAddr,
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
    
    /* メモリ領域割当 */
    pRet = AreaAllocSpecified( &( gIoTbl.allocList ),
                               &( gIoTbl.freeList  ),
                               pAddr,
                               size                   );
    
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
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
CmnRet_t MemMngIoFree( void *pAddr )
{
    CmnRet_t ret;   /* 戻り値 */
    
    /* メモリ領域解放 */
    ret = AreaFree( &( gIoTbl.allocList ), &( gIoTbl.freeList ), pAddr );
    
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
    uint32_t        index;      /* メモリマップエントリインデックス */
    AreaInfo_t      *pMemInfo;  /* 未割当メモリ領域情報             */
    MkMemMapEntry_t *pEntry;    /* メモリマップエントリ             */
    
    /* 未割当I/Oメモリ領域情報リスト初期化 */
    MLibBasicListInit( &( gIoTbl.freeList ) );
    
    /* 割当中I/Oメモリ領域情報リスト初期化 */
    MLibBasicListInit( &( gIoTbl.allocList ) );
    
    /* メモリマップエントリ毎に繰り返し */
    for ( index = 0; index < entryNum; index++ ) {
        /* メモリマップ参照設定 */
        pEntry = &( pMemMap[ index ] );
        
        /* メモリ領域タイプ判定 */
        if ( pEntry->type != MK_MEM_TYPE_RESERVED ) {
            /* 予約済み領域でない */
            
            continue;
        }
        
        /* 未割当I/Oメモリ領域情報リスト設定 */
        pMemInfo = AreaSet( &( gIoTbl.freeList ),
                            pEntry->pAddr,
                            pEntry->size          );
        
        /* 設定結果判定 */
        if ( pMemInfo == NULL ) {
            /* 失敗 */
            
            /* [TODO] */
        }
    }
    
    return;
}


/******************************************************************************/