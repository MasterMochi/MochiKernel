/******************************************************************************/
/* src/kernel/ItcCtrl/ItcCtrlMsg.c                                            */
/*                                                                 2018/05/25 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <hardware/IA32/IA32.h>
#include <kernel/message.h>
#include <MLib/Basic/MLibBasic.h>
#include <MLib/Basic/MLibBasicList.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <IntMng.h>
#include <TaskMng.h>

/* 内部モジュールヘッダ */


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                        \
    DebugLogOutput( CMN_MODULE_ITCCTRL_MSG,     \
                    __LINE__,                   \
                    __VA_ARGS__ )
#else
#define DEBUG_LOG( ... )
#endif

/* メッセージパッシング状態 */
#define STATE_INIT    ( 0 )     /**< 初期状態     */
#define STATE_SNDWAIT ( 1 )     /**< 送信待ち状態 */
#define STATE_RCVWAIT ( 2 )     /**< 受信待ち状態 */

/** メッセージバッファ数 */
#define MSG_BUFFER_NUM ( 1024 )

/** メッセージバッファ */
typedef struct {
    MLibBasicListNode_t node;                   /**< 連結リスト情報   */
    size_t              size;                   /**< メッセージサイズ */
    uint8_t             msg[ MK_MSG_SIZE_MAX ]; /**< メッセージ       */
} MsgBuffer_t;

/** 管理情報型 */
typedef struct {
    uint32_t    state;          /**< メッセージパッシング状態           */
    MsgBuffer_t *pBuffer;       /**< メッセージバッファポインタ         */
    MkTaskId_t  head;           /**< 送信待ちリンクリスト（先頭リンク） */
    MkTaskId_t  next;           /**< 送信待ちリンクリスト（後続リンク） */
    MkTaskId_t  src;            /**< 受信待ちメッセージ送信元           */
} MngTbl_t;


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* 送信待ちリンクリスト追加 */
static void AddSndWaitList( MkTaskId_t src,
                            MkTaskId_t dst  );
/* 割込みハンドラ */
static void HdlInt( uint32_t        intNo,
                    IntMngContext_t context );
/* メッセージ受信 */
static void Receive( MkMsgParam_t *pParam );
/* 送信待ちリンクリスト先頭エントリ削除 */
static MkTaskId_t RemoveSndWaitListTop( MkTaskId_t taskId );
/* 送信待ちリンクリスト指定エントリ削除 */
static MkTaskId_t RemoveSndWaitList( MkTaskId_t taskId,
                                     MkTaskId_t rmvTaskId );
/* メッセージ送信 */
static void Send( MkMsgParam_t *pParam );


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** 管理情報 */
static MngTbl_t gMngTbl[ MK_CONFIG_TASKID_NUM ];

/** メッセージバッファ */
static MsgBuffer_t gMsgBuffer[ MSG_BUFFER_NUM ];

/** 未使用メッセージバッファリスト */
MLibBasicList_t gUnusedBufferList;


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メッセージ制御初期化
 * @details     メッセージ制御サブモジュールの初期化を行う。
 */
