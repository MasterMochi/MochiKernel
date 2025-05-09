/******************************************************************************/
/*                                                                            */
/* src/kernel/IntmngCtrl/IntmngCtrl.c                                         */
/*                                                                 2024/06/18 */
/* Copyright (C) 2018-2024 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdbool.h>

/* カーネルヘッダ */
#include <kernel/interrupt.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32Instruction.h>
#include <hardware/I8259A/I8259A.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Intmng.h>
#include <Taskmng.h>

/* 内部モジュールヘッダ */


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* モジュールID */
#define _MODULE_ID_ CMN_MODULE_INTMNG_CTRL

/** 割込み待ち情報エントリ数 */
#define WAITINFO_ENTRY_NUM I8259A_IRQ_NUM

/* 割込み待ち状態 */
#define STATE_INIT ( 0 )    /**< 初期状態       */
#define STATE_WAIT ( 1 )    /**< 割込み待ち状態 */

/** 割込み監視情報型 */
typedef struct {
    uint32_t waitInfoIdx[ I8259A_IRQ_NUM ];  /**< 割込み待ち情報インデックス */
} MonitoringInfo_t;

/** 割込み待ち情報型 */
typedef struct {
    MkTaskId_t taskId;      /**< タスクID         */
    uint8_t    monitor;     /**< 監視中IRQ        */
    uint8_t    flag;        /**< 割込み発生フラグ */
    uint32_t   state;       /**< 割込み待ち状態   */
} WaitInfo_t;


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* 割込み待ち情報エントリ割り当て */
static uint32_t AllocWaitInfo( MkTaskId_t taskId );

/* 制御権限チェック */
static bool CheckAuthority( MkTaskId_t taskId,
                            uint8_t    irqNo,
                            uint32_t   *pIndex );

/* ハードウェア割込み完了 */
static void Complete( MkTaskId_t   taskId,
                      MkIntParam_t *pParam );

/* ハードウェア割込み無効化 */
static void Disable( MkTaskId_t   taskId,
                     MkIntParam_t *pParam );

/* ハードウェア割込み有効化 */
static void Enable( MkTaskId_t   taskId,
                    MkIntParam_t *pParam );

/* 割込み待ち情報インデックス取得 */
static uint32_t getWaitInfoIdx( MkTaskId_t taskId );

/* ソフトウェア割込みハンドラ */
static void HdlSwInt( uint32_t        intNo,
                      IntmngContext_t context );

/* ハードウェア割込みハンドラ */
static void HdlHwInt( uint32_t        intNo,
                      IntmngContext_t context );

/* ハードウェア割込み監視開始 */
static void StartMonitoring( MkTaskId_t   taskId,
                             MkIntParam_t *pParam );

/* ハードウェア割込み監視停止 */
static void StopMonitoring( MkTaskId_t   taskId,
                            MkIntParam_t *pParam );

/* ハードウェア割込み待ち合わせ */
static void Wait( MkTaskId_t   taskId,
                  MkIntParam_t *pParam );


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** 割込み監視情報 */
static volatile MonitoringInfo_t gMonitoringInfo;

/** 割込み待ち情報 */
static volatile WaitInfo_t gWaitInfo[ WAITINFO_ENTRY_NUM ];


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       割込み待ち情報エントリ割り当て
 * @details     指定したタスクに割込み待ち情報エントリを割り当て、インデックス
 *              を返す。既に割り当て済みの場合は割当済みエントリのインデックス
 *              を返す。
 *
 * @param[in]   taskId タスクID
 *
 * @return      割込み待ち情報エントリのインデックスを返す。
 * @retval      WAITINFO_ENTRY_NUM     空きエントリ無し
 * @retval      WAITINFO_ENTRY_NUM以外 割込み待ち情報インデックス
 */
