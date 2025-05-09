/******************************************************************************/
/*                                                                            */
/* src/kernel/Taskmng/TaskmngName.c                                           */
/*                                                                 2024/06/02 */
/* Copyright (C) 2019-2024 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdint.h>

/* ライブラリヘッダ */
#include <MLib/MLibUtil.h>

/* カーネルヘッダ */
#include <kernel/config.h>
#include <kernel/taskname.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32Descriptor.h>

/* モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Intmng.h>
#include <Taskmng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* モジュールID */
#define _MODULE_ID_ CMN_MODULE_TASKMNG_NAME

/** タスク名管理テーブルエントリ */
typedef struct {
    MkTaskId_t taskId;                                      /**< タスクID */
    char       taskName[ MK_CONFIG_TASKNAME_LENMAX + 1 ];   /**< タスク名 */
} TaskNameEntry_t;


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* タスクID取得 */
static void doGet( MkTaskNameParam_t *pParam );
/* タスク名登録 */
static void doRegister( MkTaskNameParam_t *pParam );
/* タスク名登録解除 */
static void doUnregister( MkTaskNameParam_t *pParam );
/* 未使用タスク名管理テーブルエントリ取得 */
static TaskNameEntry_t *GetUnusedEntry( void );
/* 割込みハンドラ */
static void HdlInt( uint32_t        intNo,
                    IntmngContext_t context );
/* タスクID検索 */
static TaskNameEntry_t *SearchTaskId( MkTaskId_t taskId );
/* タスク名検索 */
static TaskNameEntry_t *SearchTaskName( char *pTaskName );


/******************************************************************************/
/* グローバル変数定義                                                         */
/******************************************************************************/
/* タスク名管理テーブル */
static TaskNameEntry_t gTaskNameTbl[ MK_CONFIG_TASKNAME_NUM ];


/******************************************************************************/
/* モジュール内グローバル関数定義                                             */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       タスク名管理初期化
 * @details     タスク名管理テーブルを初期化し、タスク名管理提供カーネルコール
 *              用の割込みハンドラを設定する。
 */
