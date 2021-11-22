/******************************************************************************/
/*                                                                            */
/* src/lib/libmk/LibMkProc.c                                                  */
/*                                                                 2021/10/25 */
/* Copyright (C) 2018-2021 Mochi.                                             */
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
#include <kernel/proc.h>


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       プロセス複製
 * @details     プロセスを複製する。
 *
 * @param[out]  *pPid プロセスID
 *                  - MK_PID_NULL     複製元プロセス
 *                  - MK_PID_NULL以外 複製先プロセス
 * @param[out]  *pErr エラー内容
 *                  - MK_ERR_NONE       エラー無し
 *                  - MK_ERR_NO_RESOUCE リソース無し
 *
 * @return      複製結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkProcFork( MkPid_t *pPid,
                       MkErr_t *pErr  )
{
    volatile MkProcParam_t param;

    /* パラメータ設定 */
    param.funcId     = MK_PROC_FUNCID_FORK;
    param.ret        = MK_RET_SUCCESS;
    param.err        = MK_ERR_NONE;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param        ),
                             "i" ( MK_PROC_INTNO )
                           : "esi"                  );

    /* エラー内容設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    /* プロセスID設定 */
    MLIB_SET_IFNOT_NULL( pPid, param.pid );

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       ブレイクポイント設定
 * @details     ブレイクポイントを設定する。
 *
 * @param[in]   quantity       ブレイクポイント増減量
 * @param[out]  **ppBreakPoint ブレイクポイント
 * @param[out]  *pErr          エラー内容
 *                  - MK_ERR_NONE      エラー無し
 *                  - MK_ERR_NO_MEMORY メモリ不足
 *
 * @return      割当結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkProcSetBreakPoint( int32_t quantity,
                                void    **ppBreakPoint,
                                MkErr_t *pErr           )
{
    volatile MkProcParam_t param;

    /* パラメータ設定 */
    param.funcId      = MK_PROC_FUNCID_SET_BREAKPOINT;
    param.ret         = MK_RET_SUCCESS;
    param.err         = MK_ERR_NONE;
    param.pBreakPoint = NULL;
    param.quantity    = quantity;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param        ),
                             "i" ( MK_PROC_INTNO )
                           : "esi"                  );

    /* エラー内容設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    /* ブレイクポイント設定 */
    MLIB_SET_IFNOT_NULL( ppBreakPoint, param.pBreakPoint );

    return param.ret;
}


/******************************************************************************/
