/******************************************************************************/
/* src/kernel/TimerMng/TimerMngCtrl.c                                         */
/*                                                                 2018/11/24 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <kernel/config.h>
#include <kernel/timer.h>
#include <MLib/Basic/MLibBasicList.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <IntMng.h>
#include <TaskMng.h>
#include <TimerMng.h>

/* 内部モジュールヘッダ */


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                        \
    DebugLogOutput( CMN_MODULE_TIMERMNG_CTRL,   \
                    __LINE__,                   \
                    __VA_ARGS__ )
#else
#define DEBUG_LOG( ... )
#endif

/** タイマ情報型 */
typedef struct {
    MLibBasicListNode_t listInfo;   /**< リンクリスト情報     */
    uint32_t            timerId;    /**< タイマID             */
    uint32_t            remain;     /**< 残タイマ値           */
    uint32_t            tick;       /**< 設定タイマ値         */
    uint32_t            type;       /**< タイマ種別           */
    TimerMngFunc_t      pFunc;      /**< コールバック関数     */
    void                *pArg;      /**< コールバック関数引数 */
    MkTaskId_t          taskId;     /**< タスクID             */
} TimerInfo_t;


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* 割込みハンドラ */
static void HdlInt( uint32_t        intNo,
                    IntMngContext_t context );

/* 使用中タイマ情報リスト設定 */
static void Set( TimerInfo_t *pTimerInfo );

/* スリープ */
static void Sleep( MkTimerParam_t *pParam );

/* スリープタイムアウト */
static void SleepTimeout( uint32_t timerId,
                          void     *pArg    );

/* 未使用タイマ情報リスト設定 */
static void Unset( TimerInfo_t *pTimerInfo );


/******************************************************************************/
/* グローバル変数宣言                                                         */
/******************************************************************************/
/** 未使用タイマ情報リスト */
static MLibBasicList_t gUnusedList;

/** 使用中タイマ情報リスト */
static MLibBasicList_t gUsedList;

/** タイマ情報テーブル */
static TimerInfo_t gTimerInfoTbl[ TIMERMNG_TIMERID_NUM ];


/******************************************************************************/
/* モジュール外向けグローバル関数定義                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       タイマ設定
 * @details     指定したタイマ値でタイマを設定する。
 * 
 * @param[in]   tick  タイマ値
 * @param[in]   type  タイマ種別
 *                  - TIMERMNG_TYPE_ONESHOT ワンショットタイマ
 *                  - TIMERMNG_TYPE_REPEAT  繰り返しタイマ
 * @param[in]   pFunc コールバック関数
 * @param[in]   *pArg コールバック関数引数
 * 
 * @return      タイマ登録結果を返す。
 * @retval      TIMERMNG_TIMERID_NULL     タイマ設定失敗
 * @retval      TIMERMNG_TIMERID_NULL以外 タイマ設定成功(タイマID)
 */
/******************************************************************************/
uint32_t TimerMngCtrlSet( uint32_t       tick,
                          uint32_t       type,
                          TimerMngFunc_t pFunc,
                          void           *pArg  )
{
    TimerInfo_t *pTimerInfo;
    
    /* タイマ種別チェック */
    if ( ( type != TIMERMNG_TYPE_ONESHOT ) &&
         ( type != TIMERMNG_TYPE_REPEAT  )    ) {
        /* 不正 */
        
        return TIMERMNG_TIMERID_NULL;
    }
    
    /* コールバック関数チェック */
    if ( pFunc == NULL ) {
        /* 不正 */
        
        return TIMERMNG_TIMERID_NULL;
    }
    
    /* 未使用タイマ情報取得 */
    pTimerInfo = ( TimerInfo_t * ) MLibBasicListRemoveTail( &gUnusedList );
    
    /* 取得結果判定 */
    if ( pTimerInfo == NULL ) {
        /* 失敗 */
        
        return TIMERMNG_TIMERID_NULL;
    }
    
    /* タイマ情報設定 */
    pTimerInfo->remain = tick;
    pTimerInfo->tick   = tick;
    pTimerInfo->type   = type;
    pTimerInfo->pFunc  = pFunc;
    pTimerInfo->pArg   = pArg;
    pTimerInfo->taskId = TaskMngSchedGetTaskId();
    
    /* 使用中タイマ情報リスト設定 */
    Set( pTimerInfo );
    
    return pTimerInfo->timerId;
}


/******************************************************************************/
/**
 * @brief       タイマ解除
 * @details     指定したタイマIDのタイマ設定を解除する。
 */