/******************************************************************************/
static uint32_t AllocWaitInfo( MkTaskId_t taskId )
{
    uint32_t idx;   /* 割込み待ち情報インデックス */
    uint32_t free;  /* 空きエントリインデックス   */

    /* 初期化 */
    free = WAITINFO_ENTRY_NUM;

    /* 割込み待ち情報エントリ毎に繰り返し */
    for ( idx = 0; idx < WAITINFO_ENTRY_NUM; idx++ ) {
        /* タスクID判定 */
        if ( gWaitInfo[ idx ].taskId == taskId ) {
            /* 一致 */

            return idx;

        } else if ( gWaitInfo[ idx ].taskId == MK_TASKID_NULL ) {
            /* 空きエントリ */

            free = idx;
        }
    }

    /* 空きエントリ有無判定 */
    if ( free != WAITINFO_ENTRY_NUM ) {
        /* 空きエントリ有り */

        /* 割当て */
        gWaitInfo[ free ].taskId = taskId;

        return free;
    }

    return WAITINFO_ENTRY_NUM;
}


/******************************************************************************/
/**
 * @brief       ハードウェア割込み制御初期化
 * @details     管理データの初期化、カーネルコール用とハードウェア用の割込みハ
 *              ンドラを設定する。
 */
/******************************************************************************/
void IntmngCtrlInit( void )
{
    uint32_t i;

    DEBUG_LOG_TRC( "%s() start.", __func__ );

    /* 割込み監視情報初期化 */
    for ( i = 0; i < I8259A_IRQ_NUM; i++ ) {
        gMonitoringInfo.waitInfoIdx[ i ] = WAITINFO_ENTRY_NUM;
    }

    /* 割込み待ち情報初期化 */
    for ( i = 0; i < WAITINFO_ENTRY_NUM; i++ ) {
        gWaitInfo[ i ].taskId  = MK_TASKID_NULL;
        gWaitInfo[ i ].monitor = 0;
        gWaitInfo[ i ].flag    = 0;
        gWaitInfo[ i ].state   = STATE_INIT;
    }

    /* ソフトウェア割込みハンドラ設定 */
    IntmngHdlSet( MK_CONFIG_INTNO_INTERRUPT,           &HdlSwInt, IA32_DESCRIPTOR_DPL_3 );

    /* ハードウェア割込みハンドラ設定 */
    IntmngHdlSet( INTMNG_PIC_VCTR_BASE + I8259A_IRQ1,  &HdlHwInt, IA32_DESCRIPTOR_DPL_0 );
    IntmngHdlSet( INTMNG_PIC_VCTR_BASE + I8259A_IRQ3,  &HdlHwInt, IA32_DESCRIPTOR_DPL_0 );
    IntmngHdlSet( INTMNG_PIC_VCTR_BASE + I8259A_IRQ4,  &HdlHwInt, IA32_DESCRIPTOR_DPL_0 );
    IntmngHdlSet( INTMNG_PIC_VCTR_BASE + I8259A_IRQ5,  &HdlHwInt, IA32_DESCRIPTOR_DPL_0 );
    IntmngHdlSet( INTMNG_PIC_VCTR_BASE + I8259A_IRQ6,  &HdlHwInt, IA32_DESCRIPTOR_DPL_0 );
    IntmngHdlSet( INTMNG_PIC_VCTR_BASE + I8259A_IRQ7,  &HdlHwInt, IA32_DESCRIPTOR_DPL_0 );
    IntmngHdlSet( INTMNG_PIC_VCTR_BASE + I8259A_IRQ9,  &HdlHwInt, IA32_DESCRIPTOR_DPL_0 );
    IntmngHdlSet( INTMNG_PIC_VCTR_BASE + I8259A_IRQ10, &HdlHwInt, IA32_DESCRIPTOR_DPL_0 );
    IntmngHdlSet( INTMNG_PIC_VCTR_BASE + I8259A_IRQ11, &HdlHwInt, IA32_DESCRIPTOR_DPL_0 );
    IntmngHdlSet( INTMNG_PIC_VCTR_BASE + I8259A_IRQ12, &HdlHwInt, IA32_DESCRIPTOR_DPL_0 );
    IntmngHdlSet( INTMNG_PIC_VCTR_BASE + I8259A_IRQ13, &HdlHwInt, IA32_DESCRIPTOR_DPL_0 );
    IntmngHdlSet( INTMNG_PIC_VCTR_BASE + I8259A_IRQ14, &HdlHwInt, IA32_DESCRIPTOR_DPL_0 );
    IntmngHdlSet( INTMNG_PIC_VCTR_BASE + I8259A_IRQ15, &HdlHwInt, IA32_DESCRIPTOR_DPL_0 );

    DEBUG_LOG_TRC( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       制御権限チェック
 * @details     タスクが指定したIRQ番号のハードウェア割込みを制御する権限がある
 *              か（割込み監視を開始しているか）チェックする。
 *
 * @param[in]   taskId  タスクID
 * @param[in]   irqNo   IRQ番号
 * @param[out]  *pIndex 割込み待ち情報インデックス
 *                  - NULL     使用しない
 *                  - NULL以外 権限がある場合に設定する。
 *
 * @return      制御権限の有無を返却する。
 * @retval      true  権限有り
 * @retval      false 権限無し
 */
/******************************************************************************/
static bool CheckAuthority( MkTaskId_t taskId,
                            uint8_t    irqNo,
                            uint32_t   *pIndex )
{
    uint32_t idx;   /* 割込み待ち情報インデックス */

    /* 割込み待ち情報インデックス取得 */
    idx = gMonitoringInfo.waitInfoIdx[ irqNo ];

    /* 割込み待ち情報エントリ有無判定 */
    if ( idx == WAITINFO_ENTRY_NUM ) {
        /* エントリ無(非監視中) */

        return false;
    }

    /* 割込み監視開始タスクIDチェック */
    if ( gWaitInfo[ idx ].taskId != taskId ) {
        /* タスクID不一致 */

        return false;
    }

    /* アウトパラメータ要否判定 */
    if ( pIndex != NULL ) {
        /* 必要 */

        *pIndex = idx;
    }

    return true;
}


/******************************************************************************/
/**
 * @brief       ハードウェア割込み処理完了
 * @details     EOI通知を行い、次の割込みを可能にする。
 *
 * @param[in]   taskId  タスクID
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void Complete( MkTaskId_t   taskId,
                      MkIntParam_t *pParam )
{
    bool authority;     /* 制御権限 */

    /* 制御権限チェック */
    authority = CheckAuthority( taskId, pParam->irqNo, NULL );

    /* 制御権限チェック結果判定 */
    if ( authority == false ) {
        /* 権限無し */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_UNAUTHORIZED;

        DEBUG_LOG_WRN(
            "%s(): unauthorized. irqNo=%d, taskId=%d",
            __func__,
            pParam->irqNo,
            taskId
        );

        return;
    }

    /* EOI通知 */
    IntmngPicEoi( pParam->irqNo );

    /* 戻り値設定 */
    pParam->ret = MK_RET_SUCCESS;
    pParam->err = MK_ERR_NONE;

    return;
}


/******************************************************************************/
/**
 * @brief       ハードウェア割込み無効化
 * @details     指定したIRQ番号のハードウェア割込みを無効にする。
 *
 * @param[in]   taskId  タスクID
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void Disable( MkTaskId_t   taskId,
                     MkIntParam_t *pParam )
{
    bool authority;     /* 制御権限 */

    /* 制御権限チェック */
    authority = CheckAuthority( taskId, pParam->irqNo, NULL );

    /* 制御権限チェック結果判定 */
    if ( authority == false ) {
        /* 権限無し */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_UNAUTHORIZED;

        DEBUG_LOG_WRN(
            "%s(): unauthorized. irqNo=%d, taskId=%d",
            __func__,
            pParam->irqNo,
            taskId
        );

        return;
    }

    /* 割込み無効化 */
    IntmngPicDenyIrq( pParam->irqNo );

    /* 戻り値設定 */
    pParam->ret = MK_RET_SUCCESS;
    pParam->err = MK_ERR_NONE;

    DEBUG_LOG_INF( "%s(): irqNo=%d, taskId=%d", pParam->irqNo, taskId );

    return;
}


