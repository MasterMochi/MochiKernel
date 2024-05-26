/******************************************************************************/
/*                                                                            */
/* src/kernel/Ioctrl/IoctrlMem.c                                              */
/*                                                                 2021/11/27 */
/* Copyright (C) 2018-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <kernel/iomem.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Intmng.h>
#include <Memmng.h>
#include <Taskmng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                 \
    DebugOutput( CMN_MODULE_IOCTRL_MEM,  \
                 __LINE__,               \
                 __VA_ARGS__            )
#else
#define DEBUG_LOG( ... )
#endif


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* 割込みハンドラ */
static void HdlInt( uint32_t        intNo,
                    IntmngContext_t context );


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/


/******************************************************************************/
/* 内部モジュール向けグローバル関数定義                                       */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       I/Oメモリ制御初期化
 * @details     カーネルコール用の割込みハンドラを設定する。
 */
/******************************************************************************/
void MemInit( void )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* 割込みハンドラ設定 */
    IntmngHdlSet( MK_CONFIG_INTNO_IOMEM,        /* 割込み番号     */
                  HdlInt,                       /* 割込みハンドラ */
                  IA32_DESCRIPTOR_DPL_3  );     /* 特権レベル     */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       I/Oメモリ領域割当
 * @details     指定I/Oメモリ領域と仮想メモリ領域を割り当ててI/Oメモリ領域を仮
 *              想メモリ領域にマッピングする。
 *
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void Alloc( MkIoMemParam_t *pParam )
{
    void              *pRet;    /* 戻り値(アドレス)     */
    uint8_t           type;     /* プロセスタイプ       */
    MkPid_t           pid;      /* プロセスID           */
    CmnRet_t          ret;      /* 戻り値               */
    MkTaskId_t        taskId;   /* タスクID             */
    MemmngPageDirId_t dirId;    /* ページディレクトリID */

    /* 初期化 */
    dirId  = MemmngPageGetDirId();
    taskId = TaskmngSchedGetTaskId();
    pid    = MK_TASKID_TO_PID( taskId );
    type   = TaskmngTaskGetType( taskId );

    /* プロセスタイプチェック */
    if ( type != TASKMNG_PROC_TYPE_DRIVER ) {
        /* 非ドライバプロセス */

        /* エラー設定 */
        pParam->ret       = MK_RET_FAILURE;
        pParam->err       = MK_ERR_UNAUTHORIZED;
        pParam->pVirtAddr = NULL;

        return;
    }

    /* I/Oメモリ領域割当 */
    pRet = MemmngIoAlloc( pParam->pIoAddr, pParam->size );

    /* 割当結果判定 */
    if ( pRet == NULL ) {
        /* 失敗 */

        /* エラー設定 */
        pParam->ret       = MK_RET_FAILURE;
        pParam->err       = MK_ERR_IO_ALLOC;
        pParam->pVirtAddr = NULL;

        return;
    }

    /* 仮想メモリ領域割当 */
    pParam->pVirtAddr = MemmngVirtAlloc( pid, pParam->size );

    /* 割当結果判定 */
    if ( pParam->pVirtAddr == NULL ) {
        /* 失敗 */

        /* エラー設定 */
        pParam->ret       = MK_RET_FAILURE;
        pParam->err       = MK_ERR_VIRT_ALLOC;
        pParam->pVirtAddr = NULL;

        return;
    }

    /* ページマッピング設定 */
    ret = MemmngPageSet( dirId,
                         pParam->pVirtAddr,
                         pParam->pIoAddr,
                         pParam->size,
                         IA32_PAGING_G_NO,
                         IA32_PAGING_US_USER,
                         IA32_PAGING_RW_RW    );

    /* 設定結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* エラー設定 */
        pParam->ret       = MK_RET_FAILURE;
        pParam->err       = MK_ERR_PAGE_SET;
        pParam->pVirtAddr = NULL;

        return;
    }

    /* 戻り値設定 */
    pParam->ret = MK_RET_SUCCESS;
    pParam->err = MK_ERR_NONE;

    return;
}


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
                    IntmngContext_t context )
{
    MkIoMemParam_t *pParam;    /* パラメータ   */

    /* 初期化 */
    pParam = ( MkIoMemParam_t * ) context.genReg.esi;

    /* パラメータチェック */
    if ( pParam == NULL ) {
        /* 不正 */

        return;
    }

    /* 機能ID判定 */
    switch ( pParam->funcId ) {
        case MK_IOMEM_FUNCID_ALLOC:
            /* I/Oメモリ領域割当 */
            Alloc( pParam );
            break;

        default:
            /* 不正 */

            /* エラー設定 */
            pParam->ret       = MK_RET_FAILURE;
            pParam->err       = MK_ERR_PARAM;
            pParam->pVirtAddr = NULL;
    }

    return;
}


/******************************************************************************/
