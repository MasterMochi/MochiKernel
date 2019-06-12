/******************************************************************************/
/*                                                                            */
/* src/kernel/TaskMng/TaskMngTask.c                                           */
/*                                                                 2019/06/12 */
/* Copyright (C) 2017-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <hardware/IA32/IA32Instruction.h>
#include <kernel/config.h>
#include <kernel/types.h>
#include <MLib/MLib.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <MemMng.h>
#include <TaskMng.h>

/* 内部モジュールヘッダ */
#include "TaskMngElf.h"
#include "TaskMngProc.h"
#include "TaskMngSched.h"
#include "TaskMngTask.h"
#include "TaskMngTss.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                        \
    DebugLogOutput( CMN_MODULE_TASKMNG_TASK,    \
                    __LINE__,                   \
                    __VA_ARGS__              )
#else
#define DEBUG_LOG( ... )
#endif

/** タスクスタック情報 */
typedef struct {
    void                 *pTopAddr;         /**< 先頭アドレス */
    void                 *pBottomAddr;      /**< 終端アドレス */
    size_t               size;              /**< サイズ       */
} TaskStackInfo_t;

/** タスク管理テーブル構造体 */
typedef struct {
    uint8_t              used;              /**< 使用フラグ               */
    uint8_t              state;             /**< タスク状態               */
    uint8_t              reserved[ 2 ];     /**< パディング               */
    MkPid_t              pid;               /**< プロセスID               */
    MkTid_t              tid;               /**< スレッドID               */
    TaskStackInfo_t      aplStackInfo;      /**< アプリスタックアドレス   */
    TaskStackInfo_t      kernelStackInfo;   /**< カーネルスタックアドレス */
    TaskMngTaskContext_t context;           /**< コンテキスト             */
} TaskTbl_t;


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** タスク管理テーブル */
static TaskTbl_t gTaskTbl[ MK_TASKID_NUM ];


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       タスク追加
 * @details     タスク追加を行う。
 *
 * @param[in]   pid       プロセスID
 * @param[in]   tid       スレッドID
 * @param[in]   pageDirId ページディレクトリID
 * @param[in]   *pAddr    実行アドレス
 *
 * @return      追加時に割り当てたタスクIDを返す。
 * @retval      MK_TASKID_NULL 失敗
 * @retval      MK_TASKID_MIN  タスクID最小値
 * @retval      MK_TASKID_MAX  タスクID最大値
 */
