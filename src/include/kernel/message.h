/******************************************************************************/
/* src/include/kernel/message.h                                               */
/*                                                                 2018/05/06 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
#ifndef _MK_MESSAGE_H_
#define _MK_MESSAGE_H_
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stddef.h>
#include <stdint.h>
#include <kernel/config.h>
#include <kernel/types.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** メッセージサイズ最大 */
#define MK_MSG_SIZE_MAX         ( 4096 )

/* 機能ID */
#define MK_MSG_FUNCID_RECEIVE   ( 0x00000001 )  /** メッセージ受信 */
#define MK_MSG_FUNCID_SEND      ( 0x00000002 )  /** メッセージ送信 */

/* エラー番号 */
#define MK_MSG_ERR_NONE         ( 0x00000000 )  /** エラー無し           */
#define MK_MSG_ERR_PARAM_FUNCID ( 0x00000001 )  /** 機能ID不正           */
#define MK_MSG_ERR_SIZE_OVER    ( 0x00000002 )  /** 送信サイズ超過       */
#define MK_MSG_ERR_NO_EXIST     ( 0x00000003 )  /** タスクが存在しない   */
#define MK_MSG_ERR_PROC_TYPE    ( 0x00000004 )  /** 非隣接プロセスタイプ */
#define MK_MSG_ERR_NO_MEMORY    ( 0x00000005 )  /** メモリ不足           */

/* 戻り値 */
#define MK_MSG_RET_FAILURE      ( -1 )          /** 失敗 */
#define MK_MSG_RET_SUCCESS      (  0 )          /** 成功 */


/** メッセージ受信パラメータ */
typedef struct {
    MkTaskId_t src;             /**< 受信メッセージ送信元タスクID */
    void       *pBuffer;        /**< 受信メッセージ格納先         */
    size_t     size;            /**< 受信メッセージサイズ         */
} MkMsgParamRcv_t;

/** メッセージ送信パラメータ */
typedef struct {
    MkTaskId_t dst;             /**< 送信先タスクID       */
    void       *pMsg;           /**< 送信メッセージ格納先 */
    size_t     size;            /**< 送信メッセージサイズ */
} MkMsgParamSnd_t;

/** メッセージパッシングパラメータ */
typedef struct {
    uint32_t funcId;            /**< 機能ID                   */
    uint32_t errNo;             /**< エラー番号               */
    int32_t  ret;               /**< 戻り値                   */
    union {                     /*----------------------------*/
        MkMsgParamRcv_t rcv;    /**< メッセージ受信パラメータ */
        MkMsgParamSnd_t snd;    /**< メッセージ送信パラメータ */
    };                          /*----------------------------*/
} MkMsgParam_t;



/******************************************************************************/
#endif