/******************************************************************************/
void NameInit( void )
{
    uint32_t idx;   /* インデックス */

    DEBUG_LOG_TRC( "%s() start.", __func__ );

    /* タスク名管理テーブルエントリ毎に繰り返す */
    for ( idx = 0; idx < MK_CONFIG_TASKNAME_NUM; idx++ ) {
        /* タスクID初期化　*/
        gTaskNameTbl[ idx ].taskId = MK_TASKID_NULL;

        /* タスク名初期化 */
        MLibUtilSetMemory8( gTaskNameTbl[ idx ].taskName,
                            0,
                            MK_CONFIG_TASKNAME_LENMAX + 1 );
    }

    /* 割込みハンドラ設定 */
    IntmngHdlSet( MK_CONFIG_INTNO_TASKNAME,     /* 割込み番号     */
                  HdlInt,                       /* 割込みハンドラ */
                  IA32_DESCRIPTOR_DPL_3     );  /* 特権レベル     */

    DEBUG_LOG_TRC( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief           タスクID取得
 * @details         タスク名に該当するタスク名管理テーブルエントリからタスクID
 *                  を取得する。
 *
 * @param[in,out]   *pParam パラメータ
 */
/******************************************************************************/
static void doGet( MkTaskNameParam_t *pParam )
{
    TaskNameEntry_t *pEntry;    /* タスク名検索結果 */

    /* パラメータチェック */
    if ( pParam->pTaskName == NULL ) {
        /* 不正 */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_PARAM;

        return;
    }

    /* タスク名検索 */
    pEntry = SearchTaskName( pParam->pTaskName );

    /* 検索結果判定 */
    if ( pEntry == NULL ) {
        /* 該当無し */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_NO_REGISTERED;

        return;
    }

    /* 戻り値設定 */
    pParam->ret    = MK_RET_SUCCESS;
    pParam->err    = MK_ERR_NONE;
    pParam->taskId = pEntry->taskId;

    return;
}


/******************************************************************************/
/**
 * @brief           タスク名登録
 * @details         タスク名管理テーブルにタスク名タスクIDの登録を行う。
 *
 * @param[in,out]   *pParam パラメータ
 */
/******************************************************************************/
static void doRegister( MkTaskNameParam_t *pParam )
{
    uint8_t         type;       /* プロセスタイプ   */
    MkTaskId_t      taskId;     /* タスクID         */
    TaskNameEntry_t *pEntry;    /* タスク名検索結果 */

    /* パラメータチェック */
    if ( pParam->pTaskName == NULL ) {
        /* 不正 */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_PARAM;

        return;
    }

    /* タスクID取得 */
    taskId = TaskmngSchedGetTaskId();

    /* プロセスタイプ取得 */
    type = TaskmngTaskGetType( taskId );

    if ( type == TASKMNG_PROC_TYPE_USER ) {
        /* ユーザプロセス　*/

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_UNAUTHORIZED;

        return;
    }

    /* タスク名検索 */
    pEntry = SearchTaskName( pParam->pTaskName );

    /* 検索結果判定 */
    if ( pEntry != NULL ) {
        /* 該当エントリ有り */

        /* タスクID判定 */
        if ( pEntry->taskId == taskId ) {
            /* 一致 */

            /* 戻り値設定 */
            pParam->ret = MK_RET_SUCCESS;
            pParam->err = MK_ERR_NONE;

        } else {
            /* 不一致 */

            /* エラー設定 */
            pParam->ret = MK_RET_FAILURE;
            pParam->err = MK_ERR_REGISTERED;
        }

        return;
    }

    /* タスクID検索 */
    pEntry = SearchTaskId( taskId );

    /* 検索結果判定 */
    if ( pEntry != NULL ) {
        /* 該当エントリ有り */

        /* タスク名変更 */
        MLibUtilCopyMemory( pEntry->taskName,
                            pParam->pTaskName,
                            MK_CONFIG_TASKNAME_LENMAX + 1 );

        /* 戻り値設定 */
        pParam->ret = MK_RET_SUCCESS;
        pParam->err = MK_ERR_NONE;

        return;
    }

    /* 未使用エントリ取得 */
    pEntry = GetUnusedEntry();

    /* 取得結果判定 */
    if ( pEntry == NULL ) {
        /* 失敗 */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_NO_RESOURCE;

        return;
    }

    /* 登録 */
    pEntry->taskId = taskId;
    MLibUtilCopyMemory( pEntry->taskName,
                        pParam->pTaskName,
                        MK_CONFIG_TASKNAME_LENMAX + 1 );

    /* 戻り値設定 */
    pParam->ret = MK_RET_SUCCESS;
    pParam->err = MK_ERR_NONE;

    return;
}


/******************************************************************************/
/**
 * @brief       タスク名登録解除
 * @details     タスクIDに該当するタスク名管理テーブルエントリを初期化し、タス
 *              ク名の登録を解除する。
 *
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void doUnregister( MkTaskNameParam_t *pParam )
{
    uint8_t         type;       /* プロセスタイプ */
    MkTaskId_t      taskId;     /* タスクID       */
    TaskNameEntry_t *pEntry;    /* エントリ       */

    /* タスクID取得 */
    taskId = TaskmngSchedGetTaskId();

    /* プロセスタイプ取得 */
    type = TaskmngTaskGetType( taskId );

    /* プロセスタイプチェック */
    if ( type == TASKMNG_PROC_TYPE_USER ) {
        /* ユーザプロセス */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_UNAUTHORIZED;

        return;
    }

    /* タスクID検索 */
    pEntry = SearchTaskId( taskId );

    /* 検索結果判定 */
    if ( pEntry == NULL ) {
        /* 該当エントリ無し */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_NO_REGISTERED;

        return;
    }

    /* 該当エントリ初期化 */
    pEntry->taskId = MK_TASKID_NULL;
    MLibUtilSetMemory8( pEntry->taskName, 0, MK_CONFIG_TASKNAME_LENMAX + 1 );

    /* 戻り値設定 */
    pParam->ret = MK_RET_SUCCESS;
    pParam->err = MK_ERR_NONE;

    return;
}


/******************************************************************************/
/**
 * @brief       未使用タスク名管理テーブルエントリ取得
 * @details     タスク名管理テーブルから未使用のエントリを取得する。
 *
 * @return      タスク名管理テーブルのエントリを返す。
 * @retval      NULL以外 成功
 * @retval      NULL     失敗(未使用エントリ無し)
 */
/******************************************************************************/
static TaskNameEntry_t *GetUnusedEntry( void )
{
    uint32_t idx;   /* インデックス */

    /* タスク名管理テーブルエントリ毎に繰り返す */
    for ( idx = 0; idx < MK_CONFIG_TASKNAME_NUM; idx++ ) {
        /* 未使用エントリ判定 */
        if ( gTaskNameTbl[ idx ].taskId == MK_TASKID_NULL ) {
            /* 未使用エントリ */

            return &( gTaskNameTbl[ idx ] );
        }
    }

    return NULL;
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
    MkTaskNameParam_t *pParam;  /* パラメータ */

    /* 初期化 */
    pParam = ( MkTaskNameParam_t * ) context.genReg.esi;

    /* パラメータチェック */
    if ( pParam == NULL ) {
        /* 不正 */

        return;
    }

    /* 機能ID判定 */
    switch ( pParam->funcId ) {
        case MK_TASKNAME_FUNCID_GET:
            /* タスクID取得 */

            /* DEBUG_LOG( "%s(): get.", __func__ ); */
            doGet( pParam );
            break;

        case MK_TASKNAME_FUNCID_REGISTER:
            /* タスク名登録 */

            DEBUG_LOG_TRC( "%s(): register.", __func__ );
            doRegister( pParam );
            break;

        case MK_TASKNAME_FUNCID_UNREGISTER:
            /* タスク名登録解除 */

            DEBUG_LOG_TRC( "%s(): unregister.", __func__ );
            doUnregister( pParam );
            break;

        default:
            /* 不正 */

            /* エラー設定 */
            pParam->ret = MK_RET_FAILURE;
            pParam->err = MK_ERR_PARAM;
    }

    /*
    DEBUG_LOG_TRC( "%s(): end. ret=%d, err=%u",
                   __func__,
                   pParam->ret,
                   pParam->err                  );
    */
    return;
}


/******************************************************************************/
/**
 * @brief       タスクID検索
 * @details     指定したタスクIDに該当するタスク名管理テーブルのエントリを取得
 *              する。
 *
 * @param[in]   taskId タスクID
 *
 * @return      タスク名管理テーブルのエントリを返す。
 * @retval      NULL以外 該当エントリ有り
 * @retval      NULL     該当エントリ無し
 */
/******************************************************************************/
static TaskNameEntry_t *SearchTaskId( MkTaskId_t taskId )
{
    uint32_t idx;   /* インデックス */

    /* タスク名管理テーブルエントリ毎に繰り返す */
    for ( idx = 0; idx < MK_CONFIG_TASKNAME_NUM; idx++ ) {
        /* タスクID比較 */
        if ( gTaskNameTbl[ idx ].taskId == taskId ) {
            /* 一致 */

            return &( gTaskNameTbl[ idx ] );
        }
    }

    return NULL;
}


/******************************************************************************/
/**
 * @brief       タスク名検索
 * @details     指定したタスク名に該当するタスク名管理テーブルのエントリを取得
 *              する。
 *
 * @param[in]   *pTaskName タスク名
 *
 * @return      タスク名管理テーブルのエントリを返す。
 * @retval      NULL以外 該当エントリ有り
 * @retval      NULL     該当エントリ無し
 */
/******************************************************************************/
static TaskNameEntry_t *SearchTaskName( char *pTaskName )
{
    int      ret;   /* タスク名比較結果 */
    uint32_t idx;   /* インデックス     */

    /* タスク名管理テーブルエントリ毎に繰り返す */
    for ( idx = 0; idx < MK_CONFIG_TASKNAME_NUM; idx++ ) {
        /* タスク名比較 */
        ret = MLibUtilCmpString( gTaskNameTbl[ idx ].taskName,
                                 pTaskName,
                                 MK_CONFIG_TASKNAME_LENMAX + 1 );

        /* 比較結果判定 */
        if ( ret == 0 ) {
            /* 一致 */

            return &( gTaskNameTbl[ idx ] );
        }
    }

    return NULL;
}


/******************************************************************************/
