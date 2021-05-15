/******************************************************************************/
/*                                                                            */
/* src/kernel/TaskMng/TaskMngThread.c                                         */
/*                                                                 2021/05/05 */
/* Copyright (C) 2019-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>
#include <stdint.h>

/* カーネルヘッダ */
#include <kernel/thread.h>
#include <kernel/types.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32Instruction.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <IntMng.h>
#include <Memmng.h>
#include <TaskMng.h>

/* 内部モジュールヘッダ */
#include "TaskMngSched.h"
#include "TaskMngTask.h"
#include "TaskMngTbl.h"
#include "TaskMngThread.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                       \
    DebugLogOutput( CMN_MODULE_TASKMNG_THREAD, \
                    __LINE__,                  \
                    __VA_ARGS__                )
#else
#define DEBUG_LOG( ... )
#endif


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* スレッド生成 */
static void DoCreate( MkThreadParam_t *pParam );
/* 割込みハンドラ */
static void HdlInt( uint32_t        intNo,
                    IntMngContext_t context );
/* カーネルスタック設定 */
static CmnRet_t SetKernelStack( TblThreadInfo_t *pThreadInfo );
/* ユーザスタック設定 */
static CmnRet_t SetUserStack( TblThreadInfo_t *pThreadInfo );
/* カーネルスタック削除 */
static void UnsetKernelStack( TblThreadInfo_t *pThreadInfo );
/* ユーザスタック削除 */
static void UnsetUserStack( TblThreadInfo_t *pThreadInfo );


/******************************************************************************/
/* モジュール内グローバル関数定義                                             */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       メインスレッド追加
 * @details     プロセス管理情報pProcInfoにメインスレッド用のスレッド管理情報を
 *              追加し、メインスレッド用のユーザ空間スタック領域を割り当てる。
 *
 * @param[in]   *pProcInfo プロセス管理情報
 *
 * @return      追加したスレッドIDを返す。
 * @retval      0           成功（メインスレッドID）
 * @retval      MK_TID_NULL 失敗
 */
/******************************************************************************/
MkTid_t ThreadAddMain( TblProcInfo_t *pProcInfo )
{
    CmnRet_t        ret;            /* 関数戻り値         */
    MkTaskId_t      taskId;         /* タスクID           */
    TblThreadInfo_t *pThreadInfo;   /* スレッド管理情報   */

    /* 初期化 */
    ret          = CMN_FAILURE;
    taskId       = MK_TASKID_NULL;
    pThreadInfo  = NULL;

    DEBUG_LOG( "%s(): start. pProcInfo=%p", __func__, pProcInfo );

    /* [TODO]スレッド管理テーブルチャンクリスト初期化 */
    MLibListInit( &( pProcInfo->threadChunkList ) );

    /* スレッド管理情報割当 */
    pThreadInfo = TblAllocThreadInfo( pProcInfo );

    /* 割当結果判定 */
    if ( pThreadInfo == NULL ) {
        /* 失敗 */

        return MK_TID_NULL;
    }

    /* スレッドIDチェック */
    if ( pThreadInfo->tid != 0 ) {
        /* メインスレッドIDでない */

        /* スレッド管理情報解放 */
        TblFreeThreadInfo( pThreadInfo );

        return MK_TID_NULL;
    }

    /* カーネルスタック設定 */
    ret = SetKernelStack( pThreadInfo );

    /* 設定結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* スレッド管理情報解放 */
        TblFreeThreadInfo( pThreadInfo );

        return MK_TID_NULL;
    }

    /* ユーザスタック設定 */
    ret = SetUserStack( pThreadInfo );

    /* 設定結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* カーネルスタック削除 */
        UnsetKernelStack( pThreadInfo );

        /* スレッド管理情報解放 */
        TblFreeThreadInfo( pThreadInfo );

        return MK_TID_NULL;
    }

    /* エントリポイント設定 */
    pThreadInfo->pEntryPoint = pThreadInfo->pProcInfo->pEntryPoint;

    /* タスク追加 */
    taskId = TaskAdd( pThreadInfo );

    /* 追加結果判定 */
    if ( taskId == MK_TASKID_NULL ) {
        /* 失敗 */

        /* ユーザスタック削除 */
        UnsetUserStack( pThreadInfo );

        /* カーネルスタック削除 */
        UnsetKernelStack( pThreadInfo );

        /* スレッド管理情報解放 */
        TblFreeThreadInfo( pThreadInfo );

        return MK_TID_NULL;
    }

    DEBUG_LOG( "%s(): end. ret=%u", __func__, pThreadInfo->tid );

    return pThreadInfo->tid;
}


