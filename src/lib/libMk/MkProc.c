/******************************************************************************/
/*                                                                            */
/* src/lib/libMk/MkProc.c                                                     */
/*                                                                 2019/07/03 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stddef.h>
#include <stdint.h>

/* 外部モジュールヘッダ */
#include <kernel/config.h>
#include <kernel/proc.h>

/* 内部モジュールヘッダ */


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       ブレイクポイント設定
 * @details     ブレイクポイントを設定する。
 *
 * @param[in]   quantity 増減量
 * @param[out]  *pErrNo  エラー番号
 *                  - MK_IOMEM_ERR_NONE エラー無し
 *
 * @return      割当結果を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功(ブレイクポイント)
 */
/******************************************************************************/
void *MkProcSetBreakPoint( int32_t  quantity,
                           uint32_t *pErrNo   )
{
    volatile MkProcParam_t param;

    /* パラメータ設定 */
    param.funcId      = MK_PROC_FUNCID_SET_BREAKPOINT;
    param.errNo       = MK_PROC_ERR_NONE;
    param.ret         = MK_PROC_RET_SUCCESS;
    param.pBreakPoint = NULL;
    param.quantity    = quantity;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param               ),
                             "i" ( MK_CONFIG_INTNO_PROC )
                           : "esi"                         );

    /* エラー番号設定要否判定 */
    if ( pErrNo != NULL ) {
        /* 必要 */

        /* エラー番号設定 */
        *pErrNo = param.errNo;
    }

    return param.pBreakPoint;
}


/******************************************************************************/
