/******************************************************************************/
/*                                                                            */
/* src/kernel/Taskmng/TaskmngSched.c                                          */
/*                                                                 2021/05/22 */
/* Copyright (C) 2017-2021 Mochi.                                             */
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
#include <MLib/MLib.h>
#include <MLib/MLibList.h>
#include <MLib/MLibUtil.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32Instruction.h>
#include <kernel/config.h>
#include <kernel/types.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Memmng.h>

/* 内部モジュールヘッダ */
#include "TaskmngSched.h"
#include "TaskmngTbl.h"
#include "TaskmngTss.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                        \
    DebugLogOutput( CMN_MODULE_TASKMNG_SCHED,   \
                    __LINE__,                   \
                    __VA_ARGS__               )
#else
#define DEBUG_LOG( ... )
#endif

/** 実行可能タスクグループ数 */
#define SCHED_RUNGRP_NUM       ( 2 )

/** アイドルタスク情報インデックス */
#define SCHED_IDLE_IDX         ( 0 )

/* スケジューラ実行中レベル */
#define SCHED_LEVEL_KERNEL     ( 0 )    /**< カーネルレベル */
#define SCHED_LEVEL_DRIVER     ( 1 )    /**< ドライバレベル */
#define SCHED_LEVEL_SERVER     ( 2 )    /**< サーバレベル   */
#define SCHED_LEVEL_USER       ( 3 )    /**< ユーザレベル   */

/* 実行状態 */
#define STATE_RUN              ( 0 )    /**< 実行状態 */
#define STATE_WAIT             ( 1 )    /**< 待ち状態 */

/** 実行可能タスクグループ構造体 */
typedef struct {
    MLibList_t kernelQ;             /**< カーネルタスクキュー */
    MLibList_t driverQ;             /**< ドライバタスクキュー */
    MLibList_t serverQ;             /**< サーバタスクキュー   */
    MLibList_t userQ;               /**< ユーザタスクキュー   */
} schedRunGrp_t;

/** 待ちタスクグループ構造体 */
typedef struct {
    MLibList_t waitQ;               /**< 待ちキュー */
} schedWaitGrp_t;

/** スケジューラテーブル構造体 */
typedef struct {
    TblTaskInfo_t  *pIdleTaskInfo;              /**< アイドルタスク管理情報  */
    TblTaskInfo_t  *pRunTaskInfo;               /**< 実行中タスク情報        */
    uint32_t       runFlg;                      /**< タスク実行済みフラグ    */
    uint32_t       runningGrpIdx;               /**< 実行中タスクグループIDX */
    uint32_t       reservedGrpIdx;              /**< 予約タスクグループIDX   */
    schedRunGrp_t  runGrp[ SCHED_RUNGRP_NUM ];  /**< 実行可能タスクグループ  */
    schedWaitGrp_t waitGrp;                     /**< 待ちタスクグループ      */
} schedTbl_t;


/******************************************************************************/
/* ローカル関数プロトタイプ宣言                                               */
/******************************************************************************/
/* 実行予約タスクグループエンキュー */
static void EnqueueToReservedGrp( TblTaskInfo_t *pTaskInfo );
/* 実行可能タスクグループ役割切替 */
static void SwitchRunGrpRole( void );
/* タスクスイッチ */
static void SwitchTask( TblTaskInfo_t *pRunTaskInfo,
                        TblTaskInfo_t *pNextTaskInfo );
/* タスクスイッチ終了ポイント（ラベル） */
void SwitchTaskEnd( void );


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** スケジューラテーブル */
static schedTbl_t gSchedTbl;


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       スケジューラ実行
 * @details     スケジューラを実行して次に実行可能なタスクにタスクスイッチする。
 */