/******************************************************************************/
/**
 * @brief       スレッド制御初期化
 * @details     スレッド制御提供カーネルコール用の割込みハンドラを設定する。
 */
/******************************************************************************/
void ThreadInit( void )
{
    TblProcInfo_t *pIdleProcInfo;   /* アイドルプロセス管理情報 */

    /* 初期化 */
    pIdleProcInfo   = NULL;

    DEBUG_LOG( "%s() start.", __func__ );

    /* アイドルプロセス管理情報取得 */
    pIdleProcInfo = TblGetProcInfo( TASKMNG_PID_IDLE );

    /* [TODO]スレッド管理テーブルチャンクリスト初期化 */
    MLibListInit( &( pIdleProcInfo->threadChunkList ) );

    /* アイドルスレッド管理情報割当 */
    TblAllocThreadInfo( pIdleProcInfo );

    /* 割込みハンドラ設定 */
    IntMngHdlSet( MK_CONFIG_INTNO_THREAD,       /* 割込み番号     */
                  HdlInt,                       /* 割込みハンドラ */
                  IA32_DESCRIPTOR_DPL_3   );    /* 特権レベル     */

    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       スレッド生成
 * @details     スレッドを生成する。
 *
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void DoCreate( MkThreadParam_t *pParam )
{
    CmnRet_t        ret;            /* 関数戻り値         */
    MkTaskId_t      taskId;         /* タスクID           */
    TblProcInfo_t   *pProcInfo;     /* プロセス管理情報   */
    TblStackInfo_t  *pStackInfo;    /* ユーザスタック情報 */
    TblThreadInfo_t *pThreadInfo;   /* スレッド管理情報   */

    /* 初期化 */
    ret         = CMN_FAILURE;
    taskId      = MK_TASKID_NULL;
    pProcInfo   = NULL;
    pStackInfo  = NULL;
    pThreadInfo = NULL;

    /* パラメータチェック */
    if ( ( pParam->pStackAddr == NULL ) ||
         ( pParam->stackSize  == 0    )    ) {
        /* 不正 */

        /* 戻り値設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_PARAM;

        return;
    }

    /* プロセス管理情報取得 */
    pProcInfo = SchedGetProcInfo();

    /* スレッド管理情報割当 */
    pThreadInfo = TblAllocThreadInfo( pProcInfo );

    /* 割当て結果判定 */
    if ( pThreadInfo == NULL ) {
        /* 失敗 */

        /* 戻り値設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_NO_RESOURCE;

        return;
    }

    /* カーネルスタック設定 */
    ret = SetKernelStack( pThreadInfo );

    /* 設定結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* スレッド管理情報解放 */
        TblFreeThreadInfo( pThreadInfo );

        /* 戻り値設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_NO_RESOURCE;

        return;
    }

    /* ユーザスタック情報設定 */
    pStackInfo              = &( pThreadInfo->userStack );
    pStackInfo->pTopAddr    = pParam->pStackAddr;
    pStackInfo->pBottomAddr = ( void * )( ( uint32_t ) pParam->pStackAddr +
                                          pParam->stackSize               -
                                          sizeof ( uint32_t )               );
    pStackInfo->size        = pParam->stackSize;

    /* エントリポイント設定 */
    pThreadInfo->pEntryPoint = pParam->pFunc;

    /* タスク追加 */
    taskId = TaskAdd( pThreadInfo );

    /* 追加結果判定 */
    if ( taskId == MK_TASKID_NULL ) {
        /* 失敗 */

        /* ユーザスタック削除 */
        UnsetUserStack( pThreadInfo );

        /* カーネルスタック削除 */
        UnsetKernelStack( pThreadInfo );

        /* スレッド管理情報解放 */
        TblFreeThreadInfo( pThreadInfo );

        /* 戻り値設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_NO_RESOURCE;

        return;
    }

    /* 戻り値設定 */
    pParam->ret    = MK_RET_SUCCESS;
    pParam->err    = MK_ERR_NONE;
    pParam->taskId = taskId;

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
                    IntMngContext_t context )
{
    MkThreadParam_t *pParam;    /* パラメータ */

    /* 初期化 */
    pParam = ( MkThreadParam_t * ) context.genReg.esi;

    DEBUG_LOG( "%s(): start. pParam=%p", __func__, pParam );

    /* パラメータチェック */
    if ( pParam == NULL ) {
        /* 不正 */

        DEBUG_LOG( "%s(): end.", __func__ );

        return;
    }

    /* 機能ID判定 */
    switch ( pParam->funcId ) {
        case MK_THREAD_FUNCID_CREATE:
            /* スレッド生成 */

            DEBUG_LOG( "%s(): create.", __func__ );
            DoCreate( pParam );
            break;

        default:
            /* 不正 */

            /* エラー設定 */
            pParam->ret = MK_RET_FAILURE;
            pParam->err = MK_ERR_PARAM;
    }

    DEBUG_LOG( "%s(): end. ret=%d, err=%u",
               __func__,
               pParam->ret,
               pParam->err                  );

    return;
}


/******************************************************************************/
/**
 * @brief       カーネルスタック設定
 * @details     カーネルスタック領域を新たに割当てて、pThreadInfoにスタック情報
 *              を設定する。
 *
 * @param[in]   *pThreadInfo スレッド管理情報
 *
 * @return      処理結果を返す。
 * @retval      CMN_SUCCESS 成功
 * @retval      CMN_FAILURE 失敗
 */
/******************************************************************************/
static CmnRet_t SetKernelStack( TblThreadInfo_t *pThreadInfo )
{
    void           *pKernelStack;   /* カーネルスタック領域 */
    TblStackInfo_t *pStackInfo;     /* スタック情報         */

    /* 初期化 */
    pKernelStack = NULL;
    pStackInfo   = &( pThreadInfo->kernelStack );

    /* カーネルスタック領域割当 */
    pKernelStack = MemmngHeapAlloc( MK_CONFIG_SIZE_KERNEL_STACK );

    /* 割当結果判定 */
    if ( pKernelStack == NULL ) {
        /* 失敗 */

        return CMN_FAILURE;
    }

    /* カーネルスタック情報設定 */
    pStackInfo->pTopAddr    = pKernelStack;
    pStackInfo->pBottomAddr = ( void * ) ( ( uint32_t ) pKernelStack   +
                                           MK_CONFIG_SIZE_KERNEL_STACK -
                                           sizeof ( uint32_t )           );
    pStackInfo->size        = MK_CONFIG_SIZE_KERNEL_STACK;

    return CMN_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       ユーザスタック設定
 * @details     ユーザスタック領域を新たに割当てて、pThreadInfoにスタック情報を
 *              設定する。
 *
 * @param[in]   *pThreadInfo スレッド管理情報
 *
 * @return      処理結果を返す。
 * @retval      CMN_SUCCESS 成功
 * @retval      CMN_FAILURE 失敗
 */
/******************************************************************************/
static CmnRet_t SetUserStack( TblThreadInfo_t *pThreadInfo )
{
    void           *pPhysAddr;  /* 物理メモリ領域 */
    CmnRet_t       ret;         /* 関数戻り値     */
    TblStackInfo_t *pStackInfo; /* スタック情報   */

    /* 初期化 */
    pPhysAddr  = NULL;
    ret        = CMN_FAILURE;
    pStackInfo = &( pThreadInfo->userStack );

    /* 物理メモリ領域割当 */
    pPhysAddr = MemmngPhysAlloc( MK_CONFIG_SIZE_USER_STACK );

    /* 割当結果判定 */
    if ( pPhysAddr == NULL ) {
        /* 失敗 */

        return CMN_FAILURE;
    }

    /* ページマッピング */
    ret = MemmngPageSet(
              pThreadInfo->pProcInfo->dirId,        /* ページディレクトリID */
              ( void * ) MK_CONFIG_ADDR_USER_STACK, /* 仮想アドレス         */
              pPhysAddr,                            /* 物理アドレス         */
              MK_CONFIG_SIZE_USER_STACK,            /* マッピングサイズ     */
              IA32_PAGING_G_NO,                     /* グローバルフラグ     */
              IA32_PAGING_US_USER,                  /* ユーザ/スーパバイザ  */
              IA32_PAGING_RW_RW                     /* 読込/書込許可        */
          );

    /* マッピング結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* 物理メモリ領域解放 */
        MemmngPhysFree( pPhysAddr );

        return CMN_FAILURE;
    }

    /* ユーザスタック情報設定 */
    pStackInfo->pTopAddr    = ( void * ) MK_CONFIG_ADDR_USER_STACK;
    pStackInfo->pBottomAddr = ( void * ) ( MK_CONFIG_ADDR_USER_STACK +
                                           MK_CONFIG_SIZE_USER_STACK -
                                           sizeof ( uint32_t )         );
    pStackInfo->size        = MK_CONFIG_SIZE_USER_STACK;

    return CMN_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       カーネルスタック削除
 * @details     カーネルスタック領域を解放する。
 *
 * @param[in]   pThreadInfo スレッド管理情報
 */
/******************************************************************************/
static void UnsetKernelStack( TblThreadInfo_t *pThreadInfo )
{
    TblStackInfo_t *pStackInfo; /* スタック情報 */

    /* 初期化 */
    pStackInfo = &( pThreadInfo->kernelStack );

    /* カーネルスタック領域解放 */
    MemmngHeapFree( pStackInfo->pTopAddr );

    /* カーネルスタック情報設定 */
    pStackInfo->pTopAddr    = NULL;
    pStackInfo->pBottomAddr = NULL;
    pStackInfo->size        = 0;

    return;
}


/******************************************************************************/
/**
 * @brief       ユーザスタック削除
 * @details     メインスレッドの場合はユーザスタック領域の解放とメモリマッピン
 *              グ解除を行う。
 *
 * @param[in]   pThreadInfo スレッド管理情報
 */
/******************************************************************************/
static void UnsetUserStack( TblThreadInfo_t *pThreadInfo )
{
    TblStackInfo_t *pStackInfo; /* スタック情報 */

    /* 初期化 */
    pStackInfo = &( pThreadInfo->userStack );

    /* スレッドID判定 */
    if ( pThreadInfo->tid == 0 ) {
        /* メインスレッド */

        /* マッピング解除 */
        MemmngPageUnset(
            pThreadInfo->pProcInfo->dirId,          /* ページディレクトリID */
            ( void * ) MK_CONFIG_ADDR_USER_STACK,   /* 仮想アドレス         */
            MK_CONFIG_SIZE_USER_STACK               /* マッピングサイズ     */
        );

        /* カーネルスタック領域解放 */
        MemmngHeapFree( pStackInfo->pTopAddr );
    }

    /* ユーザスタック情報設定 */
    pStackInfo->pTopAddr    = NULL;
    pStackInfo->pBottomAddr = NULL;
    pStackInfo->size        = 0;

    return;
}


/******************************************************************************/
