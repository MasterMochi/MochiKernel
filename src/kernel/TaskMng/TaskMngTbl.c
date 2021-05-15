/******************************************************************************/
/*                                                                            */
/* src/kernel/TaskMng/TaskMngTbl.c                                            */
/*                                                                 2021/05/05 */
/* Copyright (C) 2019-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

/* ライブラリヘッダ */
#include <MLib/MLibList.h>
#include <MLib/MLibUtil.h>

/* カーネルヘッダ */
#include <kernel/types.h>

/* 外部モジュールヘッダ */
#include <Memmng.h>

/* 内部モジュールヘッダ */
#include "TaskMngTbl.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                       \
    DebugLogOutput( CMN_MODULE_TASKMNG_TBL,    \
                    __LINE__,                  \
                    __VA_ARGS__             )
#else
#define DEBUG_LOG( ... )
#endif

/** チャンク内プロセス管理情報数 */
#define CHUNK_PROCINFO_NUM \
    ( ( 4096 - sizeof ( MLibListNode_t ) ) / sizeof ( TblProcInfo_t ) )
/** チャンク内プロセス管理情報インデックス最小値 */
#define CHUNK_PROCINFO_MIN ( 0 )
/** チャンク内プロセス管理情報インデックス最大値 */
#define CHUNK_PROCINFO_MAX ( CHUNK_PROCINFO_NUM - 1 )

/** プロセス管理テーブルチャンク */
typedef struct {
    MLibListNode_t nodeInfo;                        /**< ノード情報       */
    TblProcInfo_t  procInfo[ CHUNK_PROCINFO_NUM ];  /**< プロセス管理情報 */
} procChunk_t;


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* プロセス管理テーブルチャンク追加 */
procChunk_t *AddProcChunk( void );
/* スレッド管理テーブルチャンク追加 */
TblThreadChunk_t *AddThreadChunk( TblProcInfo_t *pProcInfo );


/******************************************************************************/
/* グローバル変数定義                                                         */
/******************************************************************************/
/** プロセス管理テーブルチャンクリスト */
static MLibList_t gProcChunkList;


/******************************************************************************/
/* モジュール内グローバル関数定義                                             */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       プロセス管理情報割当て
 * @detais      プロセス管理テーブルチャンクからの最初に見つかった未使用プロセ
 *              ス管理情報を返す。全てのチャンクに未使用のプロセス管理情報が無
 *              かった場合、新しいチャンクを追加して割り当てる。
 *
 * @return      割り当てたプロセス管理情報へのポインタを返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功（プロセス管理情報）
 */
/******************************************************************************/
TblProcInfo_t *TblAllocProcInfo( void )
{
    int         idx;        /* インデックス */
    procChunk_t *pChunk;    /* チャンク     */

    /* 初期化 */
    idx    = 0;
    pChunk = NULL;

    /* プロセス管理テーブルチャンク毎に繰り返す */
    while ( true ) {
        /* プロセス管理テーブルチャンク取得 */
        pChunk = ( procChunk_t * )
                 MLibListGetNextNode( &( gProcChunkList ),
                                      ( MLibListNode_t * ) pChunk );

        /* 取得結果判定 */
        if ( pChunk == NULL ) {
            /* チャンク無し */

            /* チャンク追加 */
            pChunk = AddProcChunk();

            /* 追加結果判定 */
            if ( pChunk == NULL ) {
                /* 失敗 */

                return NULL;
            }
        }

        /* チャンク内プロセス管理情報毎に繰り返す */
        for ( idx = 0; idx < CHUNK_PROCINFO_NUM; idx++ ) {
            /* 使用フラグ判定 */
            if ( pChunk->procInfo[ idx ].used == false ) {
                /* 未使用 */

                /* 使用フラグ設定 */
                pChunk->procInfo[ idx ].used = true;

                return &( pChunk->procInfo[ idx ] );
            }
        }
    }
}


/******************************************************************************/
/**
 * @brief       スレッド管理情報割当て
 * @detais      プロセス管理情報*pProcInfo内のスレッド管理テーブルチャンクから
 *              最初に見つかった未使用スレッド管理情報を返す。全てのチャンクに
 *              未使用のスレッド管理情報が無かった場合、新しいチャンクを追加し
 *              て割り当てる。
 *
 * @param[in]   *pProcInfo プロセス管理情報
 *
 * @return      割り当てたスレッド管理情報へのポインタを返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功（スレッド管理情報）
 */
