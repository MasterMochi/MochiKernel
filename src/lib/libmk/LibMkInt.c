/******************************************************************************/
/*                                                                            */
/* src/lib/libmk/LibMkInt.c                                                   */
/*                                                                 2019/07/27 */
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
#include <kernel/interrupt.h>


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       ハードウェア割込み処理完了
 * @details     指定したIRQ番号のハードウェア割込み処理の完了を通知し、次の割込
 *              みを可能にする。
 *
 * @param[in]   irqNo IRQ番号
 *                  - LIBMK_INT_IRQ1  IRQ1番
 *                  - LIBMK_INT_IRQ3  IRQ3番
 *                  - LIBMK_INT_IRQ4  IRQ4番
 *                  - LIBMK_INT_IRQ5  IRQ5番
 *                  - LIBMK_INT_IRQ6  IRQ6番
 *                  - LIBMK_INT_IRQ7  IRQ7番
 *                  - LIBMK_INT_IRQ9  IRQ9番
 *                  - LIBMK_INT_IRQ10 IRQ10番
 *                  - LIBMK_INT_IRQ11 IRQ11番
 *                  - LIBMK_INT_IRQ12 IRQ12番
 *                  - LIBMK_INT_IRQ13 IRQ13番
 *                  - LIBMK_INT_IRQ14 IRQ14番
 *                  - LIBMK_INT_IRQ15 IRQ15番
 * @param[out]  *pErr エラー内容
 *                  - MK_ERR_NONE         エラー無し
 *                  - MK_ERR_PARAM        パラメータ不正
 *                  - MK_ERR_UNAUTHORIZED 権限無し
 *
 * @return      処理結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkIntComplete( uint8_t irqNo,
                          MkErr_t *pErr  )
{
    volatile MkIntParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_INT_FUNCID_COMPLETE;
    param.ret    = MK_RET_FAILURE;
    param.err    = MK_ERR_NONE;
    param.irqNo  = irqNo;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param       ),
                             "i" ( MK_INT_INTNO )
                           : "esi"                 );

    /* エラー設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       ハードウェア割込み無効化
 * @details     指定したIRQ番号のハードウェア割込みを無効化する。
 *
 * @param[in]   irqNo IRQ番号
 *                  - LIBMK_INT_IRQ1  IRQ1番
 *                  - LIBMK_INT_IRQ3  IRQ3番
 *                  - LIBMK_INT_IRQ4  IRQ4番
 *                  - LIBMK_INT_IRQ5  IRQ5番
 *                  - LIBMK_INT_IRQ6  IRQ6番
 *                  - LIBMK_INT_IRQ7  IRQ7番
 *                  - LIBMK_INT_IRQ9  IRQ9番
 *                  - LIBMK_INT_IRQ10 IRQ10番
 *                  - LIBMK_INT_IRQ11 IRQ11番
 *                  - LIBMK_INT_IRQ12 IRQ12番
 *                  - LIBMK_INT_IRQ13 IRQ13番
 *                  - LIBMK_INT_IRQ14 IRQ14番
 *                  - LIBMK_INT_IRQ15 IRQ15番
 * @param[out]  *pErr エラー内容
 *                  - MK_ERR_NONE         エラー無し
 *                  - MK_ERR_PARAM        パラメータ不正
 *                  - MK_ERR_UNAUTHORIZED 許可無し
 *
 * @return      処理結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t MkIntDisable( uint8_t irqNo,
                      MkErr_t *pErr  )
{
    volatile MkIntParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_INT_FUNCID_DISABLE;
    param.ret    = MK_RET_FAILURE;
    param.err    = MK_ERR_NONE;
    param.irqNo  = irqNo;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param       ),
                             "i" ( MK_INT_INTNO )
                           : "esi"                 );

    /* エラー設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       ハードウェア割込み有効化
 * @details     指定したIRQ番号のハードウェア割込みを有効化する。
 *
 * @param[in]   irqNo IRQ番号
 *                  - LIBMK_INT_IRQ1  IRQ1番
 *                  - LIBMK_INT_IRQ3  IRQ3番
 *                  - LIBMK_INT_IRQ4  IRQ4番
 *                  - LIBMK_INT_IRQ5  IRQ5番
 *                  - LIBMK_INT_IRQ6  IRQ6番
 *                  - LIBMK_INT_IRQ7  IRQ7番
 *                  - LIBMK_INT_IRQ9  IRQ9番
 *                  - LIBMK_INT_IRQ10 IRQ10番
 *                  - LIBMK_INT_IRQ11 IRQ11番
 *                  - LIBMK_INT_IRQ12 IRQ12番
 *                  - LIBMK_INT_IRQ13 IRQ13番
 *                  - LIBMK_INT_IRQ14 IRQ14番
 *                  - LIBMK_INT_IRQ15 IRQ15番
 * @param[out]  *pErr エラー内容
 *                  - MK_ERR_NONE         エラー無し
 *                  - MK_ERR_PARAM        パラメータ不正
 *                  - MK_ERR_UNAUTHORIZED 権限無し
 *
 * @return      処理結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkIntEnable( uint8_t irqNo,
                        MkErr_t *pErr  )
{
    volatile MkIntParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_INT_FUNCID_ENABLE;
    param.ret    = MK_RET_FAILURE;
    param.err    = MK_ERR_NONE;
    param.irqNo  = irqNo;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param       ),
                             "i" ( MK_INT_INTNO )
                           : "esi"                 );

    /* エラー設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       ハードウェア割込み監視開始
 * @details     指定したIRQ番号のハードウェア割込みの監視を開始する。
 *
 * @param[in]   irqNo IRQ番号
 *                  - LIBMK_INT_IRQ1  IRQ1番
 *                  - LIBMK_INT_IRQ3  IRQ3番
 *                  - LIBMK_INT_IRQ4  IRQ4番
 *                  - LIBMK_INT_IRQ5  IRQ5番
 *                  - LIBMK_INT_IRQ6  IRQ6番
 *                  - LIBMK_INT_IRQ7  IRQ7番
 *                  - LIBMK_INT_IRQ9  IRQ9番
 *                  - LIBMK_INT_IRQ10 IRQ10番
 *                  - LIBMK_INT_IRQ11 IRQ11番
 *                  - LIBMK_INT_IRQ12 IRQ12番
 *                  - LIBMK_INT_IRQ13 IRQ13番
 *                  - LIBMK_INT_IRQ14 IRQ14番
 *                  - LIBMK_INT_IRQ15 IRQ15番
 * @param[out]  *pErr エラー内容
 *                  - MK_ERR_NONE          エラー無し
 *                  - MK_ERR_PARAM         パラメータ不正
 *                  - MK_ERR_UNAUTHORIZED  権限無し
 *                  - MK_ERR_ALREADY_START 監視開始済み
 *
 * @return      処理結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkIntStartMonitoring( uint8_t irqNo,
                                 MkErr_t *pErr  )
{
    volatile MkIntParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_INT_FUNCID_START_MONITORING;
    param.ret    = MK_RET_FAILURE;
    param.err    = MK_ERR_NONE;
    param.irqNo  = irqNo;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param       ),
                             "i" ( MK_INT_INTNO )
                           : "esi"                 );

    /* エラー設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       ハードウェア割込み監視停止
 * @details     指定したIRQ番号のハードウェア割込みの監視を停止する。
 *
 * @param[in]   irqNo IRQ番号
 *                  - LIBMK_INT_IRQ1  IRQ1番
 *                  - LIBMK_INT_IRQ3  IRQ3番
 *                  - LIBMK_INT_IRQ4  IRQ4番
 *                  - LIBMK_INT_IRQ5  IRQ5番
 *                  - LIBMK_INT_IRQ6  IRQ6番
 *                  - LIBMK_INT_IRQ7  IRQ7番
 *                  - LIBMK_INT_IRQ9  IRQ9番
 *                  - LIBMK_INT_IRQ10 IRQ10番
 *                  - LIBMK_INT_IRQ11 IRQ11番
 *                  - LIBMK_INT_IRQ12 IRQ12番
 *                  - LIBMK_INT_IRQ13 IRQ13番
 *                  - LIBMK_INT_IRQ14 IRQ14番
 *                  - LIBMK_INT_IRQ15 IRQ15番
 * @param[out]  *pErr エラー内容
 *                  - MK_ERR_NONE         エラー無し
 *                  - MK_ERR_PARAM        パラメータ不正
 *                  - MK_ERR_UNAUTHORIZED 権限無し
 *
 * @return      処理結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkIntStopMonitoring( uint8_t irqNo,
                                MkErr_t *pErr  )
{
    volatile MkIntParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_INT_FUNCID_STOP_MONITORING;
    param.ret    = MK_RET_FAILURE;
    param.err    = MK_ERR_NONE;
    param.irqNo  = irqNo;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param       ),
                             "i" ( MK_INT_INTNO )
                           : "esi"                 );

    /* エラー設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       ハードウェア割込み待ち合わせ
 * @details     監視を開始しているハードウェア割込みの待ち合わせを行う。
 *
 * @param[out]  *pInt 割込み発生フラグ
 * @param[out]  *pErr エラー内容
 *                  - MK_ERR_NONE         エラー無し
 *                  - MK_ERR_PARAM        パラメータ不正
 *                  - MK_ERR_UNAUTHORIZED 権限無し
 *
 * @return      処理結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkIntWait( uint8_t *pInt,
                      MkErr_t *pErr  )
{
    volatile MkIntParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_INT_FUNCID_WAIT;
    param.ret    = MK_RET_FAILURE;
    param.err    = MK_ERR_NONE;
    param.flag   = 0;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param       ),
                             "i" ( MK_INT_INTNO )
                           : "esi"                 );

    /* 割込み発生フラグ設定 */
    MLIB_SET_IFNOT_NULL( pInt, param.flag );

    /* エラー設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    return param.ret;
}


/******************************************************************************/