/******************************************************************************/
MkTaskId_t TaskMngTaskAdd( MkPid_t  pid,
                           MkTid_t  tid,
                           uint32_t pageDirId,
                           void     *pAddr     )
{
    void            *pAplStack;      /* アプリスタック       */
    void            *pKernelStack;   /* カーネルスタック     */
    CmnRet_t        ret;             /* 関数戻り値           */
    MkTaskId_t      taskId;          /* タスクID             */
    TaskStackInfo_t *pStackInfo;     /* スタック情報         */

    /* 初期化 */
    pAplStack    = NULL;
    pKernelStack = NULL;
    ret          = CMN_FAILURE;
    taskId       = MK_TASKID_MIN;
    pStackInfo   = NULL;

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    DEBUG_LOG( "  pid=%u, tid=%u, pageDirId=%u, pAddr=%010p",
               pid,
               tid,
               pageDirId,
               pAddr                                          );

    /* 空タスク検索 */
    for ( taskId = MK_TASKID_MIN; taskId < MK_TASKID_MAX; taskId++ ) {
        /* 使用フラグ判定 */
        if ( gTaskTbl[ taskId ].used == CMN_UNUSED ) {
            /* 未使用 */

            /* タスク基本設定 */
            gTaskTbl[ taskId ].used         = CMN_USED;
            gTaskTbl[ taskId ].state        = 0;
            gTaskTbl[ taskId ].pid          = pid;
            gTaskTbl[ taskId ].tid          = tid;
            gTaskTbl[ taskId ].context.eip  = ( uint32_t ) pAddr;
            gTaskTbl[ taskId ].context.esp  = MK_CONFIG_ADDR_KERNEL_STACK +
                                              MK_CONFIG_SIZE_KERNEL_STACK -
                                              sizeof ( uint32_t );

            /* カーネルスタック情報設定 */
            pStackInfo              = &( gTaskTbl[ taskId ].kernelStackInfo );
            pStackInfo->pTopAddr    = ( void * ) MK_CONFIG_ADDR_KERNEL_STACK;
            pStackInfo->pBottomAddr = ( void * )
                                      ( MK_CONFIG_ADDR_KERNEL_STACK +
                                        MK_CONFIG_SIZE_KERNEL_STACK -
                                        sizeof ( uint32_t )            );
            pStackInfo->size        = MK_CONFIG_SIZE_KERNEL_STACK;

            /* アプリスタック情報設定 */
            pStackInfo              = &( gTaskTbl[ taskId ].aplStackInfo );
            pStackInfo->pTopAddr    = ( void * ) MK_CONFIG_ADDR_APL_STACK;
            pStackInfo->pBottomAddr = ( void * )
                                      ( MK_CONFIG_ADDR_APL_STACK +
                                        MK_CONFIG_SIZE_APL_STACK -
                                        sizeof ( uint32_t )          );
            pStackInfo->size        = MK_CONFIG_SIZE_APL_STACK;

            /* カーネルスタック領域割り当て */
            pKernelStack = MemMngPhysAlloc( MK_CONFIG_SIZE_KERNEL_STACK );

            /* 割り当て結果判定 */
            if ( pKernelStack == NULL ) {
                /* 失敗 */

                /* [TODO] */

                return MK_TASKID_NULL;
            }

            /* アプリスタック領域割り当て */
            pAplStack = MemMngPhysAlloc( MK_CONFIG_SIZE_APL_STACK );

            /* 割り当て結果判定 */
            if ( pAplStack == NULL ) {
                /* 失敗 */

                /* [TODO] */

                return MK_TASKID_NULL;
            }

            /* カーネルスタックページマップ設定 */
            ret = MemMngPageSet( pageDirId,
                                 ( void * ) MK_CONFIG_ADDR_KERNEL_STACK,
                                 pKernelStack,
                                 MK_CONFIG_SIZE_KERNEL_STACK,
                                 IA32_PAGING_G_NO,
                                 IA32_PAGING_US_SV,
                                 IA32_PAGING_RW_RW );

            /* ページマップ設定結果判定 */
            if ( ret != CMN_SUCCESS ) {
                /* 失敗 */

                /* [TODO] */

                return MK_TASKID_NULL;
            }

            /* アプリスタックページマップ設定 */
            ret = MemMngPageSet( pageDirId,
                                 ( void * ) MK_CONFIG_ADDR_APL_STACK,
                                 pAplStack,
                                 MK_CONFIG_SIZE_APL_STACK,
                                 IA32_PAGING_G_NO,
                                 IA32_PAGING_US_USER,
                                 IA32_PAGING_RW_RW );

            /* ページマップ設定結果判定 */
            if ( ret != CMN_SUCCESS ) {
                /* 失敗 */

                /* [TODO] */

                return MK_TASKID_NULL;
            }

            /* デバッグトレースログ出力 */
            DEBUG_LOG( "%s() end. ret=%u", __func__, taskId );

            return taskId;
        }
    }

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%u", __func__, MK_TASKID_NULL );

    return MK_TASKID_NULL;
}


/******************************************************************************/
/**
 * @brief       タスク存在確認
 * @details     指定したタスクIDのタスクが存在するか確認する。
 *
 * @param[in]   taskId タスクID
 *
 * @return      タスク有無を返す。
 * @retval      true  タスク有
 * @retval      false タスク無
 */
/******************************************************************************/
bool TaskMngTaskCheckExist( MkTaskId_t taskId )
{
    return gTaskTbl[ taskId ].used == CMN_USED;
}


/******************************************************************************/
/**
 * @brief       アプリスタックアドレス取得
 * @details     指定したタスクIDのアプリスタックの終端アドレスを取得する。
 *
 * @param[in]   taskId タスクID
 *                  - MK_TASKID_MIN タスクID最小値
 *                  - MK_TASKID_MAX タスクID最大値
 *
 * @return      終端アドレスを返す。
 */
/******************************************************************************/
void *TaskMngTaskGetAplStack( MkTaskId_t taskId )
{
    /* アプリスタックアドレス */
    return gTaskTbl[ taskId ].aplStackInfo.pBottomAddr;
}


/******************************************************************************/
/**
 * @brief       コンテキスト取得
 * @details     指定したタスクIDのコンテキストを取得する。
 *
 * @param[in]   taskId タスクID
 *                  - MK_TASKID_MIN タスクID最小値
 *                  - MK_TASKID_MAX タスクID最大値
 *
 * @return      コンテキストを返す。
 */
/******************************************************************************/
TaskMngTaskContext_t TaskMngTaskGetContext( MkTaskId_t taskId )
{
    /* コンテキスト返却 */
    return gTaskTbl[ taskId ].context;
}


/******************************************************************************/
/**
 * @brief       カーネルスタックアドレス取得
 * @details     指定したタスクIDのカーネルスタックの終端アドレスを取得する。
 *
 * @param[in]   taskId タスクID
 *                  - MK_TASKID_MIN タスクID最小値
 *                  - MK_TASKID_MAX タスクID最大値
 *
 * @return      終端アドレスを返す。
 */