/******************************************************************************/
void TaskmngSchedExec( void )
{
    uint32_t      level;            /* 実行中レベル           */
    TblProcInfo_t *pRunProcInfo;    /* 実行中プロセス管理情報 */
    TblTaskInfo_t *pRunTaskInfo;    /* 実行中タスク管理情報   */
    TblTaskInfo_t *pNextTaskInfo;   /* タスク情報             */
    schedRunGrp_t *pRunGrp;         /* 実行中タスクグループ   */

    /* 初期化 */
    level         = SCHED_LEVEL_KERNEL;
    pRunTaskInfo  = gSchedTbl.pRunTaskInfo;
    pRunProcInfo  = pRunTaskInfo->pProcInfo;
    pNextTaskInfo = NULL;
    pRunGrp       = NULL;

    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() start.", __func__ );*/

    /* 実行中プロセス判定 */
    if ( pRunProcInfo->pid == TASKMNG_PID_IDLE ) {
        /* アイドルプロセス */

        /* 実行可能タスクグループ役割切替 */
        SwitchRunGrpRole();

    } else {
        /* アイドルプロセス以外 */

        /* 実行中タスク状態判定 */
        if ( pRunTaskInfo->state == STATE_RUN ) {
            /* 実行状態 */

            /* 実行予約タスクグループにキューイング */
            EnqueueToReservedGrp( pRunTaskInfo );

            /* タスク実行済みフラグ設定 */
            gSchedTbl.runFlg = true;

        } else {
            /* 待ち状態 */

            /* タスク実行済みフラグ設定 */
            gSchedTbl.runFlg = false;
        }
    }

    /* 実行中タスクグループ設定 */
    pRunGrp = &gSchedTbl.runGrp[ gSchedTbl.runningGrpIdx ];

    do {
        /* 実行中レベル判定 */
        if ( level == SCHED_LEVEL_KERNEL ) {
            /* カーネルレベル */

            /* カーネルキューからデキュー */
            pNextTaskInfo =
                ( TblTaskInfo_t * )
                MLibListRemoveTail( &( pRunGrp->kernelQ ) );

        } else if ( level == SCHED_LEVEL_DRIVER ) {
            /* ドライバレベル */

            /* ドライバキューからデキュー */
            pNextTaskInfo =
                ( TblTaskInfo_t * )
                MLibListRemoveTail( &( pRunGrp->driverQ ) );

        } else if ( level == SCHED_LEVEL_SERVER ) {
            /* サーバレベル */

            /* サーバキューからデキュー */
            pNextTaskInfo =
                ( TblTaskInfo_t * )
                MLibListRemoveTail( &( pRunGrp->serverQ ) );

        } else if ( level == SCHED_LEVEL_USER ) {
            /* ユーザレベル */

            /* ユーザキューからデキュー */
            pNextTaskInfo =
                ( TblTaskInfo_t * )
                MLibListRemoveTail( &( pRunGrp->userQ ) );

        } else {
            /* 他 */

            /* タスク実行済フラグ判定 */
            if ( gSchedTbl.runFlg == false ) {
                /* 実行済みタスク無 */

                /* アイドルタスク設定 */
                pNextTaskInfo = gSchedTbl.pIdleTaskInfo;

                break;

            } else {
                /* 実行済みタスク有 */

                /* 実行可能タスクグループ役割切替 */
                SwitchRunGrpRole();

                /* 実行中タスクグループ設定 */
                pRunGrp = &gSchedTbl.runGrp[ gSchedTbl.runningGrpIdx ];

                /* 実行中レベル初期化 */
                level = SCHED_LEVEL_KERNEL;

                /* 再スケジューリング */
                continue;
            }
        }

        /* 実行中レベル加算 */
        level++;

    /* デキュー結果判定 */
    } while ( pNextTaskInfo == NULL );

    /* 実行中タスク情報切り替え */
    gSchedTbl.pRunTaskInfo = pNextTaskInfo;

    /* タスク比較 */
    if ( pRunTaskInfo != pNextTaskInfo ) {
        /* 別タスク */

        /* タスクスイッチ */
        SwitchTask( pRunTaskInfo, pNextTaskInfo );
    }

    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() end.", __func__ );*/

    return;
}


/******************************************************************************/
/**
 * @brief       タスクID取得
 * @details     現在実行中タスクのタスクIDを取得する。
 *
 * @return      タスクIDを返す。
 * @retval      MK_TASKID_MIN タスクID最小値
 * @retval      MK_TASKID_MAX タスクID最大値
 */
/******************************************************************************/
MkTaskId_t TaskmngSchedGetTaskId( void )
{
    /* タスクID返却 */
    return gSchedTbl.pRunTaskInfo->taskId;
}