/******************************************************************************/
/**
 * @brief       ハードウェア割込み有効化
 * @details     指定したIRQ番号のハードウェア割込みを有効にする。
 *
 * @param[in]   taskId  タスクID
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void Enable( MkTaskId_t   taskId,
                    MkIntParam_t *pParam )
{
    bool authority;     /* 制御権限 */

    /* 制御権限チェック */
    authority = CheckAuthority( taskId, pParam->irqNo, NULL );

    /* 制御権限チェック結果判定 */
    if ( authority == false ) {
        /* 権限無し */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_UNAUTHORIZED;

        DEBUG_LOG_WRN(
            "%s(): unauthorized. irqNo=%d, taskId=%d",
            __func__,
            pParam->irqNo,
            taskId
        );

        return;
    }

    /* 割込み有効化 */
    IntmngPicAllowIrq( pParam->irqNo );

    /* 戻り値設定 */
    pParam->ret = MK_RET_SUCCESS;
    pParam->err = MK_ERR_NONE;

    DEBUG_LOG_INF(
        "%s(): irqNo=%d, taskId=%d",
        __func__,
        pParam->irqNo,
        taskId
    );

    return;
}


/******************************************************************************/
/**
 * @brief       割込み待ち情報インデックス取得
 * @details     指定したタスクの割込み待ち情報インデックスを取得する。
 *
 * @param[in]   taskId タスクID
 *
 * @return      割込み待ち情報インデックスを返す。
 * @retval      WAITINFO_ENTRY_NUM     エントリ無し
 * @retval      WAITINFO_ENTRY_NUM以外 割込み待ち情報インデックス
 */
