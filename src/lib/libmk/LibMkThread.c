/******************************************************************************/
/*                                                                            */
/* src/lib/libmk/LibMkThread.c                                                */
/*                                                                 2019/08/28 */
/* Copyright (C) 2019 Mochi.                                                  */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stddef.h>
#include <stdint.h>

/* ライブラリヘッダ */
#include <MLib/MLib.h>

/* カーネルヘッダ */
#include <kernel/config.h>
#include <kernel/types.h>
#include <kernel/thread.h>


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       スレッド生成
 * @details     スレッドを生成する。
 *
 * @param[in]   pFunc       スレッドエントリ関数
 * @param[in]   *pArg       スレッドエントリ関数引数
 * @param[in]   *pStackAddr スタックアドレス
 * @param[in]   stackSize   スタックサイズ
 * @param[out]  *pTaskId    タスクID
 * @param[out]  *pErr       エラー内容
 *                  - MK_ERR_NONE        エラー無し
 *                  - MK_ERR_NO_RESOURCE リソース不足
 *
 * @return      割当結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkThreadCreate( MkThreadFunc_t pFunc,
                           void           *pArg,
                           void           *pStackAddr,
                           size_t         stackSize,
                           MkTaskId_t     *pTaskId,
                           MkErr_t        *pErr        )
{
    volatile MkThreadParam_t param;

    /* パラメータ設定 */
    param.funcId     = MK_THREAD_FUNCID_CREATE;
    param.ret        = MK_RET_SUCCESS;
    param.err        = MK_ERR_NONE;
    param.pFunc      = pFunc;
    param.pArg       = pArg;
    param.pStackAddr = pStackAddr;
    param.taskId     = MK_TASKID_NULL;
    param.stackSize  = stackSize;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param          ),
                             "i" ( MK_THREAD_INTNO )
                           : "esi"                    );

    /* エラー内容設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    /* タスクID設定 */
    MLIB_SET_IFNOT_NULL( pTaskId, param.taskId );

    return param.ret;
}


/******************************************************************************/