/******************************************************************************/
void TimerMngCtrlUnset( uint32_t timerId )
{
    TimerInfo_t *pNext;
    TimerInfo_t *pTimerInfo;
    
    /* タイマIDチェック */
    if ( timerId > TIMERMNG_TIMERID_MAX ) {
        /* 不正 */
        
        return;
    }
    
    pTimerInfo = &gTimerInfoTbl[ timerId ];
    
    /* タイマ情報使用中チェック */
    if ( pTimerInfo->pFunc == NULL ) {
        /* 未使用 */
        
        return;
    }
    
    /* 次タイマ情報取得 */
    pNext = ( TimerInfo_t * )
        MLibBasicListGetNextNode( &gUsedList,
                                  ( MLibBasicListNode_t * ) pTimerInfo );
    
    /* 取得結果判定 */
    if ( pNext != NULL ) {
        /* 次タイマ情報有り */
        
        /* 次タイマ情報設定 */
        pNext->remain += pTimerInfo->remain;
    }
    
    /* 使用中タイマ情報リストから削除 */
    ( void ) MLibBasicListRemove( &gUsedList,
                                  ( MLibBasicListNode_t * ) pTimerInfo );
    
    /* 未使用タイマ情報リスト設定 */
    Unset( pTimerInfo );
    
    return;
}


/******************************************************************************/
/* モジュール内向けグローバル関数                                             */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       タイマ制御初期化
 * @details     管理情報の初期化と割込みハンドラの設定を行う。
 */
/******************************************************************************/
void CtrlInit( void )
{
    uint32_t index;
    
    /* タイマ情報リスト初期化 */
    ( void ) MLibBasicListInit( &gUnusedList );
    ( void ) MLibBasicListInit( &gUsedList   );
    
    /* タイマ情報テーブルエントリ毎に繰り返し */
    for ( index  = TIMERMNG_TIMERID_MIN;
          index <= TIMERMNG_TIMERID_MAX;
          index++                        ) {
        
        /* 初期化 */
        gTimerInfoTbl[ index ].timerId = index;
        
        /* 未使用タイマ情報リスト設定 */
        Unset( &gTimerInfoTbl[ index ] );
    }
    
    /* 割込みハンドラ設定 */
    IntMngHdlSet( MK_CONFIG_INTNO_TIMER,        /* 割込み番号     */
                  HdlInt,                       /* 割込みハンドラ */
                  IA32_DESCRIPTOR_DPL_3  );     /* 特権レベル     */

}


/******************************************************************************/
/**
 * @brief       タイマ制御実行
 * @details     使用中タイマ情報リストの先頭エントリを取り出し、残タイマ値をデ
 *              クリメントする。残タイマ値が0になった場合はタイムアウト処理を行
 *              う。
 */