/******************************************************************************/
TblThreadInfo_t *TblAllocThreadInfo( TblProcInfo_t *pProcInfo )
{
    int              idx;       /* インデックス */
    TblThreadChunk_t *pChunk;   /* チャンク     */

    /* 初期化 */
    idx    = 0;
    pChunk = NULL;

    /* スレッド管理テーブルチャンク毎に繰り返す */
    while ( true ) {
        /* スレッド管理テーブルチャンク取得 */
        pChunk = ( TblThreadChunk_t * )
                 MLibListGetNextNode( &( pProcInfo->threadChunkList ),
                                      ( MLibListNode_t * ) pChunk      );

        /* 取得結果判定 */
        if ( pChunk == NULL ) {
            /* チャンク無し */

            /* チャンク追加 */
            pChunk = AddThreadChunk( pProcInfo );

            /* 追加結果判定 */
            if ( pChunk == NULL ) {
                /* 失敗 */

                return NULL;
            }
        }

        /* チャンク内スレッド管理情報毎に繰り返す */
        for ( idx = 0; idx < TBL_CHUNK_THREADINFO_NUM; idx++ ) {
            /* 使用フラグ判定 */
            if ( pChunk->threadInfo[ idx ].used == false ) {
                /* 未使用 */

                /* 使用フラグ設定 */
                pChunk->threadInfo[ idx ].used = true;

                return &( pChunk->threadInfo[ idx ] );
            }
        }
    }
}


/******************************************************************************/
/**
 * @brief       プロセス管理情報解放
 * @details     プロセス管理情報を解放する。
 *
 * @params[in]  *pProcInfo プロセス管理情報
 */
/******************************************************************************/
void TblFreeProcInfo( TblProcInfo_t *pProcInfo )
{
    /* スレッド管理テーブル初期化 */
    /* TODO */

    /* プロセス管理情報初期化 */
    pProcInfo->used        = false;
    pProcInfo->type        = 0;
    pProcInfo->dirId       = MEMMNG_PAGE_DIR_ID_NULL;
    pProcInfo->pEntryPoint = NULL;
    pProcInfo->pEndPoint   = NULL;
    pProcInfo->pBreakPoint = NULL;

    return;
}


/******************************************************************************/
/**
 * @brief       スレッド管理情報解放
 * @details     スレッド管理情報を解放する。
 *
 * @params[in]  *pThreadInfo スレッド管理情報
 */
/******************************************************************************/
void TblFreeThreadInfo( TblThreadInfo_t *pThreadInfo )
{
    /* スタック情報解放 */
    /* TODO */

    /* スレッド管理情報初期化 */
    pThreadInfo->used  = false;
    pThreadInfo->state = 0;
    MLibUtilSetMemory8(
        &( pThreadInfo->context     ), 0, sizeof ( TblContext_t   ) );
    MLibUtilSetMemory8(
        &( pThreadInfo->userStack   ), 0, sizeof ( TblStackInfo_t ) );
    MLibUtilSetMemory8(
        &( pThreadInfo->kernelStack ), 0, sizeof ( TblStackInfo_t ) );

    return;
}


/******************************************************************************/
/**
 * @brief       プロセス管理情報取得
 * @details     プロセス管理テーブルからプロセスID pid に該当するプロセス管理情
 *              報を返す。
 *
 * @param[in]   pid プロセスID
 *
 * @return      プロセス管理情報へのポインタを返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功（プロセス管理情報）
 */
/******************************************************************************/
TblProcInfo_t *TblGetProcInfo( MkPid_t pid )
{
    MkPid_t       firstPid;     /* チャンク先頭プロセスID */
    MkPid_t       lastPid;      /* チャンク後尾プロセスID */
    procChunk_t   *pChunk;      /* チャンク               */
    TblProcInfo_t *pProcInfo;   /* プロセス管理情報       */

    /* 初期化 */
    firstPid  = 0;
    lastPid   = 0;
    pChunk    = NULL;
    pProcInfo = NULL;

    /* プロセス管理テーブルチャンク毎に繰り返す */
    while ( true ) {
        /* プロセス管理テーブルチャンク取得 */
        pChunk = ( procChunk_t * )
                 MLibListGetNextNode( &gProcChunkList,
                                      ( MLibListNode_t * ) pChunk );

        /* 取得結果判定 */
        if ( pChunk == NULL ) {
            /* チャンク無し */

            return NULL;
        }

        /* チャンク先頭・後尾のプロセスID取得 */
        firstPid = pChunk->procInfo[                  0 ].pid;
        lastPid  = pChunk->procInfo[ CHUNK_PROCINFO_MAX ].pid;

        /* チャンク内PID比較 */
        if ( ( pid < firstPid ) && ( lastPid < pid ) ) {
            /* 範囲外 */

            /* 次のチャンクへ */
            continue;
        }

        /* プロセス管理情報取得 */
        pProcInfo = &( pChunk->procInfo[ pid - firstPid ] );

        /* 使用フラグ判定 */
        if ( pProcInfo->used == false ) {
            /* 未使用 */

            return NULL;
        }

        return pProcInfo;
    }
}