/******************************************************************************/
static uint32_t getWaitInfoIdx( MkTaskId_t taskId )
{
    uint32_t idx;   /* 割込み待ち情報インデックス */

    /* 割込み待ち情報エントリ毎に繰り返し */
    for ( idx = 0; idx < WAITINFO_ENTRY_NUM; idx++ ) {
        /* タスクID判定 */
        if ( gWaitInfo[ idx ].taskId == taskId ) {
            /* 一致 */

            return idx;
        }
    }

    return WAITINFO_ENTRY_NUM;
}


/******************************************************************************/
/**
 * @brief       ソフトウェア割込みハンドラ
 * @details     共通パラメータをチェックし、機能IDから該当する機能を呼び出す。
 *
 * @param[in]   intNo   割込み番号
 * @param[in]   context 割込み発生時コンテキスト
 */
/******************************************************************************/
static void HdlSwInt( uint32_t        intNo,
                      IntmngContext_t context )
{
    uint8_t      type;      /* プロセスタイプ */
    MkTaskId_t   taskId;    /* タスクID       */
    MkIntParam_t *pParam;   /* パラメータ     */

    /* 初期化 */
    pParam = ( MkIntParam_t * ) context.genReg.esi;

    /* タスクID取得 */
    taskId = TaskmngSchedGetTaskId();

    /* パラメータチェック */
    if ( pParam == NULL ) {
        /* 不正 */

        DEBUG_LOG_WRN( "%s(): invalid parameter. taskId=%d", __func__, taskId );

        return;
    }

    /* プロセスタイプ取得 */
    type = TaskmngTaskGetType( taskId );

    /* プロセスタイプチェック */
    if ( type != TASKMNG_PROC_TYPE_DRIVER ) {
        /* 非ドライバプロセス */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_UNAUTHORIZED;

        DEBUG_LOG_WRN(
            "%s(): invalid proctype(%d). taskId=%d",
            __func__,
            type,
            taskId
        );

        return;
    }

    /* 機能ID判定 */
    if ( pParam->funcId == MK_INT_FUNCID_START_MONITORING ) {
        /* ハードウェア割込み監視開始 */

        StartMonitoring( taskId, pParam );

    } else if ( pParam->funcId == MK_INT_FUNCID_STOP_MONITORING ) {
        /* ハードウェア割込み監視終了 */

        StopMonitoring( taskId, pParam );

    } else if ( pParam->funcId == MK_INT_FUNCID_WAIT ) {
        /* ハードウェア割込み待ち合わせ */

        Wait( taskId, pParam );

    } else if ( pParam->funcId == MK_INT_FUNCID_COMPLETE ) {
        /* ハードウェア割込み完了 */

        Complete( taskId, pParam );

    } else if ( pParam->funcId == MK_INT_FUNCID_ENABLE ) {
        /* ハードウェア割込み有効化 */

        Enable( taskId, pParam );

    } else if ( pParam->funcId == MK_INT_FUNCID_DISABLE ) {
        /* ハードウェア割込み無効化 */

        Disable( taskId, pParam );

    } else {
        /* 不明 */

        DEBUG_LOG_WRN(
            "%s(): invalid funcId(%#x). taskId=%d",
            __func__,
            pParam->funcId,
            taskId
        );

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_PARAM;
    }

    return;
}


