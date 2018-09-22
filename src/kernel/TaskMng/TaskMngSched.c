/******************************************************************************/
/* src/kernel/TaskMng/TaskMngSched.c                                          */
/*                                                                 2018/06/17 */
/* Copyright (C) 2017-2018 Mochi.                                             */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <MLib/MLib.h>
#include <MLib/Basic/MLibBasicList.h>
#include <hardware/IA32/IA32Instruction.h>
#include <kernel/config.h>
#include <kernel/types.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <MemMng.h>

/* 内部モジュールヘッダ */
#include "TaskMngTask.h"
#include "TaskMngTss.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                        \
    DebugLogOutput( CMN_MODULE_TASKMNG_SCHED,   \
                    __LINE__,                   \
                    __VA_ARGS__ )
#else
#define DEBUG_LOG( ... )
#endif

/** 実行可能タスクグループ数 */
#define SCHED_RUNGRP_NUM       ( 2 )

/** アイドルタスク情報インデックス */
#define SCHED_IDLE_IDX         ( 0 )

/* タスク情報使用フラグ */
#define SCHED_TASK_INFO_UNUSED ( 0 )    /** 未使用 */
#define SCHED_TASK_INFO_USED   ( 1 )    /** 使用中 */

/* スケジューラ実行中レベル */
#define SCHED_LEVEL_KERNEL     ( 0 )    /** カーネルレベル */
#define SCHED_LEVEL_DRIVER     ( 1 )    /** ドライバレベル */
#define SCHED_LEVEL_SERVER     ( 2 )    /** サーバレベル   */
#define SCHED_LEVEL_USER       ( 3 )    /** ユーザレベル   */

/* 実行状態 */
#define STATE_RUN              ( 0 )    /** 実行状態 */
#define STATE_WAIT             ( 1 )    /** 待ち状態 */

/** タスク情報構造体 */
typedef struct {
    MLibBasicListNode_t node;           /**< 連結リスト情報 */
    uint8_t             used;           /**< 使用フラグ     */
    uint8_t             state;          /**< 実行状態       */
    uint8_t             reserved[ 2 ];  /**< 予約           */
    MkTaskId_t          taskId;         /**< タスクID       */
} schedTaskInfo_t;

/** 実行可能タスクグループ構造体 */
typedef struct {
    MLibBasicList_t kernelQ;            /**< カーネルタスクキュー */
    MLibBasicList_t driverQ;            /**< ドライバタスクキュー */
    MLibBasicList_t serverQ;            /**< サーバタスクキュー   */
    MLibBasicList_t userQ;              /**< ユーザタスクキュー   */
} schedRunGrp_t;

/** 待ちタスクグループ構造体 */
typedef struct {
    MLibBasicList_t waitQ;              /**< 待ちキュー */
} schedWaitGrp_t;

/** スケジューラテーブル構造体 */
typedef struct {
    schedTaskInfo_t *pRunningTaskInfo;                  /**< 実行中タスク情報        */
    uint32_t        runFlg;                             /**< タスク実行済みフラグ    */
    uint32_t        runningGrpIdx;                      /**< 実行中タスクグループIDX */
    uint32_t        reservedGrpIdx;                     /**< 予約タスクグループIDX   */
    schedRunGrp_t   runGrp[ SCHED_RUNGRP_NUM ];         /**< 実行可能タスクグループ  */
    schedWaitGrp_t  waitGrp;                            /**< 待ちタスクグループ      */
    MLibBasicList_t freeQ;                              /**< 空タスクキュー          */
    schedTaskInfo_t taskInfo[ MK_CONFIG_TASKID_NUM ];   /**< タスク情報              */
} schedTbl_t;


/******************************************************************************/
/* ローカル関数プロトタイプ宣言                                               */
/******************************************************************************/
/* 実行予約タスクグループエンキュー */
static void SchedEnqueueToReservedGrp( schedTaskInfo_t *pTaskInfo );

/* 実行可能タスクグループ役割切替 */
static void SchedSwitchRunGrpRole( void );

/* タスクスイッチ */
static void SchedSwitchTask( MkTaskId_t nowTaskId,
                             MkTaskId_t nextTaskId );

/* タスクスイッチ終了ポイント（ラベル） */
void SchedSwitchTaskEnd( void );


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
 * @brief       スケジュール追加
 * @details     指定したタスクIDをスケジュールに追加する。
 * 
 * @param[in]   taskId タスクID
 *                  - MK_CONFIG_TASKID_MIN タスクID最小値
 *                  - MK_CONFIG_TASKID_MAX タスクID最大値
 * 
 * @retval      CMN_SUCCESS 成功
 * @retval      CMN_FAILURE 失敗
 */
