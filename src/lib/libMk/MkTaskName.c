/******************************************************************************/
/*                                                                            */
/* src/lib/libMk/MkTaskName.c                                                 */
/*                                                                 2019/06/12 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdint.h>

/* 共通ヘッダ */
#include <kernel/taskname.h>

/* 内部モジュールヘッダ */


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       タスクID取得
 * @details     文字列pTaskNameに一致するタスク名を登録したタスクのタスクIDを
 *              pTaskIdに設定する。タスクID取得に失敗した場合はエラー番号を
 *              pErrNoに設定する。
 *
 * @param[in]   *pTaskName タスク名
 * @param[out]  *pTaskId   タスクID
 * @param[out]  *pErrNo    エラー番号
 *                  - MK_TASKNAME_ERR_NONE         エラー無し
 *                  - MK_TASKNAME_ERR_PARAM_NAME   タスク名不正
 *                  - MK_TASKNAME_ERR_NOREGISTERED 登録無し
 *
 * @return      取得結果を返す。
 * @retval      MK_TASKNAME_RET_SUCCESS 成功
 * @retval      MK_TASKNAME_RET_FAILURE 失敗
 */
/******************************************************************************/
int32_t MkTaskNameGet( char       *pTaskName,
                       MkTaskId_t *pTaskId,
                       uint32_t   *pErrNo     )
{
    volatile MkTaskNameParam_t param;

    /* パラメータ設定 */
    param.funcId    = MK_TASKNAME_FUNCID_GET;
    param.errNo     = MK_TASKNAME_ERR_NONE;
    param.ret       = MK_TASKNAME_RET_FAILURE;
    param.taskId    = MK_TASKID_NULL;
    param.pTaskName = pTaskName;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param                   ),
                             "i" ( MK_CONFIG_INTNO_TASKNAME )  );

    /* タスクID設定 */
    *pTaskId = param.taskId;

    /* エラー番号設定要否判定 */
    if ( pErrNo != NULL ) {
        /* 必要 */

        /* エラー番号設定 */
        *pErrNo = param.errNo;
    }

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       タスク名登録
 * @details     関数を呼び出したタスクをタスク名pTaskNameで登録する。タスク名登
 *              録に失敗した場合はエラー番号をpErrNoに設定する。
 *
 * @param[in]   *pTaskName タスク名
 * @param[out]  *pErrNo    エラー番号
 *                  - MK_TASKNAME_ERR_NONE         エラー無し
 *                  - Mk_TASKNAME_ERR_PARAM_NAME   タスク名不正
 *                  - MK_TASKNAME_ERR_UNAUTHORIZED 権限無し
 *                  - MK_TASKNAME_ERR_REGISTERED   登録済み
 *                  - MK_TASKNAME_ERR_FULL         登録上限
 *
 * @return      登録結果を返す。
 * @retval      MK_TASKNAME_RET_SUCCESS 成功
 * @retval      MK_TASKNAME_RET_FAILURE 失敗
 */
/******************************************************************************/
int32_t MkTaskNameRegister( char     *pTaskName,
                            uint32_t *pErrNo     )
{
    volatile MkTaskNameParam_t param;   /* パラメータ */

    /* パラメータ設定 */
    param.funcId    = MK_TASKNAME_FUNCID_REGISTER;
    param.errNo     = MK_TASKNAME_ERR_NONE;
    param.ret       = MK_TASKNAME_RET_FAILURE;
    param.taskId    = MK_TASKID_NULL;
    param.pTaskName = pTaskName;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param                   ),
                             "i" ( MK_CONFIG_INTNO_TASKNAME )  );

    /* エラー番号設定要否判定 */
    if ( pErrNo != NULL ) {
        /* 必要 */

        /* エラー番号設定 */
        *pErrNo = param.errNo;
    }

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       タスク名登録解除
 * @details     関数を呼び出したタスクのタスク名登録を解除する。タスク名登録解
 *              除に失敗した場合はエラー番号をpErrNoに設定する。
 *
 * @param[out]  *pErrNo エラー番号
 *                  - MK_TASKNAME_ERR_NONE         エラー無し
 *                  - MK_TASKNAME_ERR_UNAUTHORIZED 権限無し
 *                  - MK_TASKNAME_ERR_NOREGISTERED 登録無し
 *
 * @return      登録解除結果を返す。
 * @retval      MK_TASKNAME_RET_SUCCESS 成功
 * @retval      MK_TASKNAME_RET_FAILURE 失敗
 */
/******************************************************************************/
int32_t MkTaskNameUnregister( uint32_t *pErrNo )
{
    volatile MkTaskNameParam_t param;   /* パラメータ */

    /* パラメータ設定 */
    param.funcId    = MK_TASKNAME_FUNCID_UNREGISTER;
    param.errNo     = MK_TASKNAME_ERR_NONE;
    param.ret       = MK_TASKNAME_RET_FAILURE;
    param.taskId    = MK_TASKID_NULL;
    param.pTaskName = NULL;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param                   ),
                             "i" ( MK_CONFIG_INTNO_TASKNAME )  );

    /* エラー番号設定要否判定 */
    if ( pErrNo != NULL ) {
        /* 必要 */

        /* エラー番号設定 */
        *pErrNo = param.errNo;
    }

    return param.ret;
}


/******************************************************************************/