/******************************************************************************/
/**
 * @brief       タスク管理情報取得
 * @details     プロセス管理テーブルおよびスレッド管理テーブルからタスクID
 *              taskId に該当するタスク管理情報を取得
 *
 * @param[in]   taskId タスクID
 *
 * @return      タスク管理情報を返す。
 * @retval      NULL    該当するタスク管理情報無し
 * @retval      NULL以外 成功（タスク管理情報）
 */
/******************************************************************************/
TblTaskInfo_t *TblGetTaskInfo( MkTaskId_t taskId )
{
    TblProcInfo_t   *pProcInfo;     /* プロセス管理情報 */
    TblThreadInfo_t *pThreadInfo;   /* スレッド管理情報 */

    /* 初期化 */
    pProcInfo   = NULL;
    pThreadInfo = NULL;

    /* プロセス管理情報取得 */
    pProcInfo = TblGetProcInfo( MK_TASKID_TO_PID( taskId ) );

    /* 取得結果判定 */
    if ( pProcInfo == NULL ) {
        /* 失敗 */

        return NULL;
    }

    /* スレッド管理情報取得 */
    pThreadInfo = TblGetThreadInfo( pProcInfo,
                                    MK_TASKID_TO_TID( taskId ) );

    return ( TblTaskInfo_t * ) pThreadInfo;
}


/******************************************************************************/
/**
 * @brief       スレッド管理情報取得
 * @details     プロセス管理情報pProcInfoのスレッド管理テーブルからスレッドID
 *              tid に該当するスレッド管理情報を返す。
 *
 * @param[in]   *pProcInfo プロセス管理情報
 * @param[in]   tid        スレッドID
 *
 * @return      スレッド管理情報へのポインタを返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功（スレッド管理情報）
 */
/******************************************************************************/
TblThreadInfo_t *TblGetThreadInfo( TblProcInfo_t *pProcInfo,
                                   MkTid_t       tid         )
{
    MkTid_t          firstTid;      /* チャンク先頭スレッドID */
    MkTid_t          lastTid;       /* チャンク後尾スレッドID */
    TblThreadInfo_t  *pThreadInfo;  /* スレッド管理情報       */
    TblThreadChunk_t *pChunk;       /* チャンク               */

    /* 初期化 */
    firstTid    = 0;
    lastTid     = 0;
    pThreadInfo = NULL;
    pChunk      = NULL;

    /* スレッド管理テーブルチャンク毎に繰り返す */
    while ( true ) {
        /* スレッド管理テーブルチャンク取得 */
        pChunk = ( TblThreadChunk_t * )
                 MLibListGetNextNode( &( pProcInfo->threadChunkList ),
                                      ( MLibListNode_t * ) pChunk      );

        /* 取得結果判定 */
        if ( pChunk == NULL ) {
            /* チャンク無し */

            return NULL;
        }

        /* チャンク先頭・後尾のスレッドID取得 */
        firstTid = pChunk->threadInfo[                        0 ].tid;
        lastTid  = pChunk->threadInfo[ TBL_CHUNK_THREADINFO_MAX ].tid;

        /* チャンク内スレッドID比較 */
        if ( ( tid < firstTid ) && ( lastTid < tid ) ) {
            /* 範囲外 */

            /* 次のチャンクへ */
            continue;
        }

        /* スレッド管理情報設定 */
        pThreadInfo = &( pChunk->threadInfo[ tid - firstTid ] );

        /* 使用フラグ判定 */
        if ( pThreadInfo->used == false ) {
            /* 未使用 */

            return NULL;
        }

        return pThreadInfo;
    }
}


/******************************************************************************/
/**
 * @brief       テーブル管理モジュール初期化
 * @details     プロセス管理テーブルのチャンクリストを初期化する。
 */