/******************************************************************************/
/**
 * @brief           ハードウェア割込みハンドラ
 * @details         当該割込み番号の割込み発生フラグをONに設定し、当該の割込み
 *                  待ち合わせを行っているタスクがいる場合はスケジュールを開始
 *                  し割込み待ちを解除する。
 *
 * @param[in]       intNo   割込み番号
 * @param[in]       context 割込み発生時コンテキスト
 */
/******************************************************************************/
static void HdlHwInt( uint32_t        intNo,
                      IntmngContext_t context )
{
    uint8_t  irqNo;     /* IRQ番号                    */
    uint32_t idx;       /* 割込み待ち情報インデックス */

    /* IRQ番号算出 */
    irqNo = ( uint8_t ) ( intNo - INTMNG_PIC_VCTR_BASE );

    /* 割込み待ち情報インデックス取得 */
    idx = gMonitoringInfo.waitInfoIdx[ irqNo ];

    /* 割込み待ち情報エントリ有無判定 */
    if ( idx == WAITINFO_ENTRY_NUM ) {
        /* エントリ無(非監視中) */

        DEBUG_LOG_WRN( "%s(): ignore irqNo(%d)", __func__, irqNo );
        return;
    }

    /* 割込み待ち情報設定 */
    gWaitInfo[ idx ].flag |= ( 1 << irqNo );

    /* 割込み待ち状態判定 */
    if ( gWaitInfo[ idx ].state == STATE_WAIT ) {
        /* 待ち状態 */

        /* スケジュール開始 */
        TaskmngSchedStart( gWaitInfo[ idx ].taskId );
    }

    return;
}


/******************************************************************************/
/**
 * @brief       ハードウェア割込み監視開始
 * @details     指定したIRQ番号のハードウェア割込み監視を開始する。
 *
 * @param[in]   taskId  タスクID
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void StartMonitoring( MkTaskId_t   taskId,
                             MkIntParam_t *pParam )
{
    uint32_t idx;   /* 割込み待ち情報インデックス */

    /* IRQ番号範囲チェック */
    if ( ( pParam->irqNo >= I8259A_IRQ_NUM ) ||
         ( pParam->irqNo == I8259A_IRQ0    ) ||     /* PIT */
         ( pParam->irqNo == I8259A_IRQ2    ) ||     /* PIC */
         ( pParam->irqNo == I8259A_IRQ8    )    ) { /* RTC */
        /* 範囲外 */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_PARAM;

        DEBUG_LOG_WRN(
            "%s(): invalid irqNo(%d). taskId=%d",
            __func__,
            pParam->irqNo,
            taskId
        );

        return;
    }

    /* 監視開始済みチェック */
    if ( gMonitoringInfo.waitInfoIdx[ pParam->irqNo ] != WAITINFO_ENTRY_NUM ) {
        /* 開始済み */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_ALREADY_START;

        DEBUG_LOG_WRN(
            "%s(): already start. irqNo=%d, taskId=%d",
            __func__,
            pParam->irqNo,
            taskId
        );

        return;
    }

    /* 割込み待ち情報エントリ割り当て */
    idx = AllocWaitInfo( taskId );

    /* 割込み監視情報設定 */
    gMonitoringInfo.waitInfoIdx[ pParam->irqNo ] = idx;

    /* 割込み待ち情報設定 */
    gWaitInfo[ idx ].monitor |= ( 1 << pParam->irqNo );

    /* 戻り値設定 */
    pParam->ret = MK_RET_SUCCESS;
    pParam->err = MK_ERR_NONE;

    DEBUG_LOG_INF(
        "%s(): irqNo=%d, taskId=%d, idx=%d, monitor=%02x, flag=%02x",
        __func__,
        pParam->irqNo,
        taskId,
        idx,
        gWaitInfo[ idx ].monitor,
        gWaitInfo[ idx ].flag
    );

    return;
}