/******************************************************************************/
/**
 * @brief       スケジュール開始
 * @details     タスクID taskId のタスクのスケジュールを開始する。
 *
 * @param[in]   taskId タスクID
 */
/******************************************************************************/
void TaskmngSchedStart( MkTaskId_t taskId )
{
    MLibRet_t     retMLib;      /* MLib関数戻り値 */
    TblTaskInfo_t *pTaskInfo;   /* タスク管理情報 */

    /* 初期化 */
    retMLib   = MLIB_RET_FAILURE;
    pTaskInfo = NULL;

    /* タスク管理情報取得 */
    pTaskInfo = TblGetTaskInfo( taskId );

    /* 待ちキューから削除 */
    retMLib = MLibListRemove( &( gSchedTbl.waitGrp.waitQ ),
                              &( pTaskInfo->nodeInfo )      );

    /* 削除結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        return;
    }

    /* 実行予約タスクグループにエンキュー */
    EnqueueToReservedGrp( pTaskInfo );

    return;
}


/******************************************************************************/
/**
 * @brief       スケジュール停止
 * @details     タスクID taskId のタスクのスケジュールを停止する。
 *
 * @param[in]   taskId タスクID
 */
/******************************************************************************/
void TaskmngSchedStop( MkTaskId_t taskId )
{
    MLibList_t    *pTaskQ0;     /* タスクキュー     */
    MLibList_t    *pTaskQ1;     /* タスクキュー     */
    TblTaskInfo_t *pTaskInfo;   /* タスク管理情報   */
    TblProcInfo_t *pProcInfo;   /* プロセス管理情報 */

    /* 初期化 */
    pTaskQ0   = NULL;
    pTaskQ1   = NULL;
    pTaskInfo = NULL;

    /* タスク管理情報取得 */
    pTaskInfo = TblGetTaskInfo( taskId );

    /* 取得結果判定 */
    if ( pTaskInfo == NULL ) {
        /* 失敗 */

        return;
    }

    /* プロセス管理情報取得 */
    pProcInfo = pTaskInfo->pProcInfo;

    /* 実行中タスク判定 */
    if ( pTaskInfo != gSchedTbl.pRunTaskInfo ) {
        /* 実行中タスクでない */

        /* プロセスタイプ判定 */
        if ( pProcInfo->type == TASKMNG_PROC_TYPE_KERNEL ) {
            /* カーネル */

            /* タスクキュー取得 */
            pTaskQ0 = &gSchedTbl.runGrp[ 0 ].kernelQ;
            pTaskQ1 = &gSchedTbl.runGrp[ 1 ].kernelQ;

        } else if ( pProcInfo->type == TASKMNG_PROC_TYPE_DRIVER ) {
            /* ドライバ */

            /* タスクキュー取得 */
            pTaskQ0 = &gSchedTbl.runGrp[ 0 ].driverQ;
            pTaskQ1 = &gSchedTbl.runGrp[ 1 ].driverQ;

        } else if ( pProcInfo->type == TASKMNG_PROC_TYPE_SERVER ) {
            /* サーバ */

            /* タスクキュー取得 */
            pTaskQ0 = &gSchedTbl.runGrp[ 0 ].serverQ;
            pTaskQ1 = &gSchedTbl.runGrp[ 1 ].serverQ;

        } else {
            /* ユーザ */

            /* タスクキュー取得 */
            pTaskQ0 = &gSchedTbl.runGrp[ 0 ].userQ;
            pTaskQ1 = &gSchedTbl.runGrp[ 1 ].userQ;
        }

        /* タスクキューから削除 */
        MLibListRemove( pTaskQ0, &( pTaskInfo->nodeInfo ) );
        MLibListRemove( pTaskQ1, &( pTaskInfo->nodeInfo ) );
    }

    /* 待ちキューにエンキュー */
    MLibListInsertHead( &( gSchedTbl.waitGrp.waitQ ),
                        &( pTaskInfo->nodeInfo )      );

    /* 実行状態設定 */
    pTaskInfo->state = STATE_WAIT;

    return;
}