/******************************************************************************/
void TblInit( void )
{
    /* プロセス管理テーブルチャンクリスト初期化 */
    MLibListInit( &gProcChunkList );

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       プロセス管理テーブルチャンク追加
 * @details     プロセス管理テーブルのチャンクリストに新しいチャンクを追加する。
 *
 * @return      新しく割り当てたチャンクへのポインタを返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功(チャンク)
 */
/******************************************************************************/
procChunk_t *AddProcChunk( void )
{
    int         idx;            /* インデックス   */
    MkPid_t     pid;            /* プロセスID     */
    procChunk_t *pLastChunk;    /* 最後尾チャンク */
    procChunk_t *pNewChunk;     /* 追加チャンク   */

    /* 初期化 */
    idx        = 0;
    pid        = 0;
    pLastChunk = NULL;
    pNewChunk  = NULL;

    /* 最後尾チャンク取得 */
    pLastChunk = ( procChunk_t * ) MLibListGetPrevNode( &gProcChunkList, NULL );

    /* 取得結果判定 */
    if ( pLastChunk == NULL ) {
        /* 最後尾チャンク無し */

        /* PID初期化 */
        pid = 0;

    } else {
        /* 最後尾チャンク有り */

        /* PID計算 */
        pid = pLastChunk->procInfo[ 0 ].pid + CHUNK_PROCINFO_NUM;
    }

    /* チャンク領域確保 */
    pNewChunk = MemmngHeapAlloc( sizeof ( procChunk_t ) );

    /* 確保結果判定 */
    if ( pNewChunk == NULL ) {
        /* 失敗 */

        return NULL;
    }

    /* チャンク初期化 */
    MLibUtilSetMemory8( pNewChunk, 0, sizeof ( procChunk_t ) );

    /* チャンク内プロセス管理情報毎に繰り返す */
    for ( idx = 0; idx < CHUNK_PROCINFO_NUM; idx++ ) {
        /* プロセス管理情報初期化 */
        pNewChunk->procInfo[ idx ].pid   = pid;
        pNewChunk->procInfo[ idx ].used  = false;
        pNewChunk->procInfo[ idx ].dirId = MEMMNG_PAGE_DIR_ID_NULL;

        /* スレッド管理テーブルチャンクリスト初期化 */
        MLibListInit( &( pNewChunk->procInfo[ idx ].threadChunkList ) );

        /* PID更新 */
        pid++;
    }

    /* チャンクリストに追加 */
    MLibListInsertTail( &gProcChunkList,
                        &( pNewChunk->nodeInfo ) );

    return pNewChunk;
}


/******************************************************************************/
/**
 * @brief       スレッド管理テーブルチャンク追加
 * @details     プロセス管理情報pProcInfoのスレッド管理テーブルのチャンクリスト
 *              に新しいチャンクを追加する。
 *
 * @param[in]   *pProcInfo プロセス管理情報
 *
 * @return      新しく割り当てたチャンクへのポインタを返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功(チャンク)
 */
/******************************************************************************/
TblThreadChunk_t *AddThreadChunk( TblProcInfo_t *pProcInfo )
{
    uint32_t         idx;           /* インデックス   */
    MkTid_t          tid;           /* スレッドID     */
    TblThreadChunk_t *pLastChunk;   /* 最後尾チャンク */
    TblThreadChunk_t *pNewChunk;    /* 追加チャンク   */

    /* 初期化 */
    idx        = 0;
    tid        = 0;
    pLastChunk = NULL;
    pNewChunk  = NULL;

    /* 最後尾チャンク取得 */
    pLastChunk = ( TblThreadChunk_t * )
                 MLibListGetPrevNode( &( pProcInfo->threadChunkList ), NULL );

    /* 取得結果判定 */
    if ( pLastChunk == NULL ) {
        /* 最後尾チャンク無し */

        /* スレッドID初期化 */
        tid = 0;

    } else {
        /* 最後尾チャンク有り */

        /* スレッドID計算 */
        tid = pLastChunk->threadInfo[ 0 ].tid + TBL_CHUNK_THREADINFO_NUM;
    }

    /* チャンク領域確保 */
    pNewChunk = MemmngHeapAlloc( sizeof ( TblThreadChunk_t ) );

    /* 確保結果判定 */
    if ( pNewChunk == NULL ) {
        /* 失敗 */

        return NULL;
    }

    /* チャンク初期化 */
    MLibUtilSetMemory8( pNewChunk, 0, sizeof ( TblThreadChunk_t ) );

    /* チャンク内スレッド管理情報毎に繰り返す */
    for ( idx = 0; idx < TBL_CHUNK_THREADINFO_NUM; idx++ ) {
        /* スレッド管理情報初期化 */
        pNewChunk->threadInfo[ idx ].pProcInfo = pProcInfo;
        pNewChunk->threadInfo[ idx ].tid       = tid;
        pNewChunk->threadInfo[ idx ].taskId    =
            MK_TASKID_MAKE( pProcInfo->pid, tid );

        /* スレッドID更新 */
        tid++;
    }

    /* チャンクリストに追加 */
    MLibListInsertTail( &( pProcInfo->threadChunkList ),
                        ( MLibListNode_t * ) pNewChunk   );

    return pNewChunk;
}


/******************************************************************************/
