/******************************************************************************/
/*                                                                            */
/* src/kernel/Taskmng/TaskmngThread.c                                         */
/*                                                                 2024/06/16 */
/* Copyright (C) 2019-2024 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>

/* ライブラリ */
#include <MLib/MLibDynamicArray.h>
#include <MLib/MLibUtil.h>

/* カーネルヘッダ */
#include <kernel/thread.h>
#include <kernel/types.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32Instruction.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Intmng.h>
#include <Memmng.h>
#include <Taskmng.h>

/* 内部モジュールヘッダ */
#include "TaskmngProc.h"
#include "TaskmngSched.h"
#include "TaskmngTask.h"
#include "TaskmngThread.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* モジュールID */
#define _MODULE_ID_ CMN_MODULE_TASKMNG_THREAD

/** スレッド管理情報動的配列チャンクサイズ */
#define THREAD_TBL_CHUNK_SIZE ( 4 )


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* スレッド管理情報初期化 */
static ThreadInfo_t *AllocThreadInfo( ProcInfo_t *pProcInfo );
/* スレッド生成 */
static void DoCreate( MkThreadParam_t *pParam );
/* 割込みハンドラ */
static void HdlInt( uint32_t        intNo,
                    IntmngContext_t context );


/******************************************************************************/
/* モジュール内グローバル関数定義                                             */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メインスレッド追加
 * @details     プロセス管理情報pProcInfoにメインスレッド用のスレッド管理情報を
 *              追加し、メインスレッド用のユーザスタック領域を割り当てる。
 *
 * @param[in]   *pProcInfo プロセス管理情報
 *
 * @return      追加したスレッドIDを返す。
 * @retval      0           成功（メインスレッドID）
 * @retval      MK_TID_NULL 失敗
 */
/******************************************************************************/
MkTid_t ThreadAddMain( ProcInfo_t *pProcInfo )
{
    MLibErr_t    errMLib;       /* MLIBライブラリエラー値 */
    MLibRet_t    retMLib;       /* MLIBライブラリ戻り値   */
    MkTaskId_t   taskId;        /* タスクID               */
    ThreadInfo_t *pThreadInfo;  /* スレッド管理情報       */

    /* 初期化 */
    errMLib     = MLIB_ERR_NONE;
    retMLib     = MLIB_RET_FAILURE;
    taskId      = MK_TASKID_NULL;
    pThreadInfo = NULL;

    DEBUG_LOG_TRC( "%s(): start. pProcInfo=%p", __func__, pProcInfo );

    /* スレッド管理情報動的配列初期化 */
    retMLib = MLibDynamicArrayInit( &( pProcInfo->threadTbl ),
                                    THREAD_TBL_CHUNK_SIZE,
                                    sizeof ( ThreadInfo_t ),
                                    UINT_MAX,
                                    &errMLib                   );

    /* 初期化結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        return MK_TID_NULL;
    }

    /* スレッド管理情報割当 */
    pThreadInfo = AllocThreadInfo( pProcInfo );

    /* 割当結果判定 */
    if ( pThreadInfo == NULL ) {
        /* 失敗 */

        /* スレッド管理情報動的配列削除 */
        MLibDynamicArrayExit( &( pProcInfo->threadTbl ), NULL );

        return MK_TID_NULL;
    }

    /* 起動時情報設定 */
    pThreadInfo->startInfo.pEntryPoint   = pProcInfo->pEntryPoint;
    pThreadInfo->startInfo.pStackPointer = pProcInfo->userStack.pBottomAddr;

    /* タスク追加 */
    taskId = TaskAdd( pThreadInfo );

    /* 追加結果判定 */
    if ( taskId == MK_TASKID_NULL ) {
        /* 失敗 */

        /* スレッド管理情報解放 */
        MLibDynamicArrayFree( &( pProcInfo->threadTbl ),
                              ( uint_t ) pThreadInfo->tid,
                              &errMLib                     );

        /* スレッド管理情報動的配列削除 */
        MLibDynamicArrayExit( &( pProcInfo->threadTbl ), NULL );

        return MK_TID_NULL;
    }

    DEBUG_LOG_TRC( "%s(): end. ret=%u", __func__, pThreadInfo->tid );

    return pThreadInfo->tid;
}


/******************************************************************************/
/**
 * @brief       アイドルプロセス用メインスレッド追加
 * @details     プロセス管理情報pProcInfoにメインスレッド用のスレッド管理情報を
 *              追加し、メインスレッド用のユーザスタック領域を割り当てる。
 *
 * @param[in]   *pProcInfo プロセス管理情報
 *
 * @return      追加したスレッドIDを返す。
 * @retval      0           成功（メインスレッドID）
 * @retval      MK_TID_NULL 失敗
 */