/******************************************************************************/
/* モジュール内グローバル関数定義                                             */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       スケジュール追加
 * @details     タスク管理情報pTaskInfoで管理するタスクをスケジューラに追加し、
 *              スケジューリングを開始する。
 *
 * @param[in]   *pTaskInfo タスク管理情報
 *
 * @return      処理結果を返す。
 * @retval      CMN_SUCCESS 成功
 * @retval      CMN_FAILURE 失敗
 */
/******************************************************************************/
CmnRet_t SchedAdd( TblTaskInfo_t *pTaskInfo )
{
    /* 実行予約タスクグループにエンキュー */
    EnqueueToReservedGrp( pTaskInfo );

    return CMN_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       実行中プロセス管理情報取得
 * @details     現在実行中のタスクのプロセス管理情報を取得する。
 *
 * @return      プロセス管理情報を返す。
 */
/******************************************************************************/
TblProcInfo_t *SchedGetProcInfo( void )
{
    return gSchedTbl.pRunTaskInfo->pProcInfo;
}


/******************************************************************************/
/**
 * @brief       実行中タスク管理情報取得
 * @details     現在実行中のタスクの管理情報を取得する。
 *
 * @return      タスク管理情報を返す。
 */
/******************************************************************************/
TblTaskInfo_t *SchedGetTaskInfo( void )
{
    return gSchedTbl.pRunTaskInfo;
}


/******************************************************************************/
/**
 * @brief       実行中スレッド管理情報取得
 * @details     現在実行中のタスクのスレッド管理情報を取得する。
 *
 * @return      スレッド管理情報を返す。
 */
/******************************************************************************/
TblThreadInfo_t *SchedGetThreadInfo( void )
{
    return gSchedTbl.pRunTaskInfo;
}


/******************************************************************************/
/**
 * @brief       スケジューラ初期化
 * @details     スケジューラサブモジュールの初期化を行う。
 */
/******************************************************************************/
void SchedInit( void )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* スケジューラテーブル初期化 */
    MLibUtilSetMemory8( &gSchedTbl, 0, sizeof ( schedTbl_t ) );

    /* 実行可能タスクグループIDX設定 */
    gSchedTbl.runningGrpIdx  = 0;
    gSchedTbl.reservedGrpIdx = 1;

    /* タスク実行済フラグ初期化 */
    gSchedTbl.runFlg = false;

    /* アイドルタスク管理情報取得 */
    gSchedTbl.pIdleTaskInfo = TblGetTaskInfo( TASKMNG_TASKID_IDLE );
    gSchedTbl.pRunTaskInfo  = gSchedTbl.pIdleTaskInfo;

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       実行予約タスクグループエンキュー
 * @details     実行予約タスクグループにタスク情報をキューイングする。
 *
 * @param[in]   *pTaskInfo タスク情報
 */
/******************************************************************************/
static void EnqueueToReservedGrp( TblTaskInfo_t *pTaskInfo )
{
    uint8_t         taskType;         /* タスクタイプ           */
    MLibRet_t       retMLib;          /* MLib関数戻り値         */
    schedRunGrp_t   *pReservedGrp;    /* 実行予約タスクグループ */
    MLibList_t      *pSchedQ;         /* スケジュールキュー     */

    /* 初期化 */
    taskType     = TASKMNG_PROC_TYPE_USER;
    pSchedQ      = NULL;
    pReservedGrp = &gSchedTbl.runGrp[ gSchedTbl.reservedGrpIdx ];
    retMLib      = MLIB_RET_FAILURE;

    /* デバッグトレースログ出力 */
    /*DEBUG_LOG( "%s() start. pTaskInfo=%010p", __func__, pTaskInfo );*/

    /* プロセスタイプ取得 */
    taskType = pTaskInfo->pProcInfo->type;

    /* プロセスタイプ判定 */
    if ( taskType == TASKMNG_PROC_TYPE_KERNEL ) {
        /* カーネル */

        /* エンキュー先をカーネルキューに設定 */
        pSchedQ = &pReservedGrp->kernelQ;

    } else if ( taskType == TASKMNG_PROC_TYPE_DRIVER ) {
        /* ドライバ */

        /* エンキュー先をドライバキューに設定 */
        pSchedQ = &pReservedGrp->driverQ;

    } else if ( taskType == TASKMNG_PROC_TYPE_SERVER ) {
        /* サーバ */

        /* エンキュー先をサーバキューに設定 */
        pSchedQ = &pReservedGrp->serverQ;

    } else {
        /* ユーザ */

        /* エンキュー先をユーザキューに設定 */
        pSchedQ = &( pReservedGrp->userQ );
    }

    /* エンキュー */
    retMLib =  MLibListInsertHead( pSchedQ,
                                   &( pTaskInfo->nodeInfo ) );

    /* エンキュー結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        /* [TODO] */
    }

    /* 実行状態設定 */
    pTaskInfo->state = STATE_RUN;

    /* デバッグトレースログ出力 */
    /*DEBUG_LOG( "%s() end.", __func__ );*/

    return;
}


