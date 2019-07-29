/******************************************************************************/
/*                                                                            */
/* src/lib/libmk/LibMkTaskName.c                                              */
/*                                                                 2019/07/28 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdint.h>

/* ライブラリヘッダ */
#include <MLib/MLib.h>

/* カーネルヘッダ */
#include <kernel/taskname.h>


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       タスクID取得
 * @details     文字列pTaskNameに一致するタスク名を登録したタスクのタスクIDを
 *              pTaskIdに設定する。タスクID取得に失敗した場合はエラー内容をpErr
 *              に設定する。
 *
 * @param[in]   *pTaskName タスク名
 * @param[out]  *pTaskId   タスクID
 * @param[out]  *pErr      エラー内容
 *                  - MK_ERR_NONE          エラー無し
 *                  - MK_ERR_PARAM         パラメータ不正
 *                  - MK_ERR_NO_REGISTERED 登録無し
 *
 * @return      取得結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkTaskNameGet( char       *pTaskName,
                          MkTaskId_t *pTaskId,
                          MkErr_t    *pErr       )
{
    volatile MkTaskNameParam_t param;

    /* 引数チェック */
    if ( ( pTaskName == NULL ) ||
         ( pTaskId   == NULL )    ) {
        /* 不正 */

        /* エラー内容設定 */
        MLIB_SET_IFNOT_NULL( pErr, MK_ERR_PARAM );

        return MK_RET_FAILURE;
    }

    /* パラメータ設定 */
    param.funcId    = MK_TASKNAME_FUNCID_GET;
    param.ret       = MK_RET_FAILURE;
    param.err       = MK_ERR_NONE;
    param.taskId    = MK_TASKID_NULL;
    param.pTaskName = pTaskName;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param            ),
                             "i" ( MK_TASKNAME_INTNO )
                           : "esi"                      );

    /* エラー内容設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    /* タスクID設定 */
    *pTaskId = param.taskId;

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       タスク名登録
 * @details     関数を呼び出したタスクをタスク名pTaskNameで登録する。タスク名登
 *              録に失敗した場合はエラー内容をpErrに設定する。
 *
 * @param[in]   *pTaskName タスク名
 * @param[out]  *pErr      エラー内容
 *                  - MK_ERR_NONE         エラー無し
 *                  - Mk_ERR_PARAM        パラメータ不正
 *                  - MK_ERR_UNAUTHORIZED 権限無し
 *                  - MK_ERR_REGISTERED   登録済み
 *                  - MK_ERR_FULL         登録上限
 *
 * @return      登録結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkTaskNameRegister( char    *pTaskName,
                               MkErr_t *pErr       )
{
    volatile MkTaskNameParam_t param;   /* パラメータ */

    /* 引数チェック */
    if ( pTaskName == NULL ) {
        /* 不正 */

        /* エラー内容設定 */
        MLIB_SET_IFNOT_NULL( pErr, MK_ERR_PARAM );

        return MK_RET_FAILURE;
    }

    /* パラメータ設定 */
    param.funcId    = MK_TASKNAME_FUNCID_REGISTER;
    param.ret       = MK_RET_FAILURE;
    param.err       = MK_ERR_NONE;
    param.taskId    = MK_TASKID_NULL;
    param.pTaskName = pTaskName;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param            ),
                             "i" ( MK_TASKNAME_INTNO )
                           : "esi"                      );

    /* エラー内容設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       タスク名登録解除
 * @details     関数を呼び出したタスクのタスク名登録を解除する。タスク名登録解
 *              除に失敗した場合はエラー内容をpErrに設定する。
 *
 * @param[out]  *pErr エラー内容
 *                  - MK_ERR_NONE          エラー無し
 *                  - MK_ERR_UNAUTHORIZED  権限無し
 *                  - MK_ERR_NO_REGISTERED 登録無し
 *
 * @return      登録解除結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkTaskNameUnregister( MkErr_t *pErr )
{
    volatile MkTaskNameParam_t param;   /* パラメータ */

    /* パラメータ設定 */
    param.funcId    = MK_TASKNAME_FUNCID_UNREGISTER;
    param.ret       = MK_RET_FAILURE;
    param.err       = MK_ERR_NONE;
    param.taskId    = MK_TASKID_NULL;
    param.pTaskName = NULL;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param            ),
                             "i" ( MK_TASKNAME_INTNO )
                           : "esi"                      );

    /* エラー内容設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    return param.ret;
}


/******************************************************************************/
