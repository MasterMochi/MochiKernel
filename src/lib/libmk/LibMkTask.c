/******************************************************************************/
/*                                                                            */
/* src/lib/libmk/LibMkTask.c                                                  */
/*                                                                 2019/08/28 */
/* Copyright (C) 2019 Mochi.                                                  */
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
#include <kernel/task.h>


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       タスクID取得
 * @details     関数を呼び出したタスクのタスクIDを引数*pTaskIdに設定する。取得
 *              に失敗した場合はエラー内容をpErrに設定する。
 *
 * @param[in]   *pTaskId タスクID
 * @param[out]  *pErr    エラー内容
 *                  - MK_ERR_NONE  エラー無し
 *                  - Mk_ERR_PARAM パラメータ不正
 *
 * @return      取得結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkTaskGetId( MkTaskId_t *pTaskId,
                        MkErr_t    *pErr     )
{
    volatile MkTaskParam_t param;   /* パラメータ */

    /* 引数チェック */
    if ( pTaskId == NULL ) {
        /* 不正 */

        /* エラー内容設定 */
        MLIB_SET_IFNOT_NULL( pErr, MK_ERR_PARAM );

        return MK_RET_FAILURE;
    }

    /* パラメータ設定 */
    param.funcId    = MK_TASK_FUNCID_GET_ID;
    param.ret       = MK_RET_FAILURE;
    param.err       = MK_ERR_NONE;
    param.taskId    = MK_TASKID_NULL;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param        ),
                             "i" ( MK_TASK_INTNO )
                           : "esi"                  );

    /* エラー内容設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    /* タスクID設定 */
    *pTaskId = param.taskId;

    return param.ret;
}


/******************************************************************************/