/******************************************************************************/
MkTid_t ThreadAddMainIdle( ProcInfo_t *pProcInfo )
{
    MLibErr_t    errMLib;       /* MLIBライブラリエラー値 */
    MLibRet_t    retMLib;       /* MLIBライブラリ戻り値   */
    ThreadInfo_t *pThreadInfo;  /* スレッド管理情報       */

    /* 初期化 */
    errMLib     = MLIB_ERR_NONE;
    retMLib     = MLIB_RET_FAILURE;
    pThreadInfo = NULL;

    /* スレッド管理情報動的配列初期化 */
    retMLib = MLibDynamicArrayInit( &( pProcInfo->threadTbl ),
                                    THREAD_TBL_CHUNK_SIZE,
                                    sizeof ( ThreadInfo_t ),
                                    UINT_MAX,
                                    &errMLib                   );

    /* 初期化結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        return MK_TID_NULL;
    }

    /* スレッド管理情報割当 */
    pThreadInfo = AllocThreadInfo( pProcInfo );

    /* 割当結果判定 */
    if ( pThreadInfo == NULL ) {
        /* 失敗 */

        /* スレッド管理情報動的配列削除 */
        MLibDynamicArrayExit( &( pProcInfo->threadTbl ), NULL );

        return MK_TID_NULL;
    }

    return pThreadInfo->tid;
}


/******************************************************************************/
/**
 * @brief       スレッド複製
 * @details     スレッドを複製する。
 *
 * @param[in]   *pProcInfo 複製先プロセス管理情報
 *
 * @return      複製結果を返す。
 * @retval      0           成功（メインスレッドID）
 * @retval      MK_TID_NULL 失敗
 */
/******************************************************************************/
MkTid_t ThreadFork( ProcInfo_t *pProcInfo )
{
    MLibErr_t    errMLib;       /* MLIBライブラリエラー値 */
    MLibRet_t    retMLib;       /* MLIBライブラリ戻り値   */
    MkTaskId_t   taskId;        /* タスクID               */
    ThreadInfo_t *pThreadInfo;  /* スレッド管理情報       */

    /* 初期化 */
    errMLib     = MLIB_ERR_NONE;
    retMLib     = MLIB_RET_FAILURE;
    pThreadInfo = NULL;

    /* スレッド管理情報動的配列初期化 */
    retMLib = MLibDynamicArrayInit( &( pProcInfo->threadTbl ),
                                    THREAD_TBL_CHUNK_SIZE,
                                    sizeof ( ThreadInfo_t ),
                                    UINT_MAX,
                                    &errMLib                   );

    /* 初期化結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        return MK_TID_NULL;
    }

    /* スレッド管理情報割当 */
    pThreadInfo = AllocThreadInfo( pProcInfo );

    /* 割当結果判定 */
    if ( pThreadInfo == NULL ) {
        /* 失敗 */

        /* スレッド管理情報動的配列削除 */
        MLibDynamicArrayExit( &( pProcInfo->threadTbl ), NULL );

        return MK_TID_NULL;
    }

    /* タスク複製 */
    taskId = TaskFork( pThreadInfo );

    /* 複製結果判定 */
    if ( taskId == MK_TASKID_NULL ) {
        /* 失敗 */

        /* スレッド管理情報動的配列削除 */
        MLibDynamicArrayExit( &( pProcInfo->threadTbl ), NULL );

        return MK_TID_NULL;
    }

    return pThreadInfo->tid;
}


/******************************************************************************/
/**
 * @brief       スレッド管理情報取得
 * @details     スレッドIDに該当するスレッド管理情報を取得する。
 *
 * @param[in]   *pProcInfo プロセス管理情報
 * @param[in]   tid        スレッドID
 *
 * @return      スレッド管理情報を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功(スレッド管理情報)
 */
