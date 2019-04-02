/******************************************************************************/
/*                                                                            */
/* src/lib/libMk/MkInt.c                                                      */
/*                                                                 2019/04/02 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdint.h>

/* 外部モジュールヘッダ */
#include <kernel/interrupt.h>

/* 内部モジュールヘッダ */


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       ハードウェア割込み処理完了
 * @details     指定したIRQ番号のハードウェア割込み処理の完了を通知し、次の割込
 *              みを可能にする。
 *
 * @param[in]   irqNo   IRQ番号
 *                  - I8259A_IRQ1  IRQ1番
 *                  - I8259A_IRQ3  IRQ3番
 *                  - I8259A_IRQ4  IRQ4番
 *                  - I8259A_IRQ5  IRQ5番
 *                  - I8259A_IRQ6  IRQ6番
 *                  - I8259A_IRQ7  IRQ7番
 *                  - I8259A_IRQ9  IRQ9番
 *                  - I8259A_IRQ10 IRQ10番
 *                  - I8259A_IRQ11 IRQ11番
 *                  - I8259A_IRQ12 IRQ12番
 *                  - I8259A_IRQ13 IRQ13番
 *                  - I8259A_IRQ14 IRQ14番
 *                  - I8259A_IRQ15 IRQ15番
 * @param[out]  *pErrNo エラー番号
 *                  - MK_INT_ERR_NONE         エラー無し
 *                  - MK_INT_ERR_PARAM_IRQNO  IRQ番号不正
 *                  - MK_INT_ERR_UNAUTHORIZED 許可無し
 *
 * @return      処理結果を返す。
 * @retval      MK_INT_RET_SUCCESS 成功
 * @retval      MK_INT_RET_FAILURE 失敗（エラー番号を参照）
 */
/******************************************************************************/
int32_t MkIntComplete( uint8_t  irqNo,
                       uint32_t *pErrNo )
{
    volatile MkIntParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_INT_FUNCID_COMPLETE;
    param.errNo  = MK_INT_ERR_NONE;
    param.ret    = MK_INT_RET_FAILURE;
    param.irqNo  = irqNo;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param                    ),
                             "i" ( MK_CONFIG_INTNO_INTERRUPT )  );

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
 * @brief       ハードウェア割込み無効化
 * @details     指定したIRQ番号のハードウェア割込みを無効化する。
 *
 * @param[in]   irqNo   IRQ番号
 *                  - I8259A_IRQ1  IRQ1番
 *                  - I8259A_IRQ3  IRQ3番
 *                  - I8259A_IRQ4  IRQ4番
 *                  - I8259A_IRQ5  IRQ5番
 *                  - I8259A_IRQ6  IRQ6番
 *                  - I8259A_IRQ7  IRQ7番
 *                  - I8259A_IRQ9  IRQ9番
 *                  - I8259A_IRQ10 IRQ10番
 *                  - I8259A_IRQ11 IRQ11番
 *                  - I8259A_IRQ12 IRQ12番
 *                  - I8259A_IRQ13 IRQ13番
 *                  - I8259A_IRQ14 IRQ14番
 *                  - I8259A_IRQ15 IRQ15番
 * @param[out]  *pErrNo エラー番号
 *                  - MK_INT_ERR_NONE         エラー無し
 *                  - MK_INT_ERR_PARAM_IRQNO  IRQ番号不正
 *                  - MK_INT_ERR_UNAUTHORIZED 許可無し
 *
 * @return      処理結果を返す。
 * @retval      MK_INT_RET_SUCCESS 成功
 * @retval      MK_INT_RET_FAILURE 失敗（エラー番号を参照）
 */
/******************************************************************************/
int32_t MkIntDisable( uint8_t  irqNo,
                      uint32_t *pErrNo )
{
    volatile MkIntParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_INT_FUNCID_DISABLE;
    param.errNo  = MK_INT_ERR_NONE;
    param.ret    = MK_INT_RET_FAILURE;
    param.irqNo  = irqNo;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param                    ),
                             "i" ( MK_CONFIG_INTNO_INTERRUPT )  );

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
 * @brief       ハードウェア割込み有効化
 * @details     指定したIRQ番号のハードウェア割込みを有効化する。
 *
 * @param[in]   irqNo   IRQ番号
 *                  - I8259A_IRQ1  IRQ1番
 *                  - I8259A_IRQ3  IRQ3番
 *                  - I8259A_IRQ4  IRQ4番
 *                  - I8259A_IRQ5  IRQ5番
 *                  - I8259A_IRQ6  IRQ6番
 *                  - I8259A_IRQ7  IRQ7番
 *                  - I8259A_IRQ9  IRQ9番
 *                  - I8259A_IRQ10 IRQ10番
 *                  - I8259A_IRQ11 IRQ11番
 *                  - I8259A_IRQ12 IRQ12番
 *                  - I8259A_IRQ13 IRQ13番
 *                  - I8259A_IRQ14 IRQ14番
 *                  - I8259A_IRQ15 IRQ15番
 * @param[out]  *pErrNo エラー番号
 *                  - MK_INT_ERR_NONE         エラー無し
 *                  - MK_INT_ERR_PARAM_IRQNO  IRQ番号不正
 *                  - MK_INT_ERR_UNAUTHORIZED 許可無し
 *
 * @return      処理結果を返す。
 * @retval      MK_INT_RET_SUCCESS 成功
 * @retval      MK_INT_RET_FAILURE 失敗（エラー番号を参照）
 */
