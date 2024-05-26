/******************************************************************************/
/*                                                                            */
/* src/kernel/Memmng/MemmngVirt.c                                             */
/*                                                                 2024/05/13 */
/* Copyright (C) 2018-2024 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>

/* ライブラリヘッダ */
#include <MLib/MLib.h>
#include <MLib/MLibDynamicArray.h>
#include <MLib/MLibList.h>
#include <MLib/MLibUtil.h>

/* 共通ヘッダ */
#include <memmap.h>
#include <hardware/IA32/IA32Paging.h>
#include <kernel/kernel.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Memmng.h>

/* 内部モジュールヘッダ */
#include "MemmngArea.h"
#include "MemmngVirt.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                 \
    DebugOutput( CMN_MODULE_MEMMNG_VIRT, \
                 __LINE__,               \
                 __VA_ARGS__             )
#else
#define DEBUG_LOG( ... )
#endif

/** プロセス毎管理用動的配列チャンクサイズ */
#define PROCINFO_ARRAY_CHUNK_SIZE ( 128 )

/** 未使用ブロック管理情報用動的配列エントリ内ブロック管理情報数 */
#define AREAINFO_NUM            ( 256 )
/** 未使用ブロック管理情報用動的配列エントリサイズ */
#define UNUSED_ARRAY_ENTRY_SIZE ( sizeof ( AreaInfo_t ) * AREAINFO_NUM )
/** 未使用ブロック管理情報用動的配列エントリ数 */
#define UNUSED_ARRAY_ENTRY_MAX  ( 4096 )

/** 仮想メモリ領域管理テーブル構造体 */
typedef struct {
    MLibList_t         unusedList;  /**< 未使用リンクリスト               */
    MLibDynamicArray_t procInfo;    /**< プロセス毎管理用動的配列         */
    MLibDynamicArray_t unused;      /**< 未使用ブロック管理情報用動的配列 */
} virtTbl_t;

/** プロセス管理情報構造体 */
typedef struct {
    MLibList_t allocList;   /**< 割当済リンクリスト */
    MLibList_t freeList;    /**< 未割当リンクリスト */
} procInfo_t;


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* 未使用ブロック管理情報割当 */
static void AllocUnusedAreaInfo( void );


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** 仮想メモリ領域管理テーブル */
static virtTbl_t gVirtTbl;


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
 * @return      割当先の仮想メモリアドレスを返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功
 *
 * @note        割当サイズが4Kバイトアライメントでない場合は、割当サイズが4Kバ
 *              イトアライメントに合うよう加算して、仮想メモリ領域を割り当てる。
 */
/******************************************************************************/
void *MemmngVirtAlloc( MkPid_t pid,
                       size_t  size )
{
    void       *pRet;       /* 戻り値           */
    MLibRet_t  retMLib;     /* MLib戻り値       */
    procInfo_t *pProcInfo;  /* プロセス管理情報 */

    /* 初期化 */
    pRet      = NULL;
    retMLib   = MLIB_RET_FAILURE;
    pProcInfo = NULL;

    /* サイズチェック */
    if ( size == 0 ) {
        /* 不正 */

        return NULL;

    }

    /* プロセス管理情報取得 */
    retMLib = MLibDynamicArrayGet( &( gVirtTbl.procInfo ),
                                   ( uint_t ) pid,
                                   ( void ** ) &pProcInfo,
                                   NULL                    );

    /* 取得結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        return NULL;
    }

    /* アライメント計算 */
    size = MLIB_UTIL_ALIGN( size, IA32_PAGING_PAGE_SIZE );

    /* 未使用リンクリストエントリ数判定 */
    if ( gVirtTbl.unusedList.size < 5 ) {
        /* 5未満 */

        /* 未使用ブロック管理情報割当 */
        AllocUnusedAreaInfo();
    }

    /* メモリ領域割当 */
    pRet = AreaAlloc( &( pProcInfo->allocList ),
                      &( pProcInfo->freeList  ),
                      &( gVirtTbl.unusedList  ),
                      size                       );

    return pRet;
}