/******************************************************************************/
ThreadInfo_t *ThreadGetInfo( ProcInfo_t *pProcInfo,
                             MkTid_t    tid         )
{
    MLibErr_t    errMLib;       /* MLIBライブラリエラー値 */
    MLibRet_t    retMLib;       /* MLIBライブラリ戻り値   */
    ThreadInfo_t *pThreadInfo;  /* スレッド管理情報       */

    /* 初期化 */
    errMLib     = MLIB_ERR_NONE;
    retMLib     = MLIB_RET_SUCCESS;
    pThreadInfo = NULL;

    /* スレッド管理情報取得 */
    retMLib = MLibDynamicArrayGet( &( pProcInfo->threadTbl ),
                                   ( uint_t ) tid,
                                   ( void ** ) &pThreadInfo,
                                   &errMLib                   );

    /* 取得結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        return NULL;
    }

    return pThreadInfo;
}


/******************************************************************************/
/**
 * @brief       スレッド制御初期化
 * @details     スレッド制御提供カーネルコール用の割込みハンドラを設定する。
 */
/******************************************************************************/
void ThreadInit( void )
{
    DEBUG_LOG_TRC( "%s() start.", __func__ );

    /* 割込みハンドラ設定 */
    IntmngHdlSet( MK_CONFIG_INTNO_THREAD,       /* 割込み番号     */
                  HdlInt,                       /* 割込みハンドラ */
                  IA32_DESCRIPTOR_DPL_3   );    /* 特権レベル     */

    DEBUG_LOG_TRC( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       スレッド管理情報割当
 * @details     スレッド管理情報を割当てて初期化する。
 *
 * @param[in]   *pProcInfo プロセス管理情報
 *
 * @return      スレッド管理情報を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功(スレッド管理情報)
 */
/******************************************************************************/
static ThreadInfo_t *AllocThreadInfo( ProcInfo_t *pProcInfo )
{
    MkTid_t      tid;           /* スレッドID             */
    MLibErr_t    errMLib;       /* MLibライブラリエラー値 */
    MLibRet_t    retMLib;       /* MLibライブラリ戻り値   */
    ThreadInfo_t *pThreadInfo;  /* スレッド管理情報       */

    /* 初期化 */
    errMLib = MLIB_ERR_NONE;
    retMLib = MLIB_RET_FAILURE;

    /* スレッド管理情報割当 */
    retMLib = MLibDynamicArrayAlloc( &( pProcInfo->threadTbl ),
                                     ( uint_t *  ) &tid,
                                     ( void   ** ) &pThreadInfo,
                                     &errMLib                    );

    /* 割当結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        return NULL;
    }

    /* スレッド管理情報初期化 */
    MLibUtilSetMemory8( pThreadInfo, 0, sizeof ( ThreadInfo_t ) );
    pThreadInfo->tid       = tid;
    pThreadInfo->pProcInfo = pProcInfo;

    return pThreadInfo;
}


/******************************************************************************/
/**
 * @brief       スレッド生成
 * @details     スレッドを生成する。
 *
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void DoCreate( MkThreadParam_t *pParam )
{
    MkTaskId_t   taskId;        /* タスクID         */
    ProcInfo_t   *pProcInfo;    /* プロセス管理情報 */
    ThreadInfo_t *pThreadInfo;  /* スレッド管理情報 */

    /* 初期化 */
    taskId      = MK_TASKID_NULL;
    pProcInfo   = NULL;
    pThreadInfo = NULL;

    /* パラメータチェック */
    if ( ( pParam->pStackAddr == NULL ) ||
         ( pParam->stackSize  == 0    )    ) {
        /* 不正 */

        /* 戻り値設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_PARAM;

        return;
    }

    /* プロセス管理情報取得 */
    pProcInfo = SchedGetProcInfo();

    /* スレッド管理情報割当 */
    pThreadInfo = AllocThreadInfo( pProcInfo );

    /* 割当結果判定 */
    if ( pThreadInfo == NULL ) {
        /* 失敗 */

        /* 戻り値設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_NO_RESOURCE;

        return;
    }

    /* 起動時情報設定 */
    pThreadInfo->startInfo.pEntryPoint   = pParam->pFunc;
    pThreadInfo->startInfo.pStackPointer =
        ( void * ) ( ( uint32_t ) pParam->pStackAddr +
                                  pParam->stackSize  -
                                  sizeof ( uint32_t )  );

    /* タスク追加 */
    taskId = TaskAdd( pThreadInfo );

    /* 追加結果判定 */
    if ( taskId == MK_TASKID_NULL ) {
        /* 失敗 */

        /* スレッド管理情報解放 */
        MLibDynamicArrayFree( &( pProcInfo->threadTbl ),
                              ( uint_t ) pThreadInfo->tid,
                              NULL                         );

        /* 戻り値設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_NO_RESOURCE;

        return;
    }

    /* 戻り値設定 */
    pParam->ret    = MK_RET_SUCCESS;
    pParam->err    = MK_ERR_NONE;
    pParam->taskId = taskId;

    return;
}


/******************************************************************************/
/**
 * @brief       割込みハンドラ
 * @details     機能IDから該当する機能を呼び出す。
 *
 * @param[in]   intNo   割込み番号
 * @param[in]   context 割込み発生時コンテキスト
 */
/******************************************************************************/
static void HdlInt( uint32_t        intNo,
                    IntmngContext_t context )
{
    MkThreadParam_t *pParam;    /* パラメータ */

    /* 初期化 */
    pParam = ( MkThreadParam_t * ) context.genReg.esi;

    DEBUG_LOG_TRC( "%s(): start. pParam=%p", __func__, pParam );

    /* パラメータチェック */
    if ( pParam == NULL ) {
        /* 不正 */

        DEBUG_LOG_TRC( "%s(): end.", __func__ );

        return;
    }

    /* 機能ID判定 */
    switch ( pParam->funcId ) {
        case MK_THREAD_FUNCID_CREATE:
            /* スレッド生成 */

            DEBUG_LOG_TRC( "%s(): create.", __func__ );
            DoCreate( pParam );
            break;

        default:
            /* 不正 */

            /* エラー設定 */
            pParam->ret = MK_RET_FAILURE;
            pParam->err = MK_ERR_PARAM;
    }

    DEBUG_LOG_TRC( "%s(): end. ret=%d, err=%u",
                   __func__,
                   pParam->ret,
                   pParam->err                  );

    return;
}


/******************************************************************************/
