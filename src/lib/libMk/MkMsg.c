/******************************************************************************/
/*                                                                            */
/* src/lib/libMk/MkMsg.c                                                      */
/*                                                                 2019/07/03 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdint.h>

/* 外部モジュールヘッダ */
#include <kernel/message.h>

/* 内部モジュールヘッダ */


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メッセージ受信
 * @details     指定したタスクからのメッセージまたは全てのタスクからのメッセー
 *              ジの受信を待ち合わせる。
 *
 * @param[in]   rcvTaskId   受信待ちタスクID
 *                  - MK_TASKID_NULL     全てのタスク
 *                  - MK_TASKID_NULL以外 タスク指定
 * @param[out]  *pBuffer    メッセージバッファ
 * @param[in]   size        メッセージバッファサイズ
 * @param[out]  *pSrcTaskId 送信元タスクID
 * @param[out]  *pErrNo     エラー番号
 *                  - MK_MSG_ERR_NONE      エラー無し
 *                  - MK_MSG_ERR_NO_EXIST  存在しないタスクID指定
 *                  - MK_MSG_ERR_PROC_TYPE 非隣接プロセスタイプ
 *                  - MK_MSG_ERR_NO_MEMORY メモリ不足
 *
 * @return      受信したメッセージサイズを返す。
 * @retval      MK_MSG_RET_FAILURE     失敗
 * @retval      MK_MSG_RET_FAILURE以外 受信メッセージサイズ
 */
/******************************************************************************/
int32_t MkMsgReceive( MkTaskId_t rcvTaskId,
                      void       *pBuffer,
                      size_t     size,
                      MkTaskId_t *pSrcTaskId,
                      uint32_t   *pErrNo      )
{
    volatile MkMsgParam_t param;

    /* パラメータ設定 */
    param.funcId      = MK_MSG_FUNCID_RECEIVE;
    param.errNo       = MK_MSG_ERR_NONE;
    param.ret         = MK_MSG_RET_FAILURE;
    param.rcv.src     = rcvTaskId;
    param.rcv.pBuffer = pBuffer;
    param.rcv.size    = size;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param                  ),
                             "i" ( MK_CONFIG_INTNO_MESSAGE )
                           : "esi"                            );

    /* エラー番号設定要否判定 */
    if ( pErrNo != NULL ) {
        /* 必要 */

        /* エラー番号設定 */
        *pErrNo = param.errNo;
    }

    /* 送信元タスクID設定要否判定 */
    if ( pSrcTaskId != NULL ) {
        /* 必要 */

        *pSrcTaskId = param.rcv.src;
    }

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       メッセージ送信
 * @details     指定したタスクにメッセージを送信する。送信先タスクがメッセージ
 *              を受信するまで待ち合わせる。
 *
 * @param[in]   dst   送信元タスク
 * @param[in]   *pMsg メッセージ
 * @param[in]   size  サイズ
 * @param[out]  *pErrNo  エラー番号
 *                  - MK_MSG_ERR_NONE      エラー無し
 *                  - MK_MSG_ERR_SIZE_OVER 送信サイズ超過
 *                  - MK_MSG_ERR_NO_EXIST  存在しないタスクID指定
 *                  - MK_MSG_ERR_PROC_TYPE 非隣接プロセスタイプ
 *                  - MK_MSG_ERR_NO_MEMORY メモリ不足
 *
 * @return      処理結果を返す。
 * @retval      MK_MSG_RET_SUCCESS 成功
 * @retval      MK_MSG_RET_FAILURE 失敗
 */
/******************************************************************************/
int32_t MkMsgSend( MkTaskId_t dst,
                   void       *pMsg,
                   size_t     size,
                   uint32_t   *pErrNo )
{
    volatile MkMsgParam_t param;

    /* パラメータ設定 */
    param.funcId   = MK_MSG_FUNCID_SEND;
    param.errNo    = MK_MSG_ERR_NONE;
    param.ret      = MK_MSG_RET_FAILURE;
    param.snd.dst  = dst;
    param.snd.pMsg = pMsg;
    param.rcv.size = size;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param                  ),
                             "i" ( MK_CONFIG_INTNO_MESSAGE )
                           : "esi"                            );

    /* エラー番号設定要否判定 */
    if ( pErrNo != NULL ) {
        /* 必要 */

        /* エラー番号設定 */
        *pErrNo = param.errNo;
    }

    return param.ret;
}


/******************************************************************************/