/******************************************************************************/
void *TaskMngTaskGetKernelStack( MkTaskId_t taskId )
{
    /* カーネルスタックアドレス返却 */
    return gTaskTbl[ taskId ].kernelStackInfo.pBottomAddr;
}


/******************************************************************************/
/**
 * @brief       ページディレクトリID取得
 * @details     指定したタスクIDのページディレクトリIDを取得する。
 *
 * @param[in]   taskId タスクID
 *                  - MK_TASKID_MIN タスクID最小値
 *                  - MK_TASKID_MAX タスクID最大値
 *
 * @return      ページディレクトリIDを返す。
 */
/******************************************************************************/
uint32_t TaskMngTaskGetPageDirId( MkTaskId_t taskId )
{
    /* ページディレクトリID返却 */
    return TaskMngProcGetPageDirId( gTaskTbl[ taskId ].pid );
}


/******************************************************************************/
/**
 * @brief       プロセスID取得
 * @details     指定したタスクIDのプロセスIDを取得する。
 *
 * @param[in]   taskId タスクID
 *                  - MK_TASKID_MIN タスクID最小値
 *                  - MK_TASKID_MAX タスクID最大値
 *
 * @return      プロセスIDを返す。
 * @retval      MK_PID_MIN プロセスID最小値
 * @retval      MK_PID_MAX プロセスID最大値
 */
/******************************************************************************/
MkPid_t TaskMngTaskGetPid( MkTaskId_t taskId )
{
    /* プロセスID返却 */
    return gTaskTbl[ taskId ].pid;
}


/******************************************************************************/
/**
 * @brief       プロセスタイプ取得
 * @details     指定したタスクIDのプロセスタイプを取得する。
 *
 * @param[in]   taskId タスクID
 *                  - MK_TASKID_MIN タスクID最小値
 *                  - MK_TASKID_MAX タスクID最大値
 *
 * @return      プロセスタイプを返す。
 * @retval      TASKMNG_PROC_TYPE_KERNEL カーネル
 * @retval      TASKMNG_PROC_TYPE_DRIVER ドライバ
 * @retval      TASKMNG_PROC_TYPE_SERVER サーバ
 * @retval      TASKMNG_PROC_TYPE_USER   ユーザ
 */
/******************************************************************************/
uint8_t TaskMngTaskGetType( MkTaskId_t taskId )
{
    /* タスクタイプ返却 */
    return TaskMngProcGetType( gTaskTbl[ taskId ].pid );
}


/******************************************************************************/
/**
 * @brief       プロセスタイプ差取得
 * @details     指定したタスクIDのプロセスタイプを比較し階層差を取得する。
 *
 * @param[in]   taskId1 タスクID
 * @param[in]   taskId2 タスクID
 *
 * @return      プロセスタイプ差を返す。
 * @retval      0 差0
 * @retval      1 差1
 * @retval      2 差2
 * @retval      3 差3
 */
/******************************************************************************/
uint8_t TaskMngTaskGetTypeDiff( MkTaskId_t taskId1,
                                MkTaskId_t taskId2  )
{
    uint8_t type1;  /* プロセスタイプ */
    uint8_t type2;  /* プロセスタイプ */

    /* プロセスタイプ取得 */
    type1 = TaskMngProcGetType( taskId1 );
    type2 = TaskMngProcGetType( taskId2 );

    return MLIB_MAX( type1, type2 ) - MLIB_MIN( type1, type2 );
}


/******************************************************************************/
/**
 * @brief       タスク管理初期化
 * @details     タスク管理サブモジュールの初期化を行う。
 */
/******************************************************************************/
void TaskMngTaskInit( void )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* タスク管理テーブル初期化 */
    memset( gTaskTbl, 0, sizeof ( gTaskTbl ) );

    /* アイドルタスク設定 */
    gTaskTbl[ TASKMNG_TASKID_IDLE ].used = CMN_USED;
    gTaskTbl[ TASKMNG_TASKID_IDLE ].pid  = TASKMNG_PID_IDLE;
    gTaskTbl[ TASKMNG_TASKID_IDLE ].tid  = 0;

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/**
 * @brief       コンテキスト設定
 * @details     指定したタスクIDのコンテキストを設定する。
 *
 * @param[in]   taskId    設定先タスクID
 *                  - MK_TASKID_MIN タスクID最小値
 *                  - MK_TASKID_MAX タスクID最大値
 * @param[in]   *pContext コンテキスト
 */
/******************************************************************************/
void TaskMngTaskSetContext( MkTaskId_t           taskId,
                            TaskMngTaskContext_t *pContext )
{
    /* コンテキスト設定 */
    gTaskTbl[ taskId ].context = *pContext;

    return;
}


/******************************************************************************/
