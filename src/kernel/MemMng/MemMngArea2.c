/******************************************************************************/
/* src/kernel/MemMng/MemMngArea.c                                             */
/*                                                                 2018/08/04 */
/* Copyright (C) 2017-2018 Mochi.                                             */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <firmware/bios/e820.h>
#include <kernel/kernel.h>
#include <MLib/MLib.h>
#include <MLib/Basic/MLibBasic.h>
#include <MLib/Basic/MLibBasicList.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <MemMng.h>

/* 内部モジュールヘッダ */


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_MEMMNG_AREA, \
                    __LINE__,               \
                    __VA_ARGS__ )
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

/** メモリ領域情報構造体 */
typedef struct {
    MLibBasicListNode_t node;   /**< 連結リストノード情報 */
    uint32_t            used;   /**< 使用フラグ           */
    void                *pAddr; /**< 先頭アドレス         */
    size_t              size;   /**< サイズ               */
    MkTaskId_t          taskId; /**< タスクID             */
} AreaInfo_t;

/** メモリ領域管理テーブル構造体 */
typedef struct {
    MkMemMapEntry_t *pMemMap;                   /**< メモリマップ              */
    size_t          memMapEntryNum;             /**< メモリマップエントリ数    */
    MLibBasicList_t allocList;                  /**< 割当中メモリ領域リスト    */
    MLibBasicList_t unallocList;                /**< 未割当メモリ領域リスト    */
    MLibBasicList_t usedList;                   /**< 使用中I/Oメモリ領域リスト */
    MLibBasicList_t unusedList;                 /**< 未使用I/Oメモリ領域リスト */
    MLibBasicList_t emptyList;                  /**< 空メモリ領域情報リスト    */
    AreaInfo_t      areaInfo[ AREA_INFO_NUM ];  /**< メモリ領域情報            */
} AreaTbl_t;


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** メモリ領域管理テーブル */
static AreaTbl_t gAreaTbl;


/******************************************************************************/
/* ローカル関数プロトタイプ宣言                                               */
/******************************************************************************/
/* I/Oメモリ領域割当(タイプ0) */
static void *AllocIOMemType0( AreaInfo_t *pUnused,
                              void       *pAddr,
                              size_t     size      );

/* I/Oメモリ領域割当(タイプ1) */
static void *AllocIOMemType1( AreaInfo_t *pUnused,
                              void       *pAddr,
                              size_t     size      );

/* I/Oメモリ領域割当(タイプ2) */
static void *AllocIOMemType2( AreaInfo_t *pUnused,
                              void       *pAddr,
                              size_t     size      );

/* I/Oメモリ領域割当(タイプ3) */
static void *AllocIOMemType3( AreaInfo_t *pUnused,
                              void       *pAddr,
                              size_t     size      );
/* 指定メモリ領域割当 */
static void *AllocMem( AreaInfo_t *pUnalloc );

/* 指定メモリ領域割当（一部） */
static void *AllocMemPartially( AreaInfo_t *pUnalloc,
                                size_t     size       );

/* 指定メモリ領域解放（結合） */
static CmnRet_t Free( MLibBasicList_t *pList,
                      AreaInfo_t      *pLink,
                      AreaInfo_t      *pFree  );

/* 指定メモリ領域解放（最後尾挿入） */
static CmnRet_t FreeAfterTail( MLibBasicList_t *pList,
                               AreaInfo_t      *pFree  );

/* 指定メモリ領域解放（前挿入） */
static CmnRet_t FreeBefore( MLibBasicList_t *pList,
                            AreaInfo_t      *pBase,
                            AreaInfo_t      *pFree  );

/* 割当中メモリ領域リスト初期化 */
static void InitAllocList( void );

/* 空メモリ領域情報リスト初期化 */
static void InitEmptyList( void );

/* 未割当メモリ領域リスト初期化 */
static void InitUnallocList( void );

/* 未使用I/Oメモリ領域リスト初期化 */
static void InitUnusedList( void );

/* 使用中I/Oメモリ領域リスト初期化 */
static void InitUsedList( void );


/******************************************************************************/
/* 外部モジュール向けグローバル関数定義                                       */
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
    void       *pAddr;      /* 割当メモリ領域先頭アドレス */
    AreaInfo_t *pUnalloc;   /* 未使用メモリ領域情報       */
    
    /* 初期化 */
    pAddr    = NULL;
    pUnalloc = NULL;
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. size=%#x", __func__, size );
    
    /* サイズチェック */
    if ( size == 0 ) {
        /* 不正 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=NULL", __func__ );
        
        return NULL;
        
    } else {
        /* 正常 */
        
        /* アライメント計算 */
        size = MLIB_BASIC_ALIGN( size, AREA_ALIGNMENT );
    }
    
    /* 割当可能な未割当メモリ領域検索 */
    do {
        /* 未割当メモリ領域情報取得 */
        pUnalloc = ( AreaInfo_t * )
            MLibBasicListGetNextNode( &( gAreaTbl.unallocList ),
                                      ( MLibBasicListNode_t * ) pUnalloc );
        
        /* 取得結果判定 */
        if ( pUnalloc == NULL ) {
            /* メモリ領域情報無 */
            
            /* デバッグトレースログ出力 */
            DEBUG_LOG( "%s() end. ret=NULL", __func__ );
            
            return NULL;
        }
        
        /* 割当可能判定 */
    } while ( pUnalloc->size < size );
    
    /* 未割当メモリ領域サイズ比較 */
    if ( pUnalloc->size == size ) {
        /* 必要サイズと一致 */
        
        /* 同サイズの未割当メモリ領域からメモリ割当 */
        pAddr = AllocMem( pUnalloc );
        
    } else {
        /* 必要サイズを超過 */
        
        /* 超過サイズの未割当メモリ領域からメモリ割当 */
        pAddr = AllocMemPartially( pUnalloc, size );
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%010p", __func__, pAddr );
    
    return pAddr;
}


