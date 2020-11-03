/******************************************************************************/
/*                                                                            */
/* src/kernel/ItcCtrl/ItcCtrlMsg.c                                            */
/*                                                                 2020/11/03 */
/* Copyright (C) 2018-2020 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* ライブラリヘッダ */
#include <MLib/MLib.h>
#include <MLib/MLibList.h>

/* カーネルヘッダ */
#include <kernel/message.h>
#include <kernel/types.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <IntMng.h>
#include <Memmng.h>
#include <TaskMng.h>
#include <TimerMng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_ITCCTRL_MSG, \
                    __LINE__,               \
                    __VA_ARGS__             )
#else
#define DEBUG_LOG( ... )
#endif

/* 状態 */
#define STATE_INIT        ( 0 ) /**< 初期状態                 */
#define STATE_RECVWAIT    ( 1 ) /**< 受信待ち状態             */
#define STATE_RECVTIMEOUT ( 2 ) /**< 受信待ちタイムアウト状態 */
#define STATE_SENDWAIT    ( 3 ) /**< 送信待ち状態             */

/** 管理情報 */
typedef struct {
    MLibList_t list;    /**< メッセージリスト         */
    MkTaskId_t src;     /**< 受信待ちメッセージ送信元 */
    uint32_t   state;   /**< 状態                     */
    uint32_t   seqNo;   /**< シーケンス番号           */
    uint32_t   timerId; /**< タイマID                 */
} mngEntry_t;

/** メッセージ */
typedef struct {
    MLibListNode_t nodeInfo;    /**< ノード情報       */
    MkTaskId_t     src;         /**< 送信元タスクID   */
    uint32_t       seqNo;       /**< シーケンス番号   */
    size_t         size;        /**< メッセージサイズ */
    uint8_t        msg[];       /**< メッセージ       */
} msg_t;


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* 送信元タスクIDチェック */
static bool CheckSrc( MLibListNode_t *pNode,
                      void           *pArg   );
/* タスク有効チェック */
static bool CheckValid( MkTaskId_t self,
                        MkTaskId_t other,
                        MkErr_t    *pErr  );
/* メッセージ受信 */
static void DoReceive( MkMsgParam_t *pParam );
/* メッセージ送信共通処理 */
static void DoSendCmn( MkTaskId_t   *pTaskId,
                       MkMsgParam_t *pParam   );
/* メッセージ送信 */
static void DoSendNB( MkMsgParam_t *pParam );
/* 割込みハンドラ */
static void HdlInt( uint32_t        intNo,
                    IntMngContext_t context );
/* メッセージ受信待ちタイムアウト */
static void TimeoutReceive( uint32_t timerId,
                            void     *pArg    );


/******************************************************************************/
/* 静的グローバル変数定義                                                     */
/******************************************************************************/
/** 管理情報 */
static mngEntry_t gMngTbl[ MK_TASKID_NUM ];


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メッセージ制御初期化
 * @details     機能呼出し用割込みハンドラの設定を行う。
 */