/******************************************************************************/
void CtrlRun( void )
{
    TimerInfo_t *pTimerInfo; /* タイマ情報 */
    
    /* 先頭エントリ取得 */
    pTimerInfo = ( TimerInfo_t * ) MLibBasicListGetNextNode( &gUsedList, NULL );
    
    /* 取得結果判定 */
    if ( pTimerInfo == NULL ) {
        /* エントリ無し */
        
        return;
    }
    
    /* タイムアウト判定 */
    if ( pTimerInfo->remain != 0 ) {
        /* タイムアウトでない */
        
        /* 残タイマ値減算 */
        pTimerInfo->remain--;
        
        return;
    }
    
    /* 使用中タイマ情報リストから削除 */
    ( void ) MLibBasicListRemoveHead( &gUsedList );
    
    /* タイマ種別判定 */
    if ( pTimerInfo->type == TIMERMNG_TYPE_ONESHOT ) {
        /* ワンショットタイマ */
        
        /* コールバック関数呼出し */
        ( pTimerInfo->pFunc )( pTimerInfo->timerId, pTimerInfo->pArg );
        
        /* 未使用タイマ情報リスト設定 */
        Unset( pTimerInfo );
        
    } else if ( pTimerInfo->type == TIMERMNG_TYPE_REPEAT ) {
        /* 繰り返しタイマ */
        
        /* 残タイマ値設定 */
        pTimerInfo->remain = pTimerInfo->tick;
        
        /* 使用中タイマ情報リスト設定 */
        Set( pTimerInfo );
        
        /* コールバック関数呼出し */
        ( pTimerInfo->pFunc )( pTimerInfo->timerId, pTimerInfo->pArg );
    }
    
    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       割込みハンドラ
 * @details     機能IDから該当する機能を呼び出す。
 * 
 * @param[in]   intNo   割込み番号
 * @param[in]   context 割込み発生時コンテキスト
 */
/******************************************************************************/
static void HdlInt( uint32_t        intNo,
                    IntMngContext_t context )
{
    MkTimerParam_t *pParam;    /* パラメータ   */

    /* 初期化 */
    pParam = ( MkTimerParam_t * ) context.genReg.esi;
    
    /* パラメータチェック */
    if ( pParam == NULL ) {
        /* 不正 */
        
        return;
    }
    
    /* 機能ID判定 */
    switch ( pParam->funcId ) {
        case MK_TIMER_FUNCID_SLEEP:
            /* スリープ */
            Sleep( pParam );
            break;
            
        default:
            /* 不正 */
            
            /* アウトプットパラメータ設定 */
            pParam->ret   = MK_TIMER_RET_FAILURE;
            pParam->errNo = MK_TIMER_ERR_PARAM_FUNCID;
    }
    
    return;
}


/******************************************************************************/
/**
 * @brief       使用中タイマ情報リスト設定
 * @details     タイマ情報を使用中タイマ情報リストの適切な位置に挿入する。
 * 
 * @param[in]   *pTimerInfo タイマ情報
 */
/******************************************************************************/
static void Set( TimerInfo_t *pTimerInfo )
{
    TimerInfo_t *pNext;
    TimerInfo_t *pPrev;
    
    /* 初期化 */
    pNext = NULL;
    pPrev = NULL;
    
    /* 使用中タイマ情報エントリ毎に繰り返し */
    while ( true ) {
        /* タイマ情報エントリ取得 */
        pNext = ( TimerInfo_t * )
            MLibBasicListGetNextNode( &gUsedList,
                                      ( MLibBasicListNode_t * ) pPrev );
        
        /* 取得結果判定 */
        if ( pNext == NULL ) {
            /* 次エントリ無し */
            
            break;
        }
        
        /* 次エントリと残タイマ値比較 */
        if ( pTimerInfo->remain < pNext->remain ) {
            /* 次エントリより短い */
            
            /* 挿入 */
            ( void ) MLibBasicListInsertNext(
                        &gUsedList,
                        ( MLibBasicListNode_t * ) pPrev,
                        ( MLibBasicListNode_t * ) pTimerInfo );
            
            /* 次エントリ残タイマ値減算 */
            pNext->remain -= pTimerInfo->remain;
            
            return;
            
        } else {
            /* 次エントリより長い */
            
            /* 残タイマ値減算 */
            pTimerInfo->remain -= pNext->remain;
        }
        
        pPrev = pNext;
    }
    
    /* 前エントリ有無判定 */
    if ( pPrev == NULL ) {
        /* 前エントリ無し */
        
        /* 先頭に挿入 */
        ( void ) MLibBasicListInsertHead(
                    &gUsedList,
                    ( MLibBasicListNode_t * ) pTimerInfo );
        
    } else {
        /* 前エントリ有り */
        
        /* 挿入 */
        ( void ) MLibBasicListInsertNext(
                    &gUsedList,
                    ( MLibBasicListNode_t * ) pPrev,
                    ( MLibBasicListNode_t * ) pTimerInfo );
    }
    
    return;
}


/******************************************************************************/
/**
 * @brief           スリープ
 * @details         指定した時間の間タスクをスリープ状態にする。
 * 
 * @param[in,out]   *pParam パラメータ
 */
/******************************************************************************/
static void Sleep( MkTimerParam_t *pParam )
{
    uint32_t tick;
    uint32_t timerId;
    
    /* tick変換 */
    tick = pParam->usec / ( 1000000 / MK_CONFIG_TICK_HZ );
    
    /* タイマ設定 */
    timerId = TimerMngCtrlSet( tick, TIMERMNG_TYPE_ONESHOT, SleepTimeout, NULL );
    
    /* タイマ設定結果判定 */
    if ( timerId == TIMERMNG_TIMERID_NULL ) {
        /* 失敗 */
        
        /* アウトプットパラメータ設定 */
        pParam->ret   = MK_TIMER_RET_FAILURE;
        pParam->errNo = MK_TIMER_ERR_NO_RESOURCE;
        
        return;
    }
    
    /* スケジュール停止 */
    TaskMngSchedStop( gTimerInfoTbl[ timerId ].taskId );
    
    /* スケジューラ実行 */
    TaskMngSchedExec();
    
    /* アウトプットパラメータ設定 */
    pParam->ret   = MK_TIMER_RET_SUCCESS;
    pParam->errNo = MK_TIMER_ERR_NONE;
    
    return;
}


/******************************************************************************/
/**
 * @brief       スリープタイムアウト
 * @details     タスクのスケジューリングを再開する。
 * 
 * @param[in]   timerId タイマID
 * @param[in]   *pArg   スリープ引数
 */
/******************************************************************************/
static void SleepTimeout( uint32_t timerId,
                          void     *pArg    )
{
    /* スケジュール開始 */
    TaskMngSchedStart( gTimerInfoTbl[ timerId ].taskId );
    
    /* スケジューラ実行 */
    TaskMngSchedExec();
    
    return;
}


/******************************************************************************/
/**
 * @brief       未使用タイマ情報リスト設定
 * @details     タイマ情報を初期化し、未使用タイマ情報リストに挿入する。
 * 
 * @param[in]   *pTimerInfo タイマ情報
 */
/******************************************************************************/
static void Unset( TimerInfo_t *pTimerInfo )
{
    
    /* タイマ情報初期化 */
    pTimerInfo->remain = 0;
    pTimerInfo->tick   = 0;
    pTimerInfo->type   = TIMERMNG_TYPE_ONESHOT;
    pTimerInfo->pFunc  = NULL;
    pTimerInfo->pArg   = NULL;
    pTimerInfo->taskId = MK_CONFIG_TASKID_NULL;
    
    /* 未使用タイマ情報リスト追加 */
    ( void ) MLibBasicListInsertTail( &gUnusedList,
                                      ( MLibBasicListNode_t * ) pTimerInfo );
    
    return;
}


/******************************************************************************/