/******************************************************************************/
/**
 * @brief       仮想メモリ領域管理終了
 * @details     プロセス毎の仮想メモリ領域管理を終了する。
 *
 * @param[in]   pid プロセスID
 *
 * @return      終了結果を判定する。
 * @retval      CMN_SUCCESS 成功
 * @retval      CMN_FAILURE 失敗
 */
/******************************************************************************/
CmnRet_t MemmngVirtEnd( MkPid_t pid )
{
    MLibRet_t  retMLib;     /* MLib戻り値       */
    procInfo_t *pProcInfo;  /* プロセス管理情報 */
    AreaInfo_t *pAreaInfo;  /* ブロック管理情報 */

    /* 初期化 */
    retMLib   = MLIB_RET_FAILURE;
    pProcInfo = NULL;
    pAreaInfo = NULL;

    /* プロセス管理情報取得 */
    retMLib = MLibDynamicArrayGet( &( gVirtTbl.procInfo ),
                                   ( uint_t ) pid,
                                   ( void ** ) &pProcInfo,
                                   NULL                    );

    /* 取得結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        return CMN_FAILURE;
    }

    /* 割当済リンクリスト全ノード毎に繰り返す */
    while ( true ) {
        /* 割当済リンクリスト末尾削除 */
        pAreaInfo = ( AreaInfo_t * )
                    MLibListRemoveTail( &( pProcInfo->allocList ) );

        /* 削除結果判定 */
        if ( pAreaInfo == NULL ) {
            /* ノード無し */
            break;
        }

        /* 未使用リンクリスト末尾追加 */
        MLibListInsertTail( &( gVirtTbl.unusedList ),
                            &( pAreaInfo->node     )  );
    }

    /* 未割当リンクリスト全ノード毎に繰り返す */
    while ( true ) {
        /* 未割当リンクリスト末尾削除 */
        pAreaInfo = ( AreaInfo_t * )
                    MLibListRemoveTail( &( pProcInfo->freeList ) );

        /* 削除結果判定 */
        if ( pAreaInfo == NULL ) {
            /* ノード無し */
            break;
        }

        /* 未使用リンクリスト末尾追加 */
        MLibListInsertTail( &( gVirtTbl.unusedList ),
                            &( pAreaInfo->node     )  );
    }

    /* プロセス管理情報解放 */
    MLibDynamicArrayFree( &( gVirtTbl.procInfo ), ( uint_t ) pid, NULL );

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
CmnRet_t MemmngVirtFree( MkPid_t pid,
                         void    *pAddr )
{
    CmnRet_t   ret;         /* 戻り値           */
    MLibRet_t  retMLib;     /* MLib戻り値       */
    procInfo_t *pProcInfo;  /* プロセス管理情報 */

    /* 初期化 */
    ret       = CMN_FAILURE;
    retMLib   = MLIB_RET_FAILURE;
    pProcInfo = NULL;

    /* プロセス管理情報取得 */
    retMLib = MLibDynamicArrayGet( &( gVirtTbl.procInfo ),
                                   ( uint_t ) pid,
                                   ( void ** ) &pProcInfo,
                                   NULL                    );

    /* 取得結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        return CMN_FAILURE;
    }

    /* メモリ領域解放 */
    ret = AreaFree( &( pProcInfo->allocList ),
                    &( pProcInfo->freeList  ),
                    &( gVirtTbl.unusedList  ),
                    pAddr                      );

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
CmnRet_t MemmngVirtStart( MkPid_t pid )
{
    MLibRet_t  retMLib;     /* MLib戻り値       */
    procInfo_t *pProcInfo;  /* プロセス管理情報 */

    /* 初期化 */
    retMLib   = MLIB_RET_FAILURE;
    pProcInfo = NULL;

    /* プロセス毎管理用動的配列エントリ割当 */
    retMLib = MLibDynamicArrayAllocSpec(
                  &( gVirtTbl.procInfo ),
                  ( uint_t ) pid,
                  ( void ** ) &pProcInfo,
                  NULL                    );

    /* 割当結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        return CMN_FAILURE;
    }

    /* 割当済リンクリスト初期化 */
    MLibListInit( &( pProcInfo->allocList ) );

    /* 未使用リンクリスト初期化 */
    MLibListInit( &( pProcInfo->freeList ) );

    /* 未使用リンクリストエントリ数判定 */
    if ( gVirtTbl.unusedList.size < 5 ) {
        /* 5未満 */

        /* 未使用ブロック管理情報割当 */
        AllocUnusedAreaInfo();
    }

    /* 全未割当メモリ領域設定 */
    AreaAdd( &( pProcInfo->freeList ),
             &( gVirtTbl.unusedList ),
             0x00000000,
             0xFFFFFFFC,
             true                      );

    /* ブートデータ領域割当 */
    AreaAllocSpec( &( pProcInfo->allocList ),
                   &( pProcInfo->freeList  ),
                   &( gVirtTbl.unusedList  ),
                   ( void * ) MEMMAP_VADDR_BOOTDATA,
                   MEMMAP_VSIZE_BOOTDATA             );

    /* カーネル領域割当 */
    AreaAllocSpec( &( pProcInfo->allocList ),
                   &( pProcInfo->freeList  ),
                   &( gVirtTbl.unusedList  ),
                   ( void * ) MEMMAP_VADDR_KERNEL,
                   MEMMAP_VSIZE_KERNEL             );

    /* ユーザ領域割当 */
    AreaAllocSpec( &( pProcInfo->allocList ),
                   &( pProcInfo->freeList  ),
                   &( gVirtTbl.unusedList  ),
                   ( void * ) MEMMAP_VADDR_USER,
                   MEMMAP_VSIZE_USER             );

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
    MLibRet_t retMLib;  /* MLib戻り値 */

    /* 初期化 */
    retMLib = MLIB_RET_FAILURE;

    /* 未使用リンクリスト初期化 */
    MLibListInit( &( gVirtTbl.unusedList ) );

    /* プロセス毎管理用動的配列初期化 */
    retMLib = MLibDynamicArrayInit(
                  &( gVirtTbl.procInfo ),
                  PROCINFO_ARRAY_CHUNK_SIZE,
                  sizeof ( procInfo_t ),
                  MK_PID_NUM,
                  NULL                       );

    /* 初期化結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        /* [TODO]アボート */
    }

    /* 未使用ブロック管理情報用動的配列初期化 */
    retMLib = MLibDynamicArrayInit(
                  &( gVirtTbl.unused ),
                  1,
                  UNUSED_ARRAY_ENTRY_SIZE,
                  UNUSED_ARRAY_ENTRY_MAX,
                  NULL                     );

    /* 初期化結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        /* [TODO]アボート */
    }

    /* 未使用ブロック管理情報割当 */
    AllocUnusedAreaInfo();

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       未使用ブロック管理情報割当
 * @details     動的配列エントリ割当を行い、ブロック管理情報をヒープ領域から割
 *              り当てる。割り当てたブロック管理情報は未使用リンクリストに追加
 *              する。
 */
/******************************************************************************/
static void AllocUnusedAreaInfo( void )
{
    uint_t     dummy;       /* ダミー変数       */
    uint32_t   idx;         /* インデックス     */
    MLibRet_t  retMLib;     /* MLib戻り値       */
    AreaInfo_t *pAreaInfo;  /* ブロック管理情報 */

    /* 初期化 */
    idx       = 0;
    retMLib   = MLIB_RET_FAILURE;
    pAreaInfo = NULL;

    /* ブロック管理情報割当 */
    retMLib = MLibDynamicArrayAlloc(
                  &( gVirtTbl.unused ),
                  &dummy,
                  ( void ** ) &pAreaInfo,
                  NULL                    );

    /* 割当結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        return;
    }

    /* ブロック管理情報毎に繰り返す */
    for ( idx = 0; idx < AREAINFO_NUM; idx++ ) {
        /* 未使用リンクリスト追加 */
        MLibListInsertTail( &( gVirtTbl.unusedList   ),
                            &( pAreaInfo[ idx ].node )  );
    }

    return;
}


/******************************************************************************/