/******************************************************************************/
void ItcCtrlMsgInit( void )
{
    int        i;       /* インデックス */
    MkTaskId_t taskId;  /* タスクID     */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* 初期化 */
    memset( &gUnusedBufferList, 0, sizeof ( MLibBasicList_t ) );
    memset( gMsgBuffer,         0, sizeof ( MsgBuffer_t     ) );
    
    /* 割込みハンドラ設定 */
    IntMngHdlSet( MK_CONFIG_INTNO_MSGPASSING,   /* 割込み番号     */
                  HdlInt,                       /* 割込みハンドラ */
                  IA32_DESCRIPTOR_DPL_3    );   /* 特権レベル     */
    
    /* 管理情報初期化 */
    for ( taskId = MK_CONFIG_TASKID_MIN;
          taskId < MK_CONFIG_TASKID_NUM;
          taskId++                       ) {
        gMngTbl[ taskId ].state   = STATE_INIT;
        gMngTbl[ taskId ].pBuffer = NULL;
        gMngTbl[ taskId ].head    = MK_CONFIG_TASKID_NULL;
        gMngTbl[ taskId ].next    = MK_CONFIG_TASKID_NULL;
        gMngTbl[ taskId ].src     = MK_CONFIG_TASKID_NULL;
    }
    
    /* メッセージバッファリスト初期化 */
    for ( i = 0; i < MSG_BUFFER_NUM; i++ ) {
        /* 未使用メッセージバッファリスト挿入 */
        MLibBasicListInsertHead( &gUnusedBufferList,
                                 &gMsgBuffer[ i ].node );
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       送信待ちリンクリスト追加
 * @details     送信先タスクIDの送信待ちリンクリストに送信元タスクIDを追加する。
 * 
 * @param[in]   src 送信元タスクID
 * @param[in]   dst 送信先タスクID
 */
/******************************************************************************/
static void AddSndWaitList( MkTaskId_t src,
                            MkTaskId_t dst  )
{
    MkTaskId_t next;    /* 後続リンク先タスクID       */
    MkTaskId_t *pTail;  /* 最後尾リンク格納先タスクID */
    
    /* 初期化 */
    pTail = &gMngTbl[ dst ].head;
    next  = *pTail;
    
    /* 後続リンクチェック */
    while ( next != MK_CONFIG_TASKID_NULL ) {
        /* 後続リンク有 */
        
        /* 後続リンク取得 */
        pTail = &gMngTbl[ next ].next;
        next  = *pTail;
    }
    
    /* リンクリスト追加 */
    *pTail = src;
    
    return;
}


/******************************************************************************/
/**
 * @brief       割込みハンドラ
 * @details     機能IDから該当する機能を呼び出す。
 * 
 * @param[in]   intNo   割込み番号
 * @param[in]   context 割込み発生時コンテキスト情報
 */
/******************************************************************************/
static void HdlInt( uint32_t        intNo,
                    IntMngContext_t context )
{
    MkMsgParam_t *pParam;
    
    /* 初期化 */
    pParam = ( MkMsgParam_t * ) context.genReg.esi;
    
    /* パラメータチェック */
    if ( pParam == NULL ) {
        /* 不正 */
        
        return;
    }
    
    /* 初期化 */
    pParam->ret   = MK_MSG_RET_FAILURE;     /* 戻り値     */
    pParam->errNo = MK_MSG_ERR_NONE;        /* エラー番号 */
    
    /* 機能ID判定 */
    if ( pParam->funcId == MK_MSG_FUNCID_RECEIVE ) {
        /* メッセージ受信 */
        Receive( pParam );
        
    } else if ( pParam->funcId == MK_MSG_FUNCID_SEND ) {
        /* メッセージ送信 */
        Send( pParam );
        
    } else {
        /* 不正 */
        
        /* エラー設定 */
        pParam->ret   = MK_MSG_RET_FAILURE;         /* 戻り値     */
        pParam->errNo = MK_MSG_ERR_PARAM_FUNCID;    /* エラー番号 */
    }
    
    return;
}


/******************************************************************************/
/**
 * @brief       メッセージ受信
 * @details     メッセージ受信を行う。
 * 
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void Receive( MkMsgParam_t *pParam )
{
    bool       exist;   /* タスク存在確認結果 */
    size_t     size;    /* 受信サイズ         */
    uint8_t    diff;    /* プロセス階層差     */
    MkTaskId_t taskId;  /* 受信タスクID       */
    MkTaskId_t src;     /* 送信元タスクID     */
    
    DEBUG_LOG( "%s() start. pParam=%p", __func__, pParam );
    DEBUG_LOG( " pParam->rcv.src    =%u", pParam->rcv.src     );
    DEBUG_LOG( " pParam->rcv.pBuffer=%p", pParam->rcv.pBuffer );
    DEBUG_LOG( " pParam->rcv.size   =%u", pParam->rcv.size    );
    
    /* 初期化 */
    size = 0;
    
    /* タスクID取得 */
    taskId = TaskMngSchedGetTaskId();
    
    /* 受信ループ */
    while ( true ) {
        /* 受信待ち送信元タスクID判定 */
        if ( pParam->rcv.src == MK_CONFIG_TASKID_NULL ) {
            /* ANY */
            
            /* 送信待ちリンクリストから先頭エントリ取得 */
            src = RemoveSndWaitListTop( taskId );
            
        } else {
            /* 指定 */
            
            /* タスク存在確認 */
            exist = TaskMngTaskCheckExist( pParam->rcv.src );
            
            /* 確認結果判定 */
            if ( exist == false ) {
                /* 存在しない */
                
                /* エラー設定 */
                pParam->ret   = MK_MSG_RET_FAILURE;     /* 戻り値     */
                pParam->errNo = MK_MSG_ERR_NO_EXIST;    /* エラー番号 */
                
                DEBUG_LOG( "%s() end.", __func__ );
                
                return;
            }
            
            /* プロセス階層差取得 */
            diff = TaskMngTaskGetTypeDiff( taskId, pParam->rcv.src );
            
            /* 階層差チェック */
            if ( diff > 1 ) {
                /* 非隣接階層 */
                
                /* エラー設定 */
                pParam->ret   = MK_MSG_RET_FAILURE;     /* 戻り値     */
                pParam->errNo = MK_MSG_ERR_PROC_TYPE;   /* エラー番号 */
                
                DEBUG_LOG( "%s() end.", __func__ );
                
                return;
            }
            
            /* 送信待ちリンクリストから指定タスクIDのエントリ取得 */
            src = RemoveSndWaitList( taskId, pParam->rcv.src );
        }
        
        /* エントリ取得結果判定 */
        if ( src != MK_CONFIG_TASKID_NULL ) {
            /* 該当エントリ有 */
            
            /* コピーサイズ設定 */
            size = MLIB_BASIC_MIN( gMngTbl[ src ].pBuffer->size,
                                   pParam->rcv.size              );
            
            /* メッセージコピー */
            memcpy( pParam->rcv.pBuffer,
                    gMngTbl[ src ].pBuffer->msg,
                    size                         );
            
            /* メッセージバッファ解放 */
            memset( gMngTbl[ src ].pBuffer, 0, sizeof ( MsgBuffer_t ) );
            MLibBasicListInsertHead( &gUnusedBufferList,
                                     &gMngTbl[ src ].pBuffer->node );
            gMngTbl[ src ].pBuffer = NULL;
            
            /* 送信元タスクスケジュール開始 */
            TaskMngSchedStart( src );
            
            /* 状態設定 */
            gMngTbl[ taskId ].state = STATE_INIT;
            
            /* 戻り値設定 */
            pParam->ret = size;
            
            break;
        }
        
        /* 状態設定 */
        gMngTbl[ taskId ].state = STATE_RCVWAIT;
        
        /* スケジュール停止 */
        TaskMngSchedStop( taskId );
        
        /* スケジュール実行 */
        TaskMngSchedExec();
    }
    
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
/**
 * @breif       送信待ちリンクリスト先頭エントリ削除
 * @details     指定したタスクIDの送信待ちリンクリストから先頭のタスクIDを削除
 *              し、削除したタスクIDを返却する。
 * 
 * @param[in]   taskId タスクID
 * 
 * @return      先頭エントリのタスクID
 * @retval      MK_CONFIG_TASKID_NULL 先頭エントリ無し
 * @retval      MK_CONFIG_TASKID_MIN  タスクID最小値
 * @retval      MK_CONFIG_TASKID_MAX  タスクID最大値
 */
/******************************************************************************/
static MkTaskId_t RemoveSndWaitListTop( MkTaskId_t taskId )
{
    MkTaskId_t headTaskId;  /* 先頭エントリタスクID */
    
    /* 先頭エントリタスクID取得 */
    headTaskId = gMngTbl[ taskId ].head;
    
    /* 先頭タスク有無判定 */
    if ( headTaskId != MK_CONFIG_TASKID_NULL ) {
        /* タスク有り */
        
        /* 先頭タスク削除 */
        gMngTbl[ taskId     ].head = gMngTbl[ headTaskId ].next;
        gMngTbl[ headTaskId ].next = MK_CONFIG_TASKID_NULL;
    }
    
    return headTaskId;
}


/******************************************************************************/
/**
 * @brief       送信待ちリンクリスト指定エントリ削除
 * @details     指定したタスクIDの送信待ちリンクリストから指定したタスクIDのエ
 *              ントリを削除し、削除したタスクIDを返却する。
 * 
 * @param[in]   taskId    タスクID
 * @param[in]   rmvTaskId 削除タスクID
 * 
 * @return      削除タスクID
 * @retval      MK_CONFIG_TASKID_NULL 該当エントリ無し
 * @retval      引数rmvTaskId         削除エントリのタスクID
 */
/******************************************************************************/
static MkTaskId_t RemoveSndWaitList( MkTaskId_t taskId,
                                     MkTaskId_t rmvTaskId )
{
    MkTaskId_t *pTail;      /* 最後尾リンク格納先 */
    MkTaskId_t waitTaskId;  /* 送信待ちタスクID   */
    
    /* 初期化 */
    pTail      = &gMngTbl[ taskId ].head;
    waitTaskId = *pTail;
    
    /* 削除タスクID検索 */
    while ( waitTaskId != MK_CONFIG_TASKID_NULL ) {
        if ( waitTaskId == rmvTaskId ) {
            /* 該当タスクID */
            
            /* 該当タスク削除 */
            *pTail                    = gMngTbl[ rmvTaskId ].next;
            gMngTbl[ rmvTaskId ].next = MK_CONFIG_TASKID_NULL;
            
            break;
        }
        
        /* 後続リンク取得 */
        pTail      = &gMngTbl[ waitTaskId ].next;
        waitTaskId = *pTail;
    }
    
    return waitTaskId;
}


/******************************************************************************/
/**
 * @brief       メッセージ送信
 * @details     メッセージ送信を行う。
 * 
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void Send( MkMsgParam_t *pParam )
{
    bool       exist;   /* タスク存在確認結果 */
    uint8_t    diff;    /* プロセス階層差     */
    MkTaskId_t taskId;  /* タスクID           */
    
    DEBUG_LOG( "%s() start. pParam=%p", __func__, pParam );
    DEBUG_LOG( " pParam->snd.dst =%u", pParam->snd.dst  );
    DEBUG_LOG( " pParam->snd.pMsg=%p", pParam->snd.pMsg );
    DEBUG_LOG( " pParam->snd.size=%u", pParam->snd.size );
    
    /* 送信サイズチェック */
    if ( pParam->snd.size > MK_MSG_SIZE_MAX ) {
        /* 超過 */
        
        /* エラー設定 */
        pParam->ret   = MK_MSG_RET_FAILURE;     /* 戻り値     */
        pParam->errNo = MK_MSG_ERR_SIZE_OVER;   /* エラー番号 */
        
        DEBUG_LOG( "%s() end.", __func__ );
        
        return;
    }
    
    /* タスクID取得 */
    taskId = TaskMngSchedGetTaskId();
    
    /* タスク存在確認 */
    exist = TaskMngTaskCheckExist( pParam->snd.dst );
    
    /* 確認結果判定 */
    if ( exist == false ) {
        /* 存在しない */
        
        /* エラー設定 */
        pParam->ret   = MK_MSG_RET_FAILURE;     /* 戻り値     */
        pParam->errNo = MK_MSG_ERR_NO_EXIST;    /* エラー番号 */
        
        DEBUG_LOG( "%s() end.", __func__ );
        
        return;
    }
    
    /* プロセス階層差取得 */
    diff = TaskMngTaskGetTypeDiff( taskId, pParam->snd.dst );
    
    /* 階層差チェック */
    if ( diff > 1 ) {
        /* 非隣接階層 */
        
        /* エラー設定 */
        pParam->ret   = MK_MSG_RET_FAILURE;     /* 戻り値     */
        pParam->errNo = MK_MSG_ERR_PROC_TYPE;   /* エラー番号 */
        
        DEBUG_LOG( "%s() end.", __func__ );
        
        return;
    }
    
    /* メッセージバッファ割当 */
    gMngTbl[ taskId ].pBuffer =
        ( MsgBuffer_t * ) MLibBasicListRemoveHead( &gUnusedBufferList );
    
    /* メッセージ割当結果判定 */
    if ( gMngTbl[ taskId ].pBuffer == NULL ) {
        /* 失敗 */
        
        /* エラー設定 */
        pParam->ret   = MK_MSG_RET_FAILURE;     /* 戻り値     */
        pParam->errNo = MK_MSG_ERR_NO_MEMORY;   /* エラー番号 */
        
        DEBUG_LOG( "%s() end.", __func__ );
        
        return;
    }
    
    /* メッセージコピー */
    gMngTbl[ taskId ].pBuffer->size = pParam->snd.size;
    memcpy( gMngTbl[ taskId ].pBuffer->msg,
            pParam->snd.pMsg,
            gMngTbl[ taskId ].pBuffer->size );
    
    /* 送信待ちリスト追加 */
    AddSndWaitList( taskId, pParam->snd.dst );
    
    /* 送信先タスク状態判定 */
    if ( gMngTbl[ pParam->snd.dst ].state == STATE_RCVWAIT ) {
        /* 受信待ち状態 */
        
        /* 受信待ちメッセージ送信元判定 */
        if ( ( gMngTbl[ pParam->snd.dst ].src == taskId                ) ||
             ( gMngTbl[ pParam->snd.dst ].src == MK_CONFIG_TASKID_NULL )    ) {
            /* 自タスクIDまたはANY */
            
            /* 送信先タスクスケジュール開始 */
            TaskMngSchedStart( pParam->snd.dst );
        }
    }
    
    /* 状態設定 */
    gMngTbl[ taskId ].state = STATE_SNDWAIT;
    
    /* スケジュール停止 */
    TaskMngSchedStop( taskId );
    
    /* スケジュール実行 */
    TaskMngSchedExec();
    
    /* 初期状態設定 */
    gMngTbl[ taskId ].state = STATE_INIT;
    
    /* 戻り値設定 */
    pParam->ret = MK_MSG_RET_SUCCESS;
    
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}

/******************************************************************************/