/******************************************************************************/
/**
 * @brief       実行可能タスクグループ役割切替
 * @details     実行可能タスクグループの役割を切り替える。
 */
/******************************************************************************/
static void SwitchRunGrpRole( void )
{
    /* デバッグトレースログ出力 */
    /*DEBUG_LOG( "%s() start.", __func__ );*/

    /* グループ役割切替 */
    gSchedTbl.runningGrpIdx  ^= 1;
    gSchedTbl.reservedGrpIdx ^= 1;

    /* タスク実行済みフラグ初期化 */
    gSchedTbl.runFlg = false;

    /* デバッグトレースログ出力 */
    /*DEBUG_LOG( "%s() end.", __func__ );*/

    return;
}


/******************************************************************************/
/**
 * @brief       タスクスイッチ
 * @details     現在実行中タスクのコンテキストを保存し、指定されたタスクIDのコ
 *              ンテキストを復元してタスクスイッチする。
 *
 * @param[in]   *pRunTaskInfo  実行中タスク管理情報
 * @param[in]   *pNextTaskInfo タスクスイッチ先タスク管理情報
 */
/******************************************************************************/
static __attribute__ ( ( noinline ) )
    void SwitchTask( TblTaskInfo_t *pRunTaskInfo,
                     TblTaskInfo_t *pNextTaskInfo )
{
    void          *pKernelStack;    /* カーネルスタック     */
    TblContext_t  *pRunContext;     /* 現タスクコンテキスト */
    TblContext_t  *pNextContext;    /* 次タスクコンテキスト */
    TblProcInfo_t *pNextProcInfo;   /* 次プロセス管理情報   */

    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() start. runTaskId=%u, nextTaskId=%u",
               __func__,
               pRunTaskInfo->taskId,
               pNextTaskInfo->taskId );*/

    /* 初期化 */
    pNextProcInfo = pNextTaskInfo->pProcInfo;
    pRunContext   = &( pRunTaskInfo->context );
    pNextContext  = &( pNextTaskInfo->context );
    pKernelStack  = pNextTaskInfo->kernelStack.pBottomAddr;

    /* カーネルスタック設定 */
    TssSetEsp0( ( uint32_t ) pKernelStack );

    /* ページディレクトリ切替 */
    MemmngPageSwitchDir( pNextProcInfo->dirId );

    /* コンテキスト退避 */
    pRunContext->eip = ( uint32_t ) SwitchTaskEnd;
    pRunContext->esp = IA32InstructionGetEsp();
    pRunContext->ebp = IA32InstructionGetEbp();

    /* タスクスイッチ */
    __asm__ __volatile__ ( "mov eax, %0\n"
                           "mov ebx, %1\n"
                           "mov esp, %2\n"
                           "mov ebp, %3\n"
                           "mov cr3, eax\n"
                           "jmp ebx"
                           :
                           : "m" ( pNextProcInfo->pdbr ),
                             "m" ( pNextContext->eip   ),
                             "m" ( pNextContext->esp   ),
                             "m" ( pNextContext->ebp   )
                           : "eax",
                             "ebx",
                             "ebp",
                             "esp"                        );

    /* ラベル */
    __asm__ __volatile__ ( "SwitchTaskEnd:" );

    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() end.", __func__ );*/

    return;
}


/******************************************************************************/
