/******************************************************************************/
/* src/kernel/MemMng/MemMngPhys.c                                             */
/*                                                                 2018/08/16 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdarg.h>
#include <hardware/IA32/IA32Paging.h>
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
    DebugLogOutput( CMN_MODULE_MEMMNG_PHYS, \
                    __LINE__,               \
                    __VA_ARGS__ )
#else
#define DEBUG_LOG( ... )
#endif

/** 物理メモリ領域管理テーブル構造体 */
typedef struct {
    MLibBasicList_t allocList;  /**< 割当中物理メモリ領域情報リスト */
    MLibBasicList_t freeList;   /**< 未割当物理メモリ領域情報リスト */
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
 * @return      割当結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 割り当てた物理メモリ領域の先頭アドレス
 * 
 * @note        割当サイズが4Kバイトアライメントでない場合は、割当サイズが4Kバ
 *              イトアライメントに合うよう加算して、物理メモリ領域を割り当てる。
 */
/******************************************************************************/
void *MemMngPhysAlloc( size_t size )
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
        size = MLIB_BASIC_ALIGN( size, IA32_PAGING_PAGE_SIZE );
    }
    
    /* メモリ領域割当 */
    pRet = AreaAlloc( &( gPhysTbl.allocList ), &( gPhysTbl.freeList ), size );
    
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
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
CmnRet_t MemMngPhysFree( void *pAddr )
{
    CmnRet_t ret;   /* 戻り値 */
    
    /* メモリ領域解放 */
    ret = AreaFree( &( gPhysTbl.allocList ), &( gPhysTbl.freeList ), pAddr );
    
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
    uint32_t        index;      /* メモリマップエントリインデックス */
    AreaInfo_t      *pMemInfo;  /* 未割当メモリ領域情報             */
    MkMemMapEntry_t *pEntry;    /* メモリマップエントリ             */
    
    /* 未割当物理メモリ領域情報リスト初期化 */
    MLibBasicListInit( &( gPhysTbl.freeList ) );
    
    /* 割当中物理メモリ領域情報リスト初期化 */
    MLibBasicListInit( &( gPhysTbl.allocList ) );
    
    /* メモリマップエントリ毎に繰り返し */
    for ( index = 0; index < entryNum; index++ ) {
        /* メモリマップ参照設定 */
        pEntry = &( pMemMap[ index ] );
        
        /* メモリ領域タイプ判定 */
        if ( pEntry->type != MK_MEM_TYPE_AVAILABLE ) {
            /* 使用可能メモリ領域でない */
            
            continue;
        }
        
        /* 未割当メモリ領域情報リスト設定 */
        pMemInfo = AreaSet( &( gPhysTbl.freeList ),
                            pEntry->pAddr,
                            pEntry->size            );
        
        /* 設定結果判定 */
        if ( pMemInfo == NULL ) {
            /* 失敗 */
            
            /* [TODO] */
        }
    }
    
    return;
}


/******************************************************************************/