/******************************************************************************/
/**
 * @brief       ハードウェア割込み監視終了
 * @details     指定したIRQ番号のハードウェア割込み監視を停止する。
 *
 * @param[in]   taskId  タスクID
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void StopMonitoring( MkTaskId_t   taskId,
                            MkIntParam_t *pParam )
{
    bool     authority;     /* 制御権限                   */
    uint32_t idx;           /* 割込み待ち情報インデックス */

    /* IRQ番号範囲チェック */
    if ( ( pParam->irqNo >= I8259A_IRQ_NUM ) ||
         ( pParam->irqNo == I8259A_IRQ0    ) ||     /* PIT */
         ( pParam->irqNo == I8259A_IRQ2    ) ||     /* PIC */
         ( pParam->irqNo == I8259A_IRQ8    )    ) { /* RTC */
        /* 範囲外 */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_PARAM;

        DEBUG_LOG_WRN(
            "%s(): invalid irqNo(%d). taskId=%d",
            __func__,
            pParam->irqNo,
            taskId
        );

        return;
    }

    /* 制御権限チェック */
    authority = CheckAuthority( taskId, pParam->irqNo, &idx );

    /* 制御権限チェック結果判定 */
    if ( authority == false ) {
        /* 権限無し */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_UNAUTHORIZED;

        DEBUG_LOG_WRN(
            "%s(): unauthorized. irqNo=%d, taskId=%d",
            __func__,
            pParam->irqNo,
            taskId
        );

        return;
    }

    /* 割込み待ち情報設定 */
    gWaitInfo[ idx ].monitor &= ~( 1 << pParam->irqNo );
    gWaitInfo[ idx ].flag    &= ~( 1 << pParam->irqNo );

    /* 他割込み監視判定 */
    if ( gWaitInfo[ idx ].monitor == 0 ) {
        /* 無し */

        /* 割込み待ち情報初期化 */
        gWaitInfo[ idx ].taskId = MK_TASKID_NULL;
    }

    /* 割込み待ち情報インデックス初期化 */
    gMonitoringInfo.waitInfoIdx[ pParam->irqNo ] = WAITINFO_ENTRY_NUM;

    /* 戻り値設定 */
    pParam->ret = MK_RET_SUCCESS;
    pParam->err = MK_ERR_NONE;

    DEBUG_LOG_INF(
        "%s(): irqNo=%d, taskId=%d, idx=%d, monitor=%02x, flag=%02x",
        __func__,
        pParam->irqNo,
        taskId,
        idx,
        gWaitInfo[ idx ].monitor,
        gWaitInfo[ idx ].flag
    );

    return;
}


/******************************************************************************/
/**
 * @brief       ハードウェア割込み待ち合わせ
 * @details     ハードウェア割込みが発生しているか確認する。発生していない場合
 *              は割込みが発生するまで待ち合わせる。
 *
 * @param[in]   taskId  タスクID
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void Wait( MkTaskId_t   taskId,
                  MkIntParam_t *pParam )
{
    uint32_t idx; /* 割込み待ち情報インデックス */

    /* 割込み待ち情報インデックス取得 */
    idx = getWaitInfoIdx( taskId );

    /* 取得結果判定 */
    if ( idx == WAITINFO_ENTRY_NUM ) {
        /* 該当エントリ無し */

        /* エラー設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_UNAUTHORIZED;

        DEBUG_LOG_WRN(
            "%s(): unauthorized. taskId=%d, idx=%d",
            __func__,
            taskId,
            idx
        );

        return;
    }

    /* 割込み発生フラグ判定 */
    if ( gWaitInfo[ idx ].flag == 0 ) {
        /* 割込み未発生 */

        /* 割込み状態設定 */
        gWaitInfo[ idx ].state = STATE_WAIT;

        /* スケジュール停止 */
        TaskmngSchedStop( taskId );

        /* スケジューラ実行 */
        TaskmngSchedExec();
    }

    /* 戻り値設定 */
    pParam->ret  = MK_RET_SUCCESS;
    pParam->err  = MK_ERR_NONE;
    pParam->flag = gWaitInfo[ idx ].flag;

    /* 割込み待ち情報設定 */
    gWaitInfo[ idx ].flag  = 0;
    gWaitInfo[ idx ].state = STATE_INIT;

    return;
}


/******************************************************************************/