/******************************************************************************/
/**
 * @brief       I/Oメモリ領域割当
 * @details     指定したI/Oメモリ領域を割り当てる。
 * 
 * @param[in]   *pAddr I/Oメモリ領域先頭アドレス
 * @param[in]   size   I/Oメモリ領域サイズ
 * 
 * @return      割当結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 割り当てたI/Oメモリ領域の先頭アドレス
 */
/******************************************************************************/
void *MemMngAreaAllocIO( void   *pAddr,
                         size_t size    )
{
    void       *pRet;       /* 戻り値                          */
    void       *pAddr1;     /* 未使用I/Oメモリ領域終端アドレス */
    void       *pAddr2;     /* 指定I/Oメモリ領域終端アドレス   */
    AreaInfo_t *pUnused;    /* 未使用メモリ領域情報            */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. *pAddr=%p, size=%#x", __func__, pAddr, size );
    
    /* 初期化 */
    pRet    = NULL;
    pUnused = NULL;
    
    /* サイズチェック */
    if ( size == 0 ) {
        /* 不正 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=NULL", __func__ );
        
        return NULL;
        
    } else {
        /* 正常 */
        
        /* サイズアライメント */
        size = MLIB_BASIC_ALIGN( size, AREA_ALIGNMENT );
    }
    
    /* 未使用メモリ領域情報取得 */
    pUnused = ( AreaInfo_t * )
        MLibBasicListGetNextNode( &( gAreaTbl.unusedList ),
                                  ( MLibBasicListNode_t * ) pUnused );
    
    /* 未使用メモリ領域情報毎に繰り返し */
    while ( pUnused != NULL ) {
        /* メモリ領域比較用アドレス計算 */
        pAddr1 = END_ADDR( pUnused->pAddr, pUnused->size );
        pAddr2 = END_ADDR( pAddr,          size          );
        
        /* 領域チェック */
        if ( ( pAddr < pUnused->pAddr                    ) ||
             ( ( pAddr < pAddr1 ) && ( pAddr1 < pAddr2 ) )    ) {
            /* 範囲外 */
            break;
        }
        
        /* 領域チェック */
        if ( pAddr1 <= pAddr ) {
            /* 次範囲 */
            
            /* 次メモリ領域情報取得 */
            pUnused = ( AreaInfo_t * )
                MLibBasicListGetNextNode( &( gAreaTbl.unusedList ),
                                          ( MLibBasicListNode_t * ) pUnused );
            continue;
        }
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( " pUnused->pAddr=%p, pUnused->size=%x",
                   pUnused->pAddr,
                   pUnused->size                           );
        
        /* 先頭アドレス比較 */
        if ( pUnused->pAddr == pAddr ) {
            /* 一致 */
            
            /* サイズ比較 */
            if ( pUnused->size == size ) {
                /* 一致 */
                
                /* メモリ領域情報設定 */
                return AllocIOMemType0( pUnused, pAddr, size );
                
            } else if ( pUnused->size > size ) {
                /* 不一致 */
                
                /* メモリ領域情報設定 */
                return AllocIOMemType1( pUnused, pAddr, size );
            }
            
            break;
            
        } else {
            /* 不一致 */
            
            /* メモリ領域比較 */
            if ( pAddr1 > pAddr2 ) {
                /* 中間メモリ領域 */
                
                /* メモリ領域情報設定 */
                return AllocIOMemType2( pUnused, pAddr, size );
                
            } else if ( pAddr1 == pAddr2 ) {
                /* 後方メモリ領域 */
                
                /* メモリ領域情報設定 */
                return AllocIOMemType3( pUnused, pAddr, size );
            }
        }
        
        /* 次メモリ領域情報取得 */
        pUnused = ( AreaInfo_t * )
            MLibBasicListGetNextNode( &( gAreaTbl.unusedList ),
                                      ( MLibBasicListNode_t * ) pUnused );
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%p", __func__, pRet );
    
    return pRet;
}


/******************************************************************************/
/**
 * @brief       メモリ領域情報取得
 * @details     指定したメモリ領域情報を取得する。
 * 
 * @param[out]  *pInfo メモリ領域情報
 * @param[in]   type   メモリ領域種別
 *                  - MK_MEM_TYPE_ACPI     ACPIメモリ領域
 *                  - MK_MEM_TYPE_ACPI_NVS ACPI NVSメモリ領域
 *                  - MK_MEM_TYPE_KERNEL   カーネル領域
 *                  - MK_MEM_TYPE_PROCIMG  プロセスイメージ領域
 * 
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 * 
 * @note        下記メモリ領域情報は取得不可。
 *              - 使用可能メモリ領域
 *              - 使用不可メモリ領域
 */
/******************************************************************************/
CmnRet_t MemMngAreaGetInfo( MemMngAreaInfo_t *pInfo,
                            uint32_t         type    )
{
    uint32_t        index;   /* インデックス         */
    MkMemMapEntry_t *pEntry; /* メモリマップエントリ */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. pInfo=%010p, type=%u", __func__, pInfo, type );
    
    /* メモリ領域情報チェック */
    if ( pInfo == NULL ) {
        /* 不正値 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );
        
        return CMN_FAILURE;
    }
    
    /* メモリ領域種別チェック */
    if ( ( type != BIOS_E820_TYPE_ACPI     ) &&
         ( type != BIOS_E820_TYPE_ACPI_NVS ) &&
         ( type != MK_MEM_TYPE_KERNEL      ) &&
         ( type != MK_MEM_TYPE_PROCIMG     )    ) {
        /* 不正値 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );
        
        return CMN_FAILURE;
    }
    
    /* メモリマップ全エントリ毎に繰り返し */
    for ( index = 0; index < gAreaTbl.memMapEntryNum; index++ ) {
        /* メモリマップ参照変数設定 */
        pEntry = &gAreaTbl.pMemMap[ index ];
        
        /* メモリ領域タイプ判定 */
        if ( pEntry->type == type ) {
            /* 一致 */
            
            /* メモリ領域情報設定 */
            pInfo->pAddr = pEntry->pAddr;
            pInfo->size  = pEntry->size;
            
            /* デバッグトレースログ出力 */
            DEBUG_LOG( "%s() end. ret=CMN_SUCCESS, index=%d", __func__ );
            
            return CMN_SUCCESS;
        }
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=CMN_FAILURE", __func__ );
    
    return CMN_FAILURE;
}


/******************************************************************************/
/**
 * @brief       割当中メモリ領域解放
 * @details     指定したメモリアドレスと等しい先頭アドレスの割当中メモリ領域を
 *              解放する。
 * 
 * @param[in]   *pAddr 解放するメモリアドレス
 * 
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
CmnRet_t MemMngAreaFree( void *pAddr )
{
    CmnRet_t   ret;         /* 関数戻り値           */
    MLibRet_t  retMLib;     /* MLib関数戻り値       */
    AreaInfo_t *pUnalloc;   /* 未割当メモリ領域情報 */
    AreaInfo_t *pInfo;      /* 割当中メモリ領域情報 */
    
    /* 初期化 */
    ret      = CMN_FAILURE;
    retMLib  = MLIB_FAILURE;
    pUnalloc = NULL;
    pInfo    = NULL;
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. pAddr=%010p", __func__, pAddr );
    
    /* 引数チェック */
    if ( pAddr == NULL ) {
        /* 不正 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=%d", __func__, CMN_FAILURE );
        
        return CMN_FAILURE;
    }
    
    /* 該当メモリ領域情報検索 */
    do {
        /* 割当中メモリ領域情報取得 */
        pInfo = ( AreaInfo_t * )
            MLibBasicListGetPrevNode( &( gAreaTbl.allocList ),
                                      ( MLibBasicListNode_t * ) pInfo );
        
        /* 取得結果判定 */
        if ( pInfo == NULL ) {
            /* メモリ領域情報無 */
            
            /* デバッグトレースログ出力 */
            DEBUG_LOG( "%s() end. ret=%d", __func__, CMN_FAILURE );
            
            return CMN_FAILURE;
        }
        
        /* 先頭アドレス比較 */
    } while ( pInfo->pAddr != pAddr );
    
    /* 割当中メモリ領域リストから削除 */
    retMLib = MLibBasicListRemove( &( gAreaTbl.allocList ),
                                   ( MLibBasicListNode_t * ) pInfo );
    
    /* 削除結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=%d", __func__, CMN_FAILURE );
        
        return CMN_FAILURE;
    }
    
    while ( true ) {
        /* 未割当メモリ領域情報取得 */
        pUnalloc = ( AreaInfo_t * )
            MLibBasicListGetNextNode( &( gAreaTbl.unallocList ),
                                      ( MLibBasicListNode_t * ) pUnalloc );
        
        /* メモリ領域位置関係比較 */
        if ( pUnalloc == NULL ) {
            /* メモリ領域情報無 */
            
            /* 未割当メモリ領域リストの最後尾に挿入 */
            ret = FreeAfterTail( &( gAreaTbl.unallocList ), pInfo );
            
        } else if ( ( pInfo->pAddr + pInfo->size ) < pUnalloc->pAddr ) {
            /* 未割当メモリ領域より前 */
            
            /* 前に挿入 */
            ret = FreeBefore( &( gAreaTbl.unallocList ), pUnalloc, pInfo );
            
        } else if ( ( pUnalloc->pAddr + pUnalloc->size ) < pInfo->pAddr ) {
            /* 未割当メモリ領域より後 */
            
            /* 次の未割当メモリ領域 */
            continue;
            
        } else {
            /* 未割当メモリ領域に隣接 */
            
            /* 結合 */
            ret = Free( &( gAreaTbl.unallocList ), pUnalloc, pInfo );
        }
        
        break;
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%d", __func__, ret );
    
    return ret;
}


/******************************************************************************/
/**
 * @brief       使用中I/Oメモリ領域解放
 * @details     使用中I/Oメモリアドレスメモリ領域を解放し、未使用状態にする。
 * 
 * @param[in]   *pAddr 解放するI/Oメモリアドレス
 * 
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
CmnRet_t MemMngAreaFreeIO( void *pAddr )
{
    CmnRet_t   ret;         /* 関数戻り値              */
    MLibRet_t  retMLib;     /* MLib関数戻り値          */
    AreaInfo_t *pUnused;    /* 未使用I/Oメモリ領域情報 */
    AreaInfo_t *pInfo;      /* 使用中I/Oメモリ領域情報 */
    
    /* 初期化 */
    ret     = CMN_FAILURE;
    retMLib = MLIB_FAILURE;
    pUnused = NULL;
    pInfo   = NULL;
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. pAddr=%010p", __func__, pAddr );
    
    /* 引数チェック */
    if ( pAddr == NULL ) {
        /* 不正 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=%d", __func__, CMN_FAILURE );
        
        return CMN_FAILURE;
    }
    
    /* 該当メモリ領域情報検索 */
    do {
        /* 使用中I/Oメモリ領域情報取得 */
        pInfo = ( AreaInfo_t * )
            MLibBasicListGetPrevNode( &( gAreaTbl.usedList ),
                                      ( MLibBasicListNode_t * ) pInfo );
        
        /* 取得結果判定 */
        if ( pInfo == NULL ) {
            /* メモリ領域情報無 */
            
            /* デバッグトレースログ出力 */
            DEBUG_LOG( "%s() end. ret=%d", __func__, CMN_FAILURE );
            
            return CMN_FAILURE;
        }
        
        /* 先頭アドレス比較 */
    } while ( pInfo->pAddr != pAddr );
    
    /* 使用中I/Oメモリ領域リストから削除 */
    retMLib = MLibBasicListRemove( &( gAreaTbl.usedList ),
                                   ( MLibBasicListNode_t * ) pInfo );
    
    /* 削除結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=%d", __func__, CMN_FAILURE );
        
        return CMN_FAILURE;
    }
    
    while ( true ) {
        /* 未使用I/Oメモリ領域情報取得 */
        pUnused = ( AreaInfo_t * )
            MLibBasicListGetNextNode( &( gAreaTbl.unusedList ),
                                      ( MLibBasicListNode_t * ) pUnused );
        
        /* メモリ領域位置関係比較 */
        if ( pUnused == NULL ) {
            /* メモリ領域情報無 */
            
            /* 未使用I/Oメモリ領域リストの最後尾に挿入 */
            ret = FreeAfterTail( &( gAreaTbl.unusedList ), pInfo );
            
        } else if ( ( pInfo->pAddr + pInfo->size ) < pUnused->pAddr ) {
            /* 未使用I/Oメモリ領域より前 */
            
            /* 前に挿入 */
            ret = FreeBefore( &( gAreaTbl.unallocList ), pUnused, pInfo );
            
        } else if ( ( pUnused->pAddr + pUnused->size ) < pInfo->pAddr ) {
            /* 未使用I/Oメモリ領域より後 */
            
            /* 次の未割当メモリ領域 */
            continue;
            
        } else {
            /* 未使用I/Oメモリ領域に隣接 */
            
            /* 結合 */
            ret = Free( &( gAreaTbl.unallocList ), pUnused, pInfo );
        }
        
        break;
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%d", __func__, ret );
    
    return ret;
}


/******************************************************************************/
/* 内部モジュール向けグローバル関数定義                                       */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メモリ領域管理初期化
 * @details     メモリ領域管理テーブルを初期化する。
 * 
 * @param[in]   *pMemMap       メモリマップ
 * @param[in]   memMapEntryNum メモリマップエントリ数
 */
/******************************************************************************/
void MemMngAreaInit( MkMemMapEntry_t *pMemMap,
                     size_t          memMapEntryNum )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. map=%p, num=%d",
               __func__,
               pMemMap,
               memMapEntryNum                );
    
    /* メモリマップ設定 */
    gAreaTbl.pMemMap        = pMemMap;
    gAreaTbl.memMapEntryNum = memMapEntryNum;
    
    /* 空メモリ領域情報情報リスト初期化 */
    InitEmptyList();
    
    /* 未使用メモリ領域情報リスト初期化 */
    InitUnusedList();
    
    /* 使用中メモリ領域リスト初期化 */
    InitUsedList();
    
    /* 未割当I/Oメモリ領域情報リスト初期化 */
    InitUnallocList();
    
    /* 割当中I/Oメモリ領域情報リスト初期化 */
    InitAllocList();
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       I/Oメモリ領域割当(タイプ0)
 * @details     指定した未使用I/Oメモリ領域を割り当てる。
 * 
 * @param[in]   *pUnused 未使用I/Oメモリ領域情報
 * @param[in]   *pAddr   I/Oメモリ領域先頭アドレス
 * @param[in]   size     I/Oメモリ領域サイズ
 * 
 * @return      割当結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 割り当てたI/Oメモリ領域の先頭アドレス
 */
/******************************************************************************/
static void *AllocIOMemType0( AreaInfo_t *pUnused,
                              void       *pAddr,
                              size_t     size      )
{
    MLibRet_t retMLib;  /* 関数戻り値 */
    
    /* 未使用I/Oメモリ領域情報リスト削除 */
    retMLib = MLibBasicListRemove( &( gAreaTbl.unusedList ),
                                   ( MLibBasicListNode_t * ) pUnused );
    
    /* 削除結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=NULL", __func__ );
        
        return NULL;
    }
    
    /* 使用中I/Oメモリ領域情報リスト挿入 */
    retMLib = MLibBasicListInsertTail( &( gAreaTbl.usedList ),
                                       ( MLibBasicListNode_t * ) pUnused );
    
    /* 挿入結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=NULL", __func__ );
        
        return NULL;
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%p", __func__, pAddr );
    
    return pAddr;
}


/******************************************************************************/
/**
 * @brief       I/Oメモリ領域割当(タイプ1)
 * @details     指定した未使用I/Oメモリ領域の前方を分割してI/Oメモリ領域を割り
 *              当てる。
 * 
 * @param[in]   *pUnused 未使用I/Oメモリ領域情報
 * @param[in]   *pAddr   I/Oメモリ領域先頭アドレス
 * @param[in]   size     I/Oメモリ領域サイズ
 * 
 * @return      割当結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 割り当てたI/Oメモリ領域の先頭アドレス
 */
/******************************************************************************/
static void *AllocIOMemType1( AreaInfo_t *pUnused,
                              void       *pAddr,
                              size_t     size      )
{
    MLibRet_t  retMLib;     /* MLib関数戻り値   */
    AreaInfo_t *pEmpty;     /* 空メモリ領域情報 */
    
    /* 空メモリ領域情報取得 */
    pEmpty = ( AreaInfo_t * )
        MLibBasicListRemoveTail( &( gAreaTbl.emptyList ) );
    
    /* 取得結果判定 */
    if ( pEmpty == NULL ) {
        /* 失敗 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=NULL", __func__ );
        
        return NULL;
    }
    
    /* I/Oメモリ領域情報設定 */
    pEmpty->pAddr   = pAddr;
    pEmpty->size    = size;
    pUnused->pAddr  = END_ADDR( pAddr, size );
    pUnused->size  -= size;
    
    /* 使用中I/Oメモリ領域情報リスト挿入 */
    retMLib = MLibBasicListInsertTail( &( gAreaTbl.usedList ),
                                       ( MLibBasicListNode_t * ) pEmpty );
    
    /* 挿入結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=NULL", __func__ );
        
        return NULL;
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%p", __func__, pAddr );
    
    return pAddr;
}


/******************************************************************************/
/**
 * @brief       I/Oメモリ領域割当(タイプ2)
 * @details     指定した未使用I/Oメモリ領域の中間を分割してI/Oメモリ領域を割り
 *              当てる。
 * 
 * @param[in]   *pUnused 未使用I/Oメモリ領域情報
 * @param[in]   *pAddr   I/Oメモリ領域先頭アドレス
 * @param[in]   size     I/Oメモリ領域サイズ
 * 
 * @return      割当結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 割り当てたI/Oメモリ領域の先頭アドレス
 */
/******************************************************************************/
static void *AllocIOMemType2( AreaInfo_t *pUnused,
                              void       *pAddr,
                              size_t     size      )
{
    void   *pRet;   /* 戻り値               */
    size_t size1;   /* 前方メモリ領域サイズ */
    
    /* 初期化 */
    size1 = ( size_t ) pAddr - ( size_t ) pUnused->pAddr;
    
    /* 前方分割 */
    pRet = AllocIOMemType1( pUnused,
                            pUnused->pAddr,
                            size1           );
    
    /* 分割結果判定 */
    if ( pRet != CMN_SUCCESS ) {
        /* 失敗 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=NULL", __func__ );
        
        return NULL;
    }
    
    /* 中間分割 */
    pRet = AllocIOMemType1( pUnused, pAddr, size );
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%p", __func__, pRet );
    
    return pRet;
}


/******************************************************************************/
/**
 * @brief       I/Oメモリ領域割当(タイプ3)
 * @details     指定した未使用I/Oメモリ領域の後方を分割してI/Oメモリ領域を割り
 *              当てる。
 * 
 * @param[in]   *pUnused 未使用I/Oメモリ領域情報
 * @param[in]   *pAddr   I/Oメモリ領域先頭アドレス
 * @param[in]   size     I/Oメモリ領域サイズ
 * 
 * @return      割当結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 割り当てたI/Oメモリ領域の先頭アドレス
 */
/******************************************************************************/
static void *AllocIOMemType3( AreaInfo_t *pUnused,
                              void       *pAddr,
                              size_t     size      )
{
    MLibRet_t  retMLib; /* MLib関数戻り値   */
    AreaInfo_t *pEmpty; /* 空メモリ領域情報 */
    
    /* 空メモリ領域情報取得 */
    pEmpty = ( AreaInfo_t * )
        MLibBasicListRemoveTail( &( gAreaTbl.emptyList ) );
    
    /* 取得結果判定 */
    if ( pEmpty == NULL ) {
        /* 失敗 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=NULL", __func__ );
        
        return NULL;
    }
    
    /* I/Oメモリ領域情報設定 */
    pUnused->size -= size;
    pEmpty->pAddr  = pAddr;
    pEmpty->size   = size;
    
    /* 使用中I/Oメモリ領域情報挿入 */
    retMLib = MLibBasicListInsertTail( &( gAreaTbl.usedList ),
                                       ( MLibBasicListNode_t * ) pEmpty );
    
    /* 挿入結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=NULL", __func__ );
        
        return NULL;
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%p", __func__, pAddr );
    
    return pAddr;
}


/******************************************************************************/
/**
 * @brief       指定メモリ領域割当
 * @details     指定した未割当メモリ領域を割り当てる。
 * 
 * @param[in]   *pUnalloc 未割当メモリ領域情報
 * 
 * @retval      NULL     失敗
 * @retval      NULL以外 割当メモリ領域の先頭アドレス
 */
/******************************************************************************/
static void *AllocMem( AreaInfo_t *pUnalloc )
{
    MLibRet_t retMLib;  /* MLib関数戻り値 */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. pUnalloc=%010p", __func__, pUnalloc );
    
    /* 未割当メモリ領域リストから削除 */
    retMLib = MLibBasicListRemove( &( gAreaTbl.unallocList ),
                                   ( MLibBasicListNode_t * ) pUnalloc );
    
    /* 削除結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=NULL", __func__ );
        
        return NULL;
    }
    
    /* 割当中メモリ領域リストに追加 */
    retMLib = MLibBasicListInsertTail( &( gAreaTbl.allocList ),
                                       ( MLibBasicListNode_t * ) pUnalloc );
    
    /* 追加結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=NULL", __func__ );
        
        return NULL;
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%010p", __func__, pUnalloc->pAddr );
    
    /* メモリ領域先頭アドレス返却 */
    return pUnalloc->pAddr;
}


/******************************************************************************/
/**
 * @brief       指定メモリ領域割当（一部）
 * @details     指定した未割当メモリ領域から指定した領域サイズのメモリ領域を割
 *              り当てる。
 * 
 * @param[in]   *pUnalloc 未割当メモリ領域情報
 * @param[in]   size      割当てサイズ
 * 
 * @retval      NULL     失敗
 * @retval      NULL以外 割当メモリ領域の先頭アドレス
 */
/******************************************************************************/
static void *AllocMemPartially( AreaInfo_t *pUnalloc,
                                size_t     size       )
{
    MLibRet_t  retMLib;     /* MLib関数戻り値   */
    AreaInfo_t *pEmpty;     /* 空メモリ領域情報 */
    
    /* 初期化 */
    retMLib = MLIB_FAILURE;
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. pUnalloc=%010p, size=%#x",
               __func__,
               pUnalloc,
               size                                    );
    
    /* 空メモリ領域情報リストからメモリ領域情報取得 */
    pEmpty = ( AreaInfo_t * )
        MLibBasicListRemoveTail( &( gAreaTbl.emptyList ) );
    
    /* 取得結果判定 */
    if ( pEmpty == NULL ) {
        /* メモリ領域情報無 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=NULL", __func__ );
        
        return NULL;
    }
    
    /* 空メモリ領域情報設定 */
    pEmpty->used  = CMN_USED;           /* 使用フラグ   */
    pEmpty->pAddr = pUnalloc->pAddr;    /* 先頭アドレス */
    pEmpty->size  = size;               /* サイズ       */
    
    /* 空メモリ領域情報を割当中メモリ領域リストに追加 */
    retMLib = MLibBasicListInsertTail( &( gAreaTbl.allocList ),
                                       ( MLibBasicListNode_t * ) pEmpty );
    
    /* 追加結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=NULL", __func__ );
        
        return NULL;
    }
    
    /* 未割当メモリ領域情報設定 */
    pUnalloc->pAddr += size;    /* 先頭アドレス */
    pUnalloc->size  -= size;    /* サイズ       */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%010p", __func__, pEmpty->pAddr );
    
    /* メモリ領域先頭アドレス返却 */
    return pEmpty->pAddr;
}


/******************************************************************************/
/**
 * @brief       メモリ領域解放（結合）
 * @details     メモリ領域を結合先メモリ領域に結合する。
 * 
 * @param[in]   *pList メモリ領域リスト
 * @param[in]   *pLink 結合先メモリ領域情報
 * @param[in]   *pFree 解放メモリ領域
 * 
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
static CmnRet_t Free( MLibBasicList_t *pList,
                      AreaInfo_t      *pLink,
                      AreaInfo_t      *pFree  )
{
    size_t     size;    /* 結合先メモリ領域サイズ             */
    MLibRet_t  retMLib; /* MLib関数戻り値                     */
    AreaInfo_t *pNext;  /* 結合先メモリ領域の次メモリ領域情報 */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. pList=%010p, pLink=%010p, pFree=%010p",
               __func__,
               pList,
               pLink,
               pFree                                                );
    
    /* 初期化 */
    size    = pFree->size;
    retMLib = MLIB_FAILURE;
    pNext   = NULL;
    
    /* メモリ領域位置関係比較 */
    if ( pFree->pAddr < pLink->pAddr ) {
        /* 解放メモリ領域が結合先メモリ領域の前 */
        
        /* 結合先メモリ領域先頭アドレス設定 */
        pLink->pAddr = pFree->pAddr;
    }
    
    /* 結合先メモリ領域サイズ設定 */
    pLink->size += pFree->size;
    
    /* 解放メモリ領域情報初期化 */
    memset( pFree, 0, sizeof ( AreaInfo_t ) );
    
    /* 解放メモリ領域情報を空メモリ領域情報リストの最後尾に挿入 */
    retMLib = MLibBasicListInsertTail( &( gAreaTbl.emptyList ),
                                       ( MLibBasicListNode_t * ) pFree );
    
    /* 挿入結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        /* [TODO]トレース情報 */
    }
    
    /* 結合先メモリ領域の次のメモリ領域情報取得 */
    pNext = ( AreaInfo_t * )
        MLibBasicListGetNextNode( pList, ( MLibBasicListNode_t * ) pLink );
    
    /* 取得結果判定 */
    if ( pNext == NULL ) {
        /* メモリ領域情報無 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=%d", __func__, CMN_SUCCESS );
        
        return CMN_SUCCESS;
    }
    
    /* メモリ領域隣接判定 */
    if ( ( pLink->pAddr + pLink->size ) == pNext->pAddr ) {
        /* 隣接する */
        
        /* 次メモリ領域情報をメモリ領域リストから削除 */
        retMLib = MLibBasicListRemove( pList, ( MLibBasicListNode_t * ) pNext );
        
        /* 削除結果判定 */
        if ( retMLib != MLIB_SUCCESS ) {
            /* 失敗 */
            
            /* 結合先メモリ領域サイズ復元 */
            pLink->size = size;
            
            /* デバッグトレースログ出力 */
            DEBUG_LOG( "%s() end. ret=%d", __func__, CMN_FAILURE );
            
            return CMN_FAILURE;
        }
        
        /* 結合先メモリ領域サイズ設定 */
        pLink->size += pNext->size;
        
        /* 次メモリ領域情報初期化 */
        memset( pNext, 0, sizeof ( AreaInfo_t ) );
        
        /* 次メモリ領域情報を空メモリ領域情報リストの最後尾に挿入 */
        retMLib = MLibBasicListInsertTail( &( gAreaTbl.emptyList ),
                                           ( MLibBasicListNode_t * ) pFree );
        
        /* 挿入結果判定 */
        if ( retMLib != MLIB_SUCCESS ) {
            /* 失敗 */
            
            /* [TODO]トレース情報 */
        }
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%d", __func__, CMN_SUCCESS );
    
    return CMN_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       メモリ領域解放（最後尾挿入）
 * @details     解放するメモリ領域をメモリ領域リストの最後尾に挿入する。
 * 
 * @param[in]   *pList メモリ領域リスト
 * @param[in]   *pFree 解放メモリ領域情報
 * 
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
static CmnRet_t FreeAfterTail( MLibBasicList_t *pList,
                               AreaInfo_t      *pFree  )
{
    MLibRet_t retMLib;  /* MLib関数戻り値 */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. pFree=%010p", __func__, pFree );
    
    /* 最後尾挿入 */
    retMLib = MLibBasicListInsertTail( pList, ( MLibBasicListNode_t * ) pFree );
    
    /* 挿入結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=%d", __func__, CMN_FAILURE );
        
        return CMN_FAILURE;
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%d", __func__, CMN_SUCCESS );
    
    return CMN_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       メモリ領域解放（前挿入）
 * @details     解放するメモリ領域を指定したメモリ領域の前に挿入する。
 * 
 * @param[in]   *pList メモリ領域リスト
 * @param[in]   *pBase 指定メモリ領域情報
 * @param[in]   *pFree 解放するメモリ領域情報
 * 
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
static CmnRet_t FreeBefore( MLibBasicList_t *pList,
                            AreaInfo_t      *pBase,
                            AreaInfo_t      *pFree  )
{
    MLibRet_t retMLib;  /* MLib関数戻り値 */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. pBase=%010p, pFree=%010p",
               __func__,
               pBase,
               pFree                                   );
    
    /* 指定した未割当メモリ領域の前に挿入 */
    retMLib = MLibBasicListInsertPrev( pList,
                                       ( MLibBasicListNode_t * ) pBase,
                                       ( MLibBasicListNode_t * ) pFree  );
    
    /* 挿入結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=%d", __func__, CMN_FAILURE );
        
        return CMN_FAILURE;
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%d", __func__, CMN_SUCCESS );
    
    return CMN_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       割当中メモリ領域リスト初期化
 * @details     割当中メモリ領域リストを初期化する。
 */
/******************************************************************************/
static void InitAllocList( void )
{
    MLibRet_t retMLib;  /* MLIB関数戻り値 */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* 使用中メモリ領域リスト初期化 */
    retMLib = MLibBasicListInit( &( gAreaTbl.allocList ) );
    
    /* 初期化結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        /* [TODO]カーネルパニック */
        DEBUG_LOG( "ERROR!!! retMLib=%d", retMLib );
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
/**
 * @brief       空メモリ領域情報リスト初期化
 * @details     空メモリ領域情報リストを初期化する。
 */
/******************************************************************************/
static void InitEmptyList( void )
{
    uint32_t            index;      /* メモリ領域情報リストインデックス */
    MLibRet_t           retMLib;    /* MLIB関数戻り値                   */
    MLibBasicListNode_t *pEmpty;    /* 空メモリ領域情報ノード           */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* 空メモリ領域情報リスト初期化 */
    retMLib = MLibBasicListInit( &( gAreaTbl.emptyList ) );
    
    /* 初期化結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        /* [TODO]カーネルパニック */
        DEBUG_LOG( "ERROR!!! retMLib=%d", retMLib );
    }
    
    /* メモリ領域情報初期化 */
    memset( gAreaTbl.areaInfo, 0, sizeof ( gAreaTbl.areaInfo ) );
    
    for ( index = 0; index < AREA_INFO_NUM; index++ ) {
        /* メモリ領域情報設定 */
        gAreaTbl.areaInfo[ index ].taskId = MK_CONFIG_TASKID_NULL;
        
        /* 空メモリ領域情報参照変数設定 */
        pEmpty = ( MLibBasicListNode_t * ) &( gAreaTbl.areaInfo[ index ] );
        
        /* 空メモリ領域情報リスト挿入 */
        retMLib = MLibBasicListInsertTail( &( gAreaTbl.emptyList ),
                                           pEmpty );
        
        /* 挿入結果判定 */
        if ( retMLib != MLIB_SUCCESS ) {
            /* 失敗 */
            
            /* [TODO]カーネルパニック */
            DEBUG_LOG( "ERROR!!! retMLib=%d", retMLib );
            
        }
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
/**
 * @brief       未割当メモリ領域リスト初期化
 * @details     未割当メモリ領域リストをメモリマップを基に初期化する。
 * 
 * @attention   空メモリ領域情報リストを初期化済みである事。
 */
/******************************************************************************/
static void InitUnallocList( void )
{
    uint32_t        index;      /* インデックス           */
    MLibRet_t       retMLib;    /* MLIB関数戻り値         */
    AreaInfo_t      *pEmpty;    /* 空メモリ領域情報       */
    MkMemMapEntry_t *pEntry;    /* メモリマップエントリ   */
    
    /* 初期化 */
    index   = 0;
    retMLib = MLIB_FAILURE;
    pEmpty  = NULL;
    pEntry  = NULL;
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* 未使用メモリ領域リスト初期化 */
    MLibBasicListInit( &( gAreaTbl.unallocList ) );
    
    /* メモリマップ全エントリ毎に繰り返し */
    for ( index = 0; index < gAreaTbl.memMapEntryNum; index++ ) {
        /* メモリマップ参照変数設定 */
        pEntry = &gAreaTbl.pMemMap[ index ];
        
        /* メモリ領域タイプ判定 */
        if ( pEntry->type != MK_MEM_TYPE_AVAILABLE ) {
            /* 使用可能メモリ領域でない */
            
            /* 次のメモリマップ */
            continue;
        }
        
        /* 空メモリ情報領域リストからメモリ領域情報取得 */
        pEmpty = ( AreaInfo_t * )
            MLibBasicListRemoveTail( &( gAreaTbl.emptyList ) );
        
        /* 取得結果判定 */
        if ( pEmpty == NULL ) {
            /* 失敗 */
            
            break;
        }
        
        /* メモリ領域情報設定 */
        pEmpty->used  = CMN_USED;       /* 使用フラグ   */
        pEmpty->pAddr = pEntry->pAddr;  /* 先頭アドレス */
        pEmpty->size  = pEntry->size;   /* サイズ       */
        
        /* 未使用メモリ領域リストの最後尾に挿入 */
        retMLib = MLibBasicListInsertTail( &( gAreaTbl.unallocList ),
                                           ( MLibBasicListNode_t * ) pEmpty );
        
        /* 挿入結果判定 */
        if ( retMLib != MLIB_SUCCESS ) {
            /* 失敗 */
            
            /* [TODO]トレースログ */
        }
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
/**
 * @brief       未使用I/Oメモリ領域リスト初期化
 * @details     未使用I/Oメモリ領域リストをメモリマップを基に初期化する。
 * 
 * @attention   空メモリ領域情報リストを初期化済みである事。
 */
/******************************************************************************/
static void InitUnusedList( void )
{
    uint32_t        index;      /* インデックス           */
    MLibRet_t       retMLib;    /* MLIB関数戻り値         */
    AreaInfo_t      *pEmpty;    /* 空メモリ領域情報       */
    MkMemMapEntry_t *pEntry;    /* メモリマップエントリ   */
    
    /* 初期化 */
    index   = 0;
    retMLib = MLIB_FAILURE;
    pEmpty  = NULL;
    pEntry  = NULL;
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* 未使用メモリ領域リスト初期化 */
    MLibBasicListInit( &( gAreaTbl.unusedList ) );
    
    /* メモリマップ全エントリ毎に繰り返し */
    for ( index = 0; index < gAreaTbl.memMapEntryNum; index++ ) {
        /* メモリマップ参照変数設定 */
        pEntry = &gAreaTbl.pMemMap[ index ];
        
        /* メモリ領域タイプ判定 */
        if ( pEntry->type == MK_MEM_TYPE_AVAILABLE ) {
            /* 使用可能メモリ領域 */
            
            /* 次のメモリマップ */
            continue;
        }
        
        /* 空メモリ情報領域リストからメモリ領域情報取得 */
        pEmpty = ( AreaInfo_t * )
            MLibBasicListRemoveTail( &( gAreaTbl.emptyList ) );
        
        /* 取得結果判定 */
        if ( pEmpty == NULL ) {
            /* 失敗 */
            
            break;
        }
        
        /* メモリ領域情報設定 */
        pEmpty->used  = CMN_USED;       /* 使用フラグ   */
        pEmpty->pAddr = pEntry->pAddr;  /* 先頭アドレス */
        pEmpty->size  = pEntry->size;   /* サイズ       */
        
        /* 未使用メモリ領域リストの最後尾に挿入 */
        retMLib = MLibBasicListInsertTail( &( gAreaTbl.unusedList ),
                                           ( MLibBasicListNode_t * ) pEmpty );
        
        /* 挿入結果判定 */
        if ( retMLib != MLIB_SUCCESS ) {
            /* 失敗 */
            
            /* [TODO]トレースログ */
        }
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
/**
 * @brief       使用中I/Oメモリ領域リスト初期化
 * @details     使用中I/Oメモリ領域リストを初期化する。
 */
/******************************************************************************/
static void InitUsedList( void )
{
    MLibRet_t retMLib;  /* MLIB関数戻り値 */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* 使用中メモリ領域リスト初期化 */
    retMLib = MLibBasicListInit( &( gAreaTbl.usedList ) );
    
    /* 初期化結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
        /* 失敗 */
        
        /* [TODO]カーネルパニック */
        DEBUG_LOG( "ERROR!!! retMLib=%d", retMLib );
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
