/******************************************************************************/
/* src/kernel/MemMng/MemMngVirt.c                                             */
/*                                                                 2018/12/09 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdarg.h>
#include <hardware/IA32/IA32Paging.h>
#include <kernel/kernel.h>
#include <MLib/MLib.h>
#include <MLib/MLibList.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <MemMng.h>

/* 内部モジュールヘッダ */
#include "MemMngArea.h"
#include "MemMngVirt.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_MEMMNG_VIRT, \
                    __LINE__,               \
                    __VA_ARGS__             )
#else
#define DEBUG_LOG( ... )
#endif

/** 仮想メモリ領域管理テーブル構造体 */
typedef struct {
    MLibList_t allocList;   /**< 割当中物理メモリ領域情報リスト */
    MLibList_t freeList;    /**< 未割当物理メモリ領域情報リスト */
} VirtTbl_t;


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** 仮想メモリ領域管理テーブル */
static VirtTbl_t gVirtTbl[ MK_CONFIG_PID_MAX ];


/******************************************************************************/
/* 外部モジュール向けグローバル関数定義                                       */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       仮想メモリ領域割当
 * @details     指定サイズを満たす仮想メモリ領域を割り当てる。
 * 
 * @param[in]   pid  プロセスID
 * @param[in]   size 割当サイズ
 * 
 * @return      割当結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 割り当てた仮想メモリ領域の先頭アドレス
 * 
 * @note        割当サイズが4Kバイトアライメントでない場合は、割当サイズが4Kバ
 *              イトアライメントに合うよう加算して、仮想メモリ領域を割り当てる。
 */
/******************************************************************************/
void *MemMngVirtAlloc( MkPid_t pid,
                       size_t  size )
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
        size = MLIB_ALIGN( size, IA32_PAGING_PAGE_SIZE );
    }
    
    /* メモリ領域割当 */
    pRet = AreaAlloc( &( gVirtTbl[ pid ].allocList ),
                      &( gVirtTbl[ pid ].freeList  ),
                      size                            );
    
    return pRet;
}


/******************************************************************************/
/**
 * @brief       指定仮想メモリ領域割当
 * @details     指定したアドレスとサイズの仮想メモリ領域を割り当てる。
 * 
 * @param[in]   pid    プロセスID
 * @param[in]   *pAddr 領域先頭アドレス
 * @param[in]   size   領域サイズ
 * 
 * @return      割当結果を返す。
 * @retval      NULL    失敗
 * @retval      NULL以外 割り当てた仮想メモリの先頭アドレス
 */
/******************************************************************************/
void *MemMngVirtAllocSpecified( MkPid_t pid,
                                void    *pAddr,
                                size_t  size    )
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
    pRet = AreaAllocSpecified( &( gVirtTbl[ pid ].allocList ),
                               &( gVirtTbl[ pid ].freeList  ),
                               pAddr,
                               size                            );
    
    return pRet;
}


/******************************************************************************/
/**
 * @brief       仮想メモリ領域管理終了
 * @details     プロセス毎の仮想メモリ領域管理を終了する。
 * 
 * @param[in]   pid プロセスID
 * 
 * @return      初期化結果を判定する。
 * @retval      CMN_SUCCESS 成功
 * @retval      CMN_FAILURE 失敗
 */
/******************************************************************************/
CmnRet_t MemMngVirtEnd( MkPid_t pid )
{
    /* [TODO] */
    
    return CMN_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       仮想メモリ領域解放
 * @details     割当中の仮想メモリ領域を解放する。
 * 
 * @param[in]   pid    プロセスID
 * @param[in]   *pAddr 解放するメモリアドレス
 * 
 * @return      解放結果を返す。
 * @retval      CMN_SUCCESS 正常終了
 * @retval      CMN_FAILURE 異常終了
 */
/******************************************************************************/
CmnRet_t MemMngVirtFree( MkPid_t pid,
                         void    *pAddr )
{
    CmnRet_t ret;   /* 戻り値 */
    
    /* メモリ領域解放 */
    ret = AreaFree( &( gVirtTbl[ pid ].allocList ),
                    &( gVirtTbl[ pid ].freeList  ),
                    pAddr                           );
    
    return ret;
}


/******************************************************************************/
/**
 * @brief       仮想メモリ領域管理開始
 * @details     プロセス毎の仮想メモリ領域管理を開始する。
 * 
 * @param[in]   pid プロセスID
 * 
 * @return      初期化結果を判定する。
 * @retval      CMN_SUCCESS 成功
 * @retval      CMN_FAILURE 失敗
 */
/******************************************************************************/
CmnRet_t MemMngVirtStart( MkPid_t pid )
{
    AreaInfo_t *pInfo;  /* 未割当仮想メモリ領域情報 */
    
    /* 全未割当仮想メモリ領域設定 */
    pInfo = AreaSet( &( gVirtTbl[ pid ].freeList ), NULL, 0xFFFFFFFC );
    
    /* 設定結果判定 */
    if ( pInfo == NULL ) {
        /* 失敗 */
        
        return CMN_FAILURE;
    }
    
    return CMN_SUCCESS;
}


/******************************************************************************/
/* 内部モジュール向けグローバル関数定義                                       */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       仮想メモリ領域管理初期化
 * @details     仮想メモリ領域管理テーブルを初期化する。
 */
/******************************************************************************/
void VirtInit( void )
{
    MkPid_t pid;    /* プロセスID */
    
    /* 仮想メモリ管理テーブルエントリ毎に繰り返し */
    for ( pid = MK_CONFIG_PID_MIN; pid <= MK_CONFIG_PID_MAX; pid++ ) {
        /* 未割当仮想メモリ領域情報リスト初期化 */
        MLibListInit( &( gVirtTbl[ pid ].freeList  ) );
        
        /* 割当中仮想メモリ領域情報リスト初期化 */
        MLibListInit( &( gVirtTbl[ pid ].allocList ) );
    }
    
    return;
}


/******************************************************************************/