/******************************************************************************/
void ItcCtrlMsgInit( void )
{
    uint32_t idx;   /* インデックス */

    /* 初期化 */
    idx = 0;

    /* 割込みハンドラ設定 */
    IntMngHdlSet( MK_CONFIG_INTNO_MESSAGE,      /* 割込み番号     */
                  HdlInt,                       /* 割込みハンドラ */
                  IA32_DESCRIPTOR_DPL_3    );   /* 特権レベル     */

    /* 管理テーブルエントリ毎に繰り返す */
    for ( idx = 0; idx < MK_TASKID_NUM; idx++ ) {
        /* 初期化 */
        MLibListInit( &( gMngTbl[ idx ].list ) );
        gMngTbl[ idx ].src     = MK_TASKID_NULL;
        gMngTbl[ idx ].state   = STATE_INIT;
        gMngTbl[ idx ].seqNo   = 0;
        gMngTbl[ idx ].timerId = TIMERMNG_TIMERID_NULL;
    }

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       送信元タスクIDチェック
 * @details     メッセージ送信元が指定したタスクIDと一致するかチェックする。
 *
 * @param[in]   *pNode ノード
 * @param[in]   *pArg  タスクID
 *
 * @return      判定結果を返す。
 * @retval      true  一致
 * @retval      false 不一致
 */
/******************************************************************************/
static bool CheckSrc( MLibListNode_t *pNode,
                      void           *pArg   )
{
    msg_t      *pMsg;   /* メッセージ */
    MkTaskId_t taskId;  /* タスクID   */

    /* 初期化 */
    pMsg   = ( msg_t * ) pNode;
    taskId = *( ( MkTaskId_t * ) pArg );

    /* タスクID比較 */
    if ( taskId == pMsg->src ) {
        /* 一致 */

        return true;
    }

    return false;
}


/******************************************************************************/
/**
 * @brief       タスク有効チェック
 * @details     相手タスクの存在チェックとタスク間のプロセス階層差チェックを行
 *              う。
 *
 * @param[in]   self  自タスクID
 * @param[in]   other 他タスクID
 * @param[out]  *pErr エラー要因
 *                  - MK_ERR_NONE         エラー無し
 *                  - MK_ERR_NO_EXIST     タスクが存在しない
 *                  - MK_ERR_UNAUTHORIZED 権限無し
 *
 * @return      チェック結果を返す。
 * @retval      true  有効
 * @retval      false 無効
 */
/******************************************************************************/
static bool CheckValid( MkTaskId_t self,
                        MkTaskId_t other,
                        MkErr_t    *pErr  )
{
    bool    exist;  /* タスク存在確認結果 */
    uint8_t diff;   /* プロセス階層差     */

    /* 初期化 */
    exist = false;
    diff  = 0;
    *pErr = MK_ERR_NONE;

    /* 他タスク存在確認 */
    exist = TaskMngTaskCheckExist( other );

    /* 確認結果判定 */
    if ( exist == false ) {
        /* 存在しない */

        /* エラー要因設定 */
        *pErr = MK_ERR_NO_EXIST;

        return false;
    }

    /* プロセス階層差取得 */
    diff = TaskMngTaskGetTypeDiff( self, other );

    /* 階層差判定 */
    if ( diff > 1 ) {
        /* 非隣接 */

        /* エラー要因設定 */
        *pErr = MK_ERR_UNAUTHORIZED;

        return false;
    }

    return true;
 }

/******************************************************************************/
/**
 * @brief           メッセージ受信
 * @details         メッセージキューからメッセージをデキューし、機能呼び出し元
 *                  にメッセージを返す。デキューしたメッセージの送信元タスクが
 *                  送信ブロッキング状態となっている場合はブロッキング状態を解
 *                  除する。メッセージが無い場合は、メッセージがキューイングさ
 *                  れるまでブロックする。タイムアウト時間が設定されている場合
 *                  はタイマを設定する。
 *
 * @param[in,out]   *pParam パラメータ
 */
/******************************************************************************/
static void DoReceive( MkMsgParam_t *pParam )
{
    bool       valid;       /* タスク有効     */
    msg_t      *pMsg;       /* メッセージ     */
    size_t     size;        /* コピーサイズ   */
    MkErr_t    err;         /* エラー要因     */
    uint32_t   tick;        /* タイムアウト値 */
    MkTaskId_t taskId;      /* タスクID       */
    mngEntry_t *pDstInfo;   /* 送信先管理情報 */

    /* 初期化 */
    valid    = false;
    pMsg     = NULL;
    size     = 0;
    err      = MK_ERR_NONE;
    tick     = 0;
    taskId   = TaskMngSchedGetTaskId();
    pDstInfo = &( gMngTbl[ taskId ] );

    /* 受信ループ */
    while ( true ) {
        /* 受信待ちタスクID判定 */
        if ( pParam->recv.src == MK_TASKID_NULL ) {
            /* ANY */

            /* メッセージデキュー */
            pMsg = ( msg_t * ) MLibListRemoveHead( &( pDstInfo->list ) );

        } else {
            /* 指定 */

            /* タスク有効チェック */
            valid = CheckValid( taskId, pParam->recv.src, &err );

            /* チェック結果判定 */
            if ( valid == false ) {
                /* 無効 */

                /* タイマ解除 */
                TimerMngCtrlUnset( pDstInfo->timerId );

                /* 管理情報初期化 */
                pDstInfo->state   = STATE_INIT;
                pDstInfo->timerId = TIMERMNG_TIMERID_NULL;

                /* 戻り値設定 */
                pParam->ret = MK_RET_FAILURE;
                pParam->err = err;

                return;
            }

            /* 指定送信元メッセージデキュー */
            pMsg = ( msg_t * )
                   MLibListSearchHead( &( pDstInfo->list ),
                                       &CheckSrc,
                                       &( pParam->recv.src ),
                                       MLIB_LIST_REMOVE       );
        }

        /* メッセージ取得結果判定 */
        if ( pMsg != NULL ) {
            /* メッセージ有 */

            /* 送信元タスク状態判定 */
            if ( ( gMngTbl[ pMsg->src ].state == STATE_SENDWAIT ) &&
                 ( gMngTbl[ pMsg->src ].seqNo == pMsg->seqNo    )    ) {
                /* 送信待ち状態 */

                /* 送信元タスクスケジュール開始 */
                TaskMngSchedStart( pMsg->src );
            }

            /* タイマ解除 */
            TimerMngCtrlUnset( pDstInfo->timerId );

            /* 管理情報初期化 */
            pDstInfo->state   = STATE_INIT;
            pDstInfo->timerId = TIMERMNG_TIMERID_NULL;

            /* コピーサイズ設定 */
            size = MLIB_MIN( pMsg->size, pParam->recv.bufferSize );

            /* 戻り値設定 */
            pParam->ret       = MK_RET_SUCCESS;
            pParam->err       = MK_ERR_NONE;
            pParam->recv.size = size;
            pParam->recv.src  = pMsg->src;

            /* メッセージコピー */
            memcpy( pParam->recv.pBuffer,
                    pMsg->msg,
                    size                  );

            /* メッセージバッファ解放 */
            memset( pMsg, 0, sizeof ( pMsg->size ) );
            MemmngHeapFree( pMsg );
            pMsg = NULL;

            return;
        }

        /* タイムアウト設定判定 */
        if ( pParam->timeout != 0 ) {
            /* タイムアウト有り */

            /* tick変換 */
            tick = pParam->timeout / ( 1000000 / MK_CONFIG_TICK_HZ );

            /* タイマ設定 */
            pDstInfo->timerId =
                TimerMngCtrlSet( tick,
                                 TIMERMNG_TYPE_ONESHOT,
                                 TimeoutReceive,
                                 pDstInfo               );

            /* タイムアウト設定初期化 */
            pParam->timeout = 0;

            /* タイマ設定結果判定 */
            if ( pDstInfo->timerId == TIMERMNG_TIMERID_NULL ) {
                /* 失敗 */

                /* 状態設定 */
                pDstInfo->state = STATE_INIT;

                /* 戻り値設定 */
                pParam->ret = MK_RET_FAILURE;
                pParam->err = MK_ERR_NO_RESOURCE;

                return;
            }
        }

        /* 状態設定 */
        pDstInfo->state = STATE_RECVWAIT;

        /* スケジュール停止 */
        TaskMngSchedStop( taskId );

        /* スケジュール実行 */
        TaskMngSchedExec();

        /* タイムアウト判定 */
        if ( pDstInfo->state == STATE_RECVTIMEOUT ) {
            /* タイムアウト */

            /* 状態設定 */
            pDstInfo->state = STATE_INIT;

            /* 戻り値設定 */
            pParam->ret = MK_RET_FAILURE;
            pParam->err = MK_ERR_TIMEOUT;

            return;
        }
    }

    return;
}


/******************************************************************************/
/**
 * @brief           メッセージ送信(ブロック)
 * @details         メッセージ送信共通処理を呼び出し、送信先タスクがメッセージ
 *                  を受け取るまでブロックする。
 *
 * @param[in,out]   *pParam パラメータ
 */
/******************************************************************************/
static void DoSend( MkMsgParam_t *pParam )
{
    MkTaskId_t taskId;  /* 送信元タスクID */

    /* 初期化 */
    taskId = MK_TASKID_NULL;

    /* 共通処理 */
    DoSendCmn( &taskId, pParam );

    /* 処理結果判定 */
    if ( pParam->ret == MK_RET_FAILURE ) {
        /* 失敗 */
        return;
    }

    /* 状態設定 */
    gMngTbl[ taskId ].state = STATE_SENDWAIT;

    /* スケジュール停止 */
    TaskMngSchedStop( taskId );

    /* スケジュール実行 */
    TaskMngSchedExec();

    /* 状態初期化 */
    gMngTbl[ taskId ].state = STATE_INIT;

    /* 戻り値設定 */
    pParam->ret = MK_RET_SUCCESS;
    pParam->err = MK_ERR_NONE;

    return;
}


/******************************************************************************/
/**
 * @brief           メッセージ送信共通処理
 * @details         送信先タスクが有効かチェックした後、メッセージをカーネル空
 *                  間内にコピーし、送信先タスクにキューイングする。送信先タス
 *                  クが受信ブロッキング状態にある場合は、ブロッキング状態を解
 *                  除する。
 *
 * @param[out]      *pTaskId 送信元タスクID
 * @param[in,out]   *pParam  パラメータ
 */
/******************************************************************************/
static void DoSendCmn( MkTaskId_t   *pTaskId,
                       MkMsgParam_t *pParam   )
{
    bool    valid;   /* タスク有効 */
    msg_t   *pMsg;   /* メッセージ */
    MkErr_t err;     /* エラー要因 */

    /* 初期化 */
    valid    = false;
    err      = MK_ERR_NONE;
    *pTaskId = TaskMngSchedGetTaskId();

    /* タスク有効チェック */
    valid = CheckValid( *pTaskId, pParam->send.dst, &err );

    /* チェック結果判定 */
    if ( valid == false ) {
        /* 無効 */

        /* 戻り値設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = err;

        return;
    }

    /* メッセージ領域割当 */
    pMsg = MemmngHeapAlloc( sizeof ( msg_t ) + pParam->send.size );

    /* 割当結果判定 */
    if ( pMsg == NULL ) {
        /* 失敗 */

        /* 戻り値設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_NO_MEMORY;

        return;
    }

    /* シーケンス番号更新 */
    ( gMngTbl[ *pTaskId ].seqNo )++;

    /* メッセージヘッダ設定 */
    pMsg->src   = *pTaskId;
    pMsg->seqNo = gMngTbl[ *pTaskId ].seqNo;
    pMsg->size  = pParam->send.size;

    /* メッセージコピー */
    memcpy( pMsg->msg, pParam->send.pMsg, pParam->send.size );

    /* キューイング */
    MLibListInsertTail( &( gMngTbl[ pParam->send.dst ].list ),
                        &( pMsg->nodeInfo                   )  );

    /* 送信先タスク状態判定 */
    if ( gMngTbl[ pParam->send.dst ].state == STATE_RECVWAIT ) {
        /* 受信待ち状態 */

        /* 受信待ちメッセージ送信元判定 */
        if ( ( gMngTbl[ pParam->send.dst ].src == *pTaskId       ) ||
             ( gMngTbl[ pParam->send.dst ].src == MK_TASKID_NULL )    ) {
            /* 自タスクIDまたはANY */

            /* 送信先タスクスケジュール開始 */
            TaskMngSchedStart( pParam->send.dst );
        }
    }

    /* 戻り値設定 */
    pParam->ret = MK_RET_SUCCESS;
    pParam->err = MK_ERR_NONE;

    return;
}


/******************************************************************************/
/**
 * @brief           メッセージ送信(ノンブロック)
 * @details         メッセージ送信共通処理を呼び出す。
 *
 * @param[in,out]   *pParam パラメータ
 */
/******************************************************************************/
static void DoSendNB( MkMsgParam_t *pParam )
{
    MkTaskId_t taskId;  /* 送信元タスクID */

    /* 初期化 */
    taskId = MK_TASKID_NULL;

    /* 共通処理 */
    DoSendCmn( &taskId, pParam );

    return;
}


/******************************************************************************/
/**
 * @brief           割込みハンドラ
 * @details         機能IDから適切なハンドラを呼び出す。
 *
 * @param[in]       intNo   割込み番号
 * @param[in,out]   context 割込み発生時コンテキスト情報
 */
/******************************************************************************/
static void HdlInt( uint32_t        intNo,
                    IntMngContext_t context )
{
    MkMsgParam_t *pParam;   /* パラメータ */

    /* 初期化 */
    pParam = ( MkMsgParam_t * ) context.genReg.esi;

    /* パラメータチェック */
    if ( pParam == NULL ) {
        /* 不正 */
        return;
    }

    /* パラメータ初期化 */
    pParam->ret       = MK_RET_FAILURE;
    pParam->err       = MK_ERR_NONE;
    pParam->recv.size = 0;

    /* 機能ID判定 */
    if ( pParam->funcId == MK_MSG_FUNCID_RECEIVE ) {
        /* メッセージ受信 */

        DoReceive( pParam );

    } else if ( pParam->funcId == MK_MSG_FUNCID_SEND ) {
        /* メッセージ送信 */

        DoSend( pParam );

    } else if ( pParam->funcId == MK_MSG_FUNCID_SEND_NB ) {
        /* メッセージ送信(ノンブロック) */

        DoSendNB( pParam );

    } else {
        /* 不正 */

        /* エラー設定 */
        pParam->ret       = MK_RET_FAILURE;
        pParam->err       = MK_ERR_PARAM;
        pParam->recv.size = 0;
    }

    return;
}


/******************************************************************************/
/**
 * @brief       メッセージ受信待ちタイムアウト
 * @details     メッセージ受信待ち合わせを解除する。
 *
 * @param[in]   timerId タイマID
 * @param[in]   *pArg   管理情報
 */
/******************************************************************************/
static void TimeoutReceive( uint32_t timerId,
                            void     *pArg    )
{
    MkTaskId_t taskId;      /* タスクID */
    mngEntry_t *pDstInfo;   /* 管理情報 */

    /* 初期化 */
    taskId   = TimerMngCtrlGetTaskId( timerId );
    pDstInfo = ( mngEntry_t * ) pArg;

    /* タイマID無効判定 */
    if ( pDstInfo->timerId != timerId ) {
        /* 無効 */

        return;
    }

    /* 状態設定 */
    pDstInfo->state = STATE_RECVTIMEOUT;

    /* タイマID初期化 */
    pDstInfo->timerId = TIMERMNG_TIMERID_NULL;

    /* スケジュール開始 */
    TaskMngSchedStart( taskId );

    /* スケジューラ実行 */
    TaskMngSchedExec();

    return;
}


/******************************************************************************/