/******************************************************************************/
CmnRet_t TaskMngSchedAdd( MkTaskId_t taskId )
{
    schedTaskInfo_t *pTaskInfo; /* タスク情報 */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start. taskId=%u", __func__, taskId );
    
    /* 空タスクキューから空タスク情報取得 */
    pTaskInfo =
        ( schedTaskInfo_t * ) MLibBasicListRemoveTail( &( gSchedTbl.freeQ ) );
    
    /* 取得結果判定 */
    if ( pTaskInfo == NULL ) {
        /* 空タスク情報無し */
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=%d", __func__, CMN_FAILURE );
        
        return CMN_FAILURE;
    }
    
    /* タスク情報設定 */
    pTaskInfo->used   = SCHED_TASK_INFO_USED;   /* 使用フラグ */
    pTaskInfo->state  = STATE_RUN;              /* 実行状態   */
    pTaskInfo->taskId = taskId;                 /* タスクID   */
    
    /* 実行予約タスクグループにエンキュー */
    SchedEnqueueToReservedGrp( pTaskInfo );
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%d", __func__, CMN_SUCCESS );
    
    return CMN_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       スケジューラ実行
 * @details     スケジューラを実行して次に実行可能なタスクにタスクスイッチする。
 */
/******************************************************************************/
void TaskMngSchedExec( void )
{
    uint32_t        level;          /* 実行中レベル         */
    MkTaskId_t      nowTaskId;      /* 実行中タスクID       */
    schedRunGrp_t   *pRunningGrp;   /* 実行中タスクグループ */
    schedTaskInfo_t *pTaskInfo;     /* タスク情報           */
    
    /* 初期化 */
    level       = SCHED_LEVEL_KERNEL;
    nowTaskId   = gSchedTbl.pRunningTaskInfo->taskId;
    pRunningGrp = NULL;
    pTaskInfo   = NULL;
    
    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() start.", __func__ );*/
    
    /* 実行中タスク判定 */
    if ( nowTaskId == TASKMNG_TASKID_IDLE ) {
        /* アイドルタスク */
        
        /* 実行可能タスクグループ役割切替 */
        SchedSwitchRunGrpRole();
        
    } else {
        /* アイドルタスク以外 */
        
        /* 実行中タスク状態判定 */
        if ( gSchedTbl.pRunningTaskInfo->state == STATE_RUN ) {
            /* 実行状態 */
            
            /* 実行予約タスクグループにキューイング */
            SchedEnqueueToReservedGrp( gSchedTbl.pRunningTaskInfo );
            
            /* タスク実行済みフラグ設定 */
            gSchedTbl.runFlg = true;
            
        } else {
            /* 待ち状態 */
            
            /* タスク実行済みフラグ設定 */
            gSchedTbl.runFlg = false;
        }
    }
    
    /* 実行中タスクグループ設定 */
    pRunningGrp = &gSchedTbl.runGrp[ gSchedTbl.runningGrpIdx ];
    
    do {
        /* 実行中レベル判定 */
        if ( level == SCHED_LEVEL_KERNEL ) {
            /* カーネルレベル */
            
            /* カーネルキューからデキュー */
            pTaskInfo = ( schedTaskInfo_t * )
                MLibBasicListRemoveTail( &( pRunningGrp->kernelQ ) );
            
        } else if ( level == SCHED_LEVEL_DRIVER ) {
            /* ドライバレベル */
            
            /* ドライバキューからデキュー */
            pTaskInfo = ( schedTaskInfo_t * )
                MLibBasicListRemoveTail( &( pRunningGrp->driverQ ) );
            
        } else if ( level == SCHED_LEVEL_SERVER ) {
            /* サーバレベル */
            
            /* サーバキューからデキュー */
            pTaskInfo = ( schedTaskInfo_t * )
                MLibBasicListRemoveTail( &( pRunningGrp->serverQ ) );
            
        } else if ( level == SCHED_LEVEL_USER ) {
            /* ユーザレベル */
            
            /* ユーザキューからデキュー */
            pTaskInfo = ( schedTaskInfo_t * )
                MLibBasicListRemoveTail( &( pRunningGrp->userQ ) );
            
        } else {
            /* 他 */
            
            /* タスク実行済フラグ判定 */
            if ( gSchedTbl.runFlg == false ) {
                /* 実行済みタスク無 */
                
                /* アイドルタスク設定 */
                pTaskInfo = &gSchedTbl.taskInfo[ SCHED_IDLE_IDX ];
                
                break;
                
            } else {
                /* 実行済みタスク有 */
                
                /* 実行可能タスクグループ役割切替 */
                SchedSwitchRunGrpRole();
                
                /* 実行中タスクグループ設定 */
                pRunningGrp = &gSchedTbl.runGrp[ gSchedTbl.runningGrpIdx ];
                
                /* 実行中レベル初期化 */
                level = SCHED_LEVEL_KERNEL;
                
                /* 再スケジューリング */
                continue;
            }
        }
        
        /* 実行中レベル加算 */
        level++;
        
    /* デキュー結果判定 */
    } while ( pTaskInfo == NULL );
    
    /* 実行中タスク情報切り替え */
    gSchedTbl.pRunningTaskInfo = pTaskInfo;
    
    /* タスクID比較 */
    if ( nowTaskId != pTaskInfo->taskId ) {
        /* 別タスク */
        
        /* タスクスイッチ */
        SchedSwitchTask( nowTaskId, pTaskInfo->taskId );
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
 * @return      タスクID
 * @retval      MK_CONFIG_TASKID_MIN タスクID最小値
 * @retval      MK_CONFIG_TASKID_MAX タスクID最大値
 */
/******************************************************************************/
MkTaskId_t TaskMngSchedGetTaskId( void )
{
    /* タスクID返却 */
    return gSchedTbl.pRunningTaskInfo->taskId;
}


/******************************************************************************/
/**
 * @brief       スケジューラ初期化
 * @details     スケジューラサブモジュールの初期化を行う。
 */
/******************************************************************************/
void TaskMngSchedInit( void )
{
    MkTaskId_t      taskId;         /* タスクID           */
    MLibRet_t       retMLib;        /* MLib関数戻り値     */
    schedTaskInfo_t *pIdleTaskInfo; /* アイドルタスク情報 */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* スケジューラテーブル0初期化 */
    memset( &gSchedTbl, 0, sizeof ( schedTbl_t ) );
    
    /* 実行可能タスクグループIDX設定 */
    gSchedTbl.runningGrpIdx  = 0;
    gSchedTbl.reservedGrpIdx = 1;
    
    /* タスク実行済フラグ初期化 */
    gSchedTbl.runFlg = false;
    
    /* アイドルタスク情報初期化 */
    pIdleTaskInfo              = &gSchedTbl.taskInfo[ SCHED_IDLE_IDX ];
    gSchedTbl.pRunningTaskInfo = pIdleTaskInfo;
    pIdleTaskInfo->used        = SCHED_TASK_INFO_USED;
    pIdleTaskInfo->taskId      = TASKMNG_TASKID_IDLE;
    
    /* 空タスクキュー初期化 */
    for ( taskId  = SCHED_IDLE_IDX + 1;
          taskId <= MK_CONFIG_TASKID_MAX;
          taskId++                       ) {
        /* エンキュー */
        retMLib = 
            MLibBasicListInsertHead( &( gSchedTbl.freeQ ),
                                     &( gSchedTbl.taskInfo[ taskId ].node ) );
        
        /* エンキュー結果判定 */
        if ( retMLib != MLIB_SUCCESS ) {
            /* 失敗 */
            
            /* [TODO] */
        }
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
/**
 * @brief       スケジュール開始
 * @details     指定したタスクのスケジュールを開始する。
 * 
 * @param[in]   taskId タスクID
 */
/******************************************************************************/
void TaskMngSchedStart( MkTaskId_t taskId )
{
    schedTaskInfo_t *pTaskInfo; /* タスク情報 */
    
    /* 初期化 */
    pTaskInfo = NULL;
    
    /* 待ちキュータスク情報検索 */
    do {
        /* 次タスク情報取得 */
        pTaskInfo = ( schedTaskInfo_t * )
            MLibBasicListGetNextNode( &gSchedTbl.waitGrp.waitQ,
                                      &pTaskInfo->node          );
        
        /* 取得結果判定 */
        if ( pTaskInfo == NULL ) {
            /* タスク無し */
            
            return;
        }
        
    } while ( pTaskInfo->taskId != taskId );
    
    /* 待ちキューから削除 */
    MLibBasicListRemove( &gSchedTbl.waitGrp.waitQ,
                         &pTaskInfo->node          );
    
    /* 実行予約タスクグループにエンキュー */
    SchedEnqueueToReservedGrp( pTaskInfo );
    
    return;
}


/******************************************************************************/
/**
 * @brief       スケジュール停止
 * @details     指定したタスクのスケジュールを停止する。
 * 
 * @param[in]   taskId タスクID
 */
/******************************************************************************/
void TaskMngSchedStop( MkTaskId_t taskId )
{
    uint8_t         type;       /* プロセスタイプ */
    MLibBasicList_t *pTaskQ0;   /* タスクキュー   */
    MLibBasicList_t *pTaskQ1;   /* タスクキュー   */
    schedTaskInfo_t *pTaskInfo; /* タスク情報     */
    
    /* 初期化 */
    type      = 0;
    pTaskQ0   = NULL;
    pTaskQ1   = NULL;
    pTaskInfo = NULL;
    
    /* 実行中タスク判定 */
    if ( gSchedTbl.pRunningTaskInfo->taskId == taskId ) {
        /* 実行中タスク */
        
        /* 待ちキューにエンキュー */
        MLibBasicListInsertHead( &gSchedTbl.waitGrp.waitQ,
                                 &( gSchedTbl.pRunningTaskInfo->node ) );
        
        /* 実行状態設定 */
        gSchedTbl.pRunningTaskInfo->state = STATE_WAIT;
    }
    
    /* プロセスタイプ取得 */
    type = TaskMngTaskGetType( taskId );
    
    /* プロセスタイプ判定 */
    if ( type == TASKMNG_PROC_TYPE_KERNEL ) {
        /* カーネル */
        
        /* タスクキュー取得 */
        pTaskQ0 = &gSchedTbl.runGrp[ 0 ].kernelQ;
        pTaskQ1 = &gSchedTbl.runGrp[ 1 ].kernelQ;
        
    } else if ( type == TASKMNG_PROC_TYPE_DRIVER ) {
        /* ドライバ */
        
        /* タスクキュー取得 */
        pTaskQ0 = &gSchedTbl.runGrp[ 0 ].driverQ;
        pTaskQ1 = &gSchedTbl.runGrp[ 1 ].driverQ;
        
    } else if ( type == TASKMNG_PROC_TYPE_SERVER ) {
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
    
    /* 先頭タスク情報取得 */
    pTaskInfo = ( schedTaskInfo_t * )
        MLibBasicListGetNextNode( pTaskQ0, NULL );
    
    /* タスク検索 */
    while ( pTaskInfo != NULL ) {
        /* 対象タスク情報判定 */
        if ( pTaskInfo->taskId == taskId ) {
            /* 対象 */
            
            /* タスクキューから削除 */
            MLibBasicListRemove( pTaskQ0, &pTaskInfo->node );
            
            /* 待ちキューにエンキュー */
            MLibBasicListInsertHead( &gSchedTbl.waitGrp.waitQ,
                                     &( pTaskInfo->node )      );
            
            /* 実行状態設定 */
            pTaskInfo->state = STATE_WAIT;
            
            return;
        }
        
        /* 次タスク情報取得 */
        pTaskInfo = ( schedTaskInfo_t * )
            MLibBasicListGetNextNode( pTaskQ0, &pTaskInfo->node );
    }
    
    /* 先頭タスク情報取得 */
    pTaskInfo = ( schedTaskInfo_t * )
        MLibBasicListGetNextNode( pTaskQ1, NULL );
    
    /* タスク検索 */
    while ( pTaskInfo != NULL ) {
        /* 対象タスク情報判定 */
        if ( pTaskInfo->taskId == taskId ) {
            /* 対象 */
            
            /* タスクキューから削除 */
            MLibBasicListRemove( pTaskQ1, &pTaskInfo->node );
            
            /* 待ちキューにエンキュー */
            MLibBasicListInsertHead( &gSchedTbl.waitGrp.waitQ,
                                     &( pTaskInfo->node )      );
            
            /* 実行状態設定 */
            pTaskInfo->state = STATE_WAIT;
            
            return;
        }
        
        /* 次タスク情報取得 */
        pTaskInfo = ( schedTaskInfo_t * )
            MLibBasicListGetNextNode( pTaskQ1, &pTaskInfo->node );
    }
    
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
static void SchedEnqueueToReservedGrp( schedTaskInfo_t *pTaskInfo )
{
    uint8_t         taskType;         /* タスクタイプ           */
    MLibRet_t       retMLib;          /* MLib関数戻り値         */
    schedRunGrp_t   *pReservedGrp;    /* 実行予約タスクグループ */
    MLibBasicList_t *pTaskQ;          /* タスクキュー           */
    
    /* 初期化 */
    taskType     = TASKMNG_PROC_TYPE_USER;
    pTaskQ       = NULL;
    pReservedGrp = &gSchedTbl.runGrp[ gSchedTbl.reservedGrpIdx ];
    retMLib      = MLIB_FAILURE;
    
    /* デバッグトレースログ出力 */
    /*DEBUG_LOG( "%s() start. pTaskInfo=%010p", __func__, pTaskInfo );*/
    
    /* プロセスタイプ取得 */
    taskType = TaskMngTaskGetType( pTaskInfo->taskId );
    
    /* プロセスタイプ判定 */
    if ( taskType == TASKMNG_PROC_TYPE_KERNEL ) {
        /* カーネル */
        
        /* タスクキュー設定 */
        pTaskQ = &pReservedGrp->kernelQ;
        
    } else if ( taskType == TASKMNG_PROC_TYPE_DRIVER ) {
        /* ドライバ */
        
        /* タスクキュー設定 */
        pTaskQ = &pReservedGrp->driverQ;
        
    } else if ( taskType == TASKMNG_PROC_TYPE_SERVER ) {
        /* サーバ */
        
        /* タスクキュー設定 */
        pTaskQ = &pReservedGrp->serverQ;
        
    } else {
        /* ユーザ */
        
        /* タスクキュー設定 */
        pTaskQ = &pReservedGrp->userQ;
    }
    
    /* エンキュー */
    retMLib =  MLibBasicListInsertHead( pTaskQ,
                                        &( pTaskInfo->node ) );
    
    /* エンキュー結果判定 */
    if ( retMLib != MLIB_SUCCESS ) {
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
static void SchedSwitchRunGrpRole( void )
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
 * @param[in]   nowTaskId  現在実行中のタスクID
 * @param[in]   nextTaskId タスクスイッチ先タスクID
 */
/******************************************************************************/
static __attribute__ ( ( noinline ) )
    void SchedSwitchTask( MkTaskId_t nowTaskId,
                          MkTaskId_t nextTaskId )
{
    void                 *pKernelStack;    /* カーネルスタック     */
    uint32_t             pageDirId;        /* ページディレクトリID */
    IA32PagingPDBR_t     pdbr;             /* PDBR                 */
    TaskMngTaskContext_t nowContext;       /* 現タスクコンテキスト */
    TaskMngTaskContext_t nextContext;      /* 次タスクコンテキスト */
    
    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() start. nowTaskId=%u, nextTaskId=%u",
               __func__,
               nowTaskId,
               nextTaskId );*/
    
    /* コンテキスト取得 */
    nextContext = TaskMngTaskGetContext( nextTaskId );
    
    /* カーネルスタック設定 */
    pKernelStack = TaskMngTaskGetKernelStack( nextTaskId );
    TaskMngTssSetEsp0( ( uint32_t ) pKernelStack );
    
    /* ページディレクトリID取得 */
    pageDirId = TaskMngTaskGetPageDirId( nextTaskId );
    
    /* ページディレクトリ切替 */
    pdbr = MemMngPageSwitchDir( pageDirId );
    
    /* コンテキスト退避 */
    nowContext.eip = ( uint32_t ) SchedSwitchTaskEnd;
    nowContext.esp = IA32InstructionGetEsp();
    nowContext.ebp = IA32InstructionGetEbp();
    TaskMngTaskSetContext( nowTaskId, &nowContext );
    
    /* タスクスイッチ */
    __asm__ __volatile__ ( "mov eax, %0\n"
                           "mov ebx, %1\n"
                           "mov esp, %2\n"
                           "mov ebp, %3\n"
                           "mov cr3, eax\n"
                           "jmp ebx"
                           :
                           : "m" ( pdbr ),
                             "m" ( nextContext.eip ),
                             "m" ( nextContext.esp ),
                             "m" ( nextContext.ebp )
                           : "eax",
                             "ebx",
                             "ebp",
                             "esp"                    );
    
    /* ラベル */
    __asm__ __volatile__ ( "SchedSwitchTaskEnd:" );
    
    /* デバッグトレースログ出力 *//*
    DEBUG_LOG( "%s() end.", __func__ );*/
    
    return;
}


/******************************************************************************/