/******************************************************************************/
int32_t MkIntEnable( uint8_t  irqNo,
                     uint32_t *pErrNo )
{
    volatile MkIntParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_INT_FUNCID_ENABLE;
    param.errNo  = MK_INT_ERR_NONE;
    param.ret    = MK_INT_RET_FAILURE;
    param.irqNo  = irqNo;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param                    ),
                             "i" ( MK_CONFIG_INTNO_INTERRUPT )  );

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
 * @brief       ハードウェア割込み監視開始
 * @details     指定したIRQ番号のハードウェア割込みの監視を開始する。
 *
 * @param[in]   irqNo   IRQ番号
 *                  - I8259A_IRQ1  IRQ1番
 *                  - I8259A_IRQ3  IRQ3番
 *                  - I8259A_IRQ4  IRQ4番
 *                  - I8259A_IRQ5  IRQ5番
 *                  - I8259A_IRQ6  IRQ6番
 *                  - I8259A_IRQ7  IRQ7番
 *                  - I8259A_IRQ9  IRQ9番
 *                  - I8259A_IRQ10 IRQ10番
 *                  - I8259A_IRQ11 IRQ11番
 *                  - I8259A_IRQ12 IRQ12番
 *                  - I8259A_IRQ13 IRQ13番
 *                  - I8259A_IRQ14 IRQ14番
 *                  - I8259A_IRQ15 IRQ15番
 * @param[out]  *pErrNo エラー番号
 *                  - MK_INT_ERR_NONE          エラー無し
 *                  - MK_INT_ERR_PARAM_IRQNO   IRQ番号不正
 *                  - MK_INT_ERR_UNAUTHORIZED  許可無し
 *                  - MK_INT_ERR_ALREADY_START 監視開始済み
 *
 * @return      処理結果を返す。
 * @retval      MK_INT_RET_SUCCESS 成功
 * @retval      MK_INT_RET_FAILURE 失敗（エラー番号を参照）
 */
/******************************************************************************/
int32_t MkIntStartMonitoring( uint8_t  irqNo,
                              uint32_t *pErrNo )
{
    volatile MkIntParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_INT_FUNCID_START_MONITORING;
    param.errNo  = MK_INT_ERR_NONE;
    param.ret    = MK_INT_RET_FAILURE;
    param.irqNo  = irqNo;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param                    ),
                             "i" ( MK_CONFIG_INTNO_INTERRUPT )  );

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
 * @brief       ハードウェア割込み監視停止
 * @details     指定したIRQ番号のハードウェア割込みの監視を停止する。
 *
 * @param[in]   irqNo   IRQ番号
 *                  - I8259A_IRQ1  IRQ1番
 *                  - I8259A_IRQ3  IRQ3番
 *                  - I8259A_IRQ4  IRQ4番
 *                  - I8259A_IRQ5  IRQ5番
 *                  - I8259A_IRQ6  IRQ6番
 *                  - I8259A_IRQ7  IRQ7番
 *                  - I8259A_IRQ9  IRQ9番
 *                  - I8259A_IRQ10 IRQ10番
 *                  - I8259A_IRQ11 IRQ11番
 *                  - I8259A_IRQ12 IRQ12番
 *                  - I8259A_IRQ13 IRQ13番
 *                  - I8259A_IRQ14 IRQ14番
 *                  - I8259A_IRQ15 IRQ15番
 * @param[out]  *pErrNo エラー番号
 *                  - MK_INT_ERR_NONE         エラー無し
 *                  - MK_INT_ERR_PARAM_IRQNO  IRQ番号不正
 *                  - MK_INT_ERR_UNAUTHORIZED 許可無し
 *
 * @return      処理結果を返す。
 * @retval      MK_INT_RET_SUCCESS 成功
 * @retval      MK_INT_RET_FAILURE 失敗（エラー番号を参照）
 */
/******************************************************************************/
int32_t MkIntStopMonitoring( uint8_t  irqNo,
                             uint32_t *pErrNo )
{
    volatile MkIntParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_INT_FUNCID_STOP_MONITORING;
    param.errNo  = MK_INT_ERR_NONE;
    param.ret    = MK_INT_RET_FAILURE;
    param.irqNo  = irqNo;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param                    ),
                             "i" ( MK_CONFIG_INTNO_INTERRUPT )  );

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
 * @brief       ハードウェア割込み待ち合わせ
 * @details     監視を開始しているハードウェア割込みの待ち合わせを行う。
 *
 * @param[out]  *pInt   割込み発生フラグ
 * @param[out]  *pErrNo エラー番号
 *                  - MK_INT_ERR_NONE         エラー無し
 *                  - MK_INT_ERR_PARAM_IRQNO  IRQ番号不正
 *                  - MK_INT_ERR_UNAUTHORIZED 許可無し
 *
 * @return      処理結果を返す。
 * @retval      MK_INT_RET_SUCCESS 成功
 * @retval      MK_INT_RET_FAILURE 失敗（エラー番号を参照）
 */
/******************************************************************************/
int32_t MkIntWait( uint8_t  *pInt,
                   uint32_t *pErrNo )
{
    volatile MkIntParam_t param;

    /* パラメータ設定 */
    param.funcId = MK_INT_FUNCID_WAIT;
    param.errNo  = MK_INT_ERR_NONE;
    param.ret    = MK_INT_RET_FAILURE;
    param.flag   = 0;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param                    ),
                             "i" ( MK_CONFIG_INTNO_INTERRUPT )  );

    /* 割込み発生フラグ設定要否判定 */
    if ( pInt != NULL ) {
        /* 必要 */

        /* 割込み発生フラグ設定 */
        *pInt = param.flag;
    }

    /* エラー番号設定要否判定 */
    if ( pErrNo != NULL ) {
        /* 必要 */

        /* エラー番号設定 */
        *pErrNo = param.errNo;
    }

    return param.ret;
}

/******************************************************************************/
