/******************************************************************************/
/*                                                                            */
/* src/lib/libmk/MkMsg.c                                                      */
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
#include <kernel/message.h>


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メッセージ受信
 * @details     指定したタスクからのメッセージまたは全てのタスクからのメッセー
 *              ジの受信を待ち合わせる。
 *
 * @param[in]   recvTaskId  受信待ちタスクID
 *                  - MK_TASKID_NULL     全てのタスク
 *                  - MK_TASKID_NULL以外 タスク指定
 * @param[out]  *pBuffer    メッセージバッファ
 * @param[in]   bufferSize  メッセージバッファサイズ
 * @param[out]  *pSrcTaskId 送信元タスクID
 * @param[out]  *pRecvSize  受信メッセージサイズ
 * @param[out]  *pErr       エラー内容
 *                  - MK_ERR_NONE         エラー無し
 *                  - MK_ERR_PARAM        パラメータ不正
 *                  - MK_ERR_UNAUTHORIZED 権限の無いタスク指定
 *                  - MK_ERR_NO_EXIST     存在しないタスクID指定
 *                  - MK_ERR_NO_MEMORY    メモリ不足
 *
 * @return      受信結果を返す。
 * @retval      MK_RET_SUCCESS  成功
 * @retval      MK_RET_FAILURE  失敗
 */
/******************************************************************************/
MkRet_t LibMkMsgReceive( MkTaskId_t recvTaskId,
                         void       *pBuffer,
                         size_t     bufferSize,
                         MkTaskId_t *pSrcTaskId,
                         size_t     *pRecvSize,
                         MkErr_t    *pErr        )
{
    volatile MkMsgParam_t param;

    /* 引数チェック */
    if ( ( pBuffer == NULL ) && ( bufferSize != 0 ) ) {
        /* バッファサイズ未指定でバッファサイズ有り */

        /* エラー内容設定 */
        MLIB_SET_IFNOT_NULL( pErr, MK_ERR_PARAM );

        return MK_RET_FAILURE;
    }

    /* パラメータ設定 */
    param.funcId          = MK_MSG_FUNCID_RECEIVE;
    param.ret             = MK_RET_FAILURE;
    param.err             = MK_ERR_NONE;
    param.recv.src        = recvTaskId;
    param.recv.pBuffer    = pBuffer;
    param.recv.bufferSize = bufferSize;
    param.recv.recvSize   = 0;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param       ),
                             "i" ( MK_MSG_INTNO )
                           : "esi"                 );

    /* エラー内容設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    /* 送信元タスクID設定 */
    MLIB_SET_IFNOT_NULL( pSrcTaskId, param.recv.src );

    /* 受信メッセージサイズ */
    MLIB_SET_IFNOT_NULL( pRecvSize, param.recv.recvSize );

    return param.ret;
}


/******************************************************************************/
/**
 * @brief       メッセージ送信
 * @details     指定したタスクにメッセージを送信する。送信先タスクがメッセージ
 *              を受信するまで待ち合わせる。
 *
 * @param[in]   dst   送信先タスク
 * @param[in]   *pMsg メッセージ
 * @param[in]   size  サイズ
 * @param[out]  *pErr エラー内容
 *                  - MK_ERR_NONE         エラー無し
 *                  - MK_ERR_PARAM        パラメータ不正
 *                  - MK_ERR_UNAUTHORIZED 非隣接プロセスタイプ
 *                  - MK_ERR_SIZE_OVER    送信サイズ超過
 *                  - MK_ERR_NO_EXIST     存在しないタスクID指定
 *                  - MK_ERR_NO_MEMORY    メモリ不足
 *
 * @return      処理結果を返す。
 * @retval      MK_RET_SUCCESS 成功
 * @retval      MK_RET_FAILURE 失敗
 */
/******************************************************************************/
MkRet_t LibMkMsgSend( MkTaskId_t dst,
                      void       *pMsg,
                      size_t     size,
                      MkErr_t    *pErr   )
{
    volatile MkMsgParam_t param;

    /* 引数チェック */
    if ( ( pMsg == NULL ) || ( size == 0 ) ) {
        /* 不正 */

        /* エラー内容設定 */
        MLIB_SET_IFNOT_NULL( pErr, MK_ERR_PARAM );

        return MK_RET_FAILURE;
    }

    /* パラメータ設定 */
    param.funcId    = MK_MSG_FUNCID_SEND;
    param.ret       = MK_RET_FAILURE;
    param.err       = MK_ERR_NONE;
    param.send.dst  = dst;
    param.send.pMsg = pMsg;
    param.send.size = size;

    /* カーネルコール */
    __asm__ __volatile__ ( "mov esi, %0\n"
                           "int %1"
                           :
                           : "a" ( &param       ),
                             "i" ( MK_MSG_INTNO )
                           : "esi"                 );

    /* エラー内容設定 */
    MLIB_SET_IFNOT_NULL( pErr, param.err );

    return param.ret;
}


/******************************************************************************/
