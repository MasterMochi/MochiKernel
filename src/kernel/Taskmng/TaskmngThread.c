/******************************************************************************/
/*                                                                            */
/* src/kernel/Taskmng/TaskmngThread.c                                         */
/*                                                                 2021/05/31 */
/* Copyright (C) 2019-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>

/* ライブラリ */
#include <MLib/MLibDynamicArray.h>

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
#include <Taskmng.h>

/* 内部モジュールヘッダ */
#include "TaskmngSched.h"
#include "TaskmngTask.h"
#include "TaskmngThread.h"


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
static CmnRet_t SetKernelStack( ThreadInfo_t *pThreadInfo );
/* ユーザスタック設定 */
static CmnRet_t SetUserStack( ThreadInfo_t *pThreadInfo );
/* カーネルスタック削除 */
static void UnsetKernelStack( ThreadInfo_t *pThreadInfo );
/* ユーザスタック削除 */
static void UnsetUserStack( ThreadInfo_t *pThreadInfo );


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
MkTid_t ThreadAddMain( ProcInfo_t *pProcInfo )
{
    MkTid_t      tid;           /* スレッドID             */
    CmnRet_t     ret;           /* 関数戻り値             */
    MLibErr_t    errMLib;       /* MLIBライブラリエラー値 */
    MLibRet_t    retMLib;       /* MLIBライブラリ戻り値   */
    MkTaskId_t   taskId;        /* タスクID               */
    ThreadInfo_t *pThreadInfo;  /* スレッド管理情報       */

    /* 初期化 */
    tid          = MK_TID_NULL;
    ret          = CMN_FAILURE;
    errMLib      = MLIB_ERR_NONE;
    retMLib      = MLIB_RET_FAILURE;
    taskId       = MK_TASKID_NULL;
    pThreadInfo  = NULL;

    DEBUG_LOG( "%s(): start. pProcInfo=%p", __func__, pProcInfo );

    /* スレッド管理情報動的配列初期化 */
    retMLib = MLibDynamicArrayInit( &( pProcInfo->threadTbl ),
                                    THREAD_TBL_CHUNK_SIZE,
                                    sizeof ( ThreadInfo_t ),
                                    UINT_MAX,
                                    &errMLib                   );

    /* 初期化結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        return MK_TID_NULL;
    }

    /* スレッド管理情報割当 */
    retMLib = MLibDynamicArrayAlloc( &( pProcInfo->threadTbl ),
                                     ( uint_t *  ) &tid,
                                     ( void   ** ) &pThreadInfo,
                                     &errMLib                    );

    /* 割当結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        /* スレッド管理情報動的配列削除 */
        MLibDynamicArrayExit( &( pProcInfo->threadTbl ), NULL );

        return MK_TID_NULL;
    }

    /* スレッド管理情報設定 */
    pThreadInfo->tid       = tid;
    pThreadInfo->taskId    = MK_TASKID_MAKE( pProcInfo->pid, tid );
    pThreadInfo->pProcInfo = pProcInfo;

    /* カーネルスタック設定 */
    ret = SetKernelStack( pThreadInfo );

    /* 設定結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* スレッド管理情報動的配列削除 */
        MLibDynamicArrayExit( &( pProcInfo->threadTbl ), NULL );

        /* スレッド管理情報解放 */
        MLibDynamicArrayFree( &( pProcInfo->threadTbl ),
                              ( uint_t ) tid,
                              &errMLib                   );

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
        MLibDynamicArrayFree( &( pProcInfo->threadTbl ),
                              ( uint_t ) tid,
                              &errMLib                   );

        /* スレッド管理情報動的配列削除 */
        MLibDynamicArrayExit( &( pProcInfo->threadTbl ), NULL );

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
        MLibDynamicArrayFree( &( pProcInfo->threadTbl ),
                              ( uint_t ) tid,
                              &errMLib                   );

        /* スレッド管理情報動的配列削除 */
        MLibDynamicArrayExit( &( pProcInfo->threadTbl ), NULL );

        return MK_TID_NULL;
    }

    DEBUG_LOG( "%s(): end. ret=%u", __func__, pThreadInfo->tid );

    return pThreadInfo->tid;
}


/******************************************************************************/
/**
 * @brief       アイドルプロセス用メインスレッド追加
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
MkTid_t ThreadAddMainIdle( ProcInfo_t *pProcInfo )
{
    MkTid_t      tid;           /* スレッドID             */
    MLibErr_t    errMLib;       /* MLIBライブラリエラー値 */
    MLibRet_t    retMLib;       /* MLIBライブラリ戻り値   */
    ThreadInfo_t *pThreadInfo;  /* スレッド管理情報       */

    /* 初期化 */
    tid         = MK_TID_NULL;
    errMLib     = MLIB_ERR_NONE;
    retMLib     = MLIB_RET_FAILURE;
    pThreadInfo = NULL;

    /* スレッド管理情報動的配列初期化 */
    retMLib = MLibDynamicArrayInit( &( pProcInfo->threadTbl ),
                                    THREAD_TBL_CHUNK_SIZE,
                                    sizeof ( ThreadInfo_t ),
                                    UINT_MAX,
                                    &errMLib                   );

    /* 初期化結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        return MK_TID_NULL;
    }

    /* スレッド管理情報割当 */
    retMLib = MLibDynamicArrayAlloc( &( pProcInfo->threadTbl ),
                                     &tid,
                                     ( void ** ) &pThreadInfo,
                                     &errMLib                   );

    /* 割当結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        /* スレッド管理情報動的配列削除 */
        MLibDynamicArrayExit( &( pProcInfo->threadTbl ), NULL );

        return MK_TID_NULL;
    }

    /* スレッド管理情報設定 */
    pThreadInfo->tid       = tid;
    pThreadInfo->taskId    = MK_TASKID_MAKE( pProcInfo->pid, tid );
    pThreadInfo->pProcInfo = pProcInfo;

    return tid;
}


/******************************************************************************/
/**
 * @brief       スレッド管理情報取得
 * @details     引数tidのスレッド管理情報を取得する。
 *
 * @param[in]   *pProcInfo プロセス管理情報
 * @param[in]   tid        スレッドID
 *
 * @return      スレッド管理情報を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功(スレッド管理情報)
 */
/******************************************************************************/
ThreadInfo_t *ThreadGetInfo( ProcInfo_t *pProcInfo,
                             MkTid_t    tid         )
{
    MLibErr_t    errMLib;       /* MLIBライブラリエラー値 */
    MLibRet_t    retMLib;       /* MLIBライブラリ戻り値   */
    ThreadInfo_t *pThreadInfo;  /* スレッド管理情報       */

    /* 初期化 */
    errMLib     = MLIB_ERR_NONE;
    retMLib     = MLIB_RET_SUCCESS;
    pThreadInfo = NULL;

    /* スレッド管理情報取得 */
    retMLib = MLibDynamicArrayGet( &( pProcInfo->threadTbl ),
                                   tid,
                                   ( void ** ) &pThreadInfo,
                                   &errMLib                   );

    /* 取得結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        return NULL;
    }

    return pThreadInfo;
}


/******************************************************************************/
/**
 * @brief       スレッド制御初期化
 * @details     スレッド制御提供カーネルコール用の割込みハンドラを設定する。
 */
/******************************************************************************/
void ThreadInit( void )
{
    DEBUG_LOG( "%s() start.", __func__ );

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
    MkTid_t           tid;          /* スレッドID             */
    CmnRet_t          ret;          /* 関数戻り値             */
    MLibErr_t         errMLib;      /* MLIBライブラリエラー値 */
    MLibRet_t         retMLib;      /* MLIBライブラリ戻り値   */
    MkTaskId_t        taskId;       /* タスクID               */
    ProcInfo_t        *pProcInfo;   /* プロセス管理情報       */
    ThreadInfo_t      *pThreadInfo; /* スレッド管理情報       */
    ThreadStackInfo_t *pStackInfo;  /* ユーザスタック情報     */

    /* 初期化 */
    tid         = MK_TID_NULL;
    ret         = CMN_FAILURE;
    errMLib     = MLIB_ERR_NONE;
    retMLib     = MLIB_RET_FAILURE;
    taskId      = MK_TASKID_NULL;
    pProcInfo   = NULL;
    pThreadInfo = NULL;
    pStackInfo  = NULL;

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
    retMLib = MLibDynamicArrayAlloc( &( pProcInfo->threadTbl ),
                                     &tid,
                                     ( void ** ) &pThreadInfo,
                                     &errMLib                   );

    /* 割当て結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        /* 戻り値設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_NO_RESOURCE;

        return;
    }

    /* スレッド管理情報設定 */
    pThreadInfo->tid       = tid;
    pThreadInfo->taskId    = MK_TASKID_MAKE( pProcInfo->pid, tid );
    pThreadInfo->pProcInfo = pProcInfo;

    /* カーネルスタック設定 */
    ret = SetKernelStack( pThreadInfo );

    /* 設定結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* スレッド管理情報解放 */
        MLibDynamicArrayFree( &( pProcInfo->threadTbl ), ( uint_t ) tid, NULL );

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
        MLibDynamicArrayFree( &( pProcInfo->threadTbl ), tid, NULL );

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
static CmnRet_t SetKernelStack( ThreadInfo_t *pThreadInfo )
{
    void              *pKernelStack;    /* カーネルスタック領域 */
    ThreadStackInfo_t *pStackInfo;      /* スタック情報         */

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
static CmnRet_t SetUserStack( ThreadInfo_t *pThreadInfo )
{
    void              *pPhysAddr;   /* 物理メモリ領域 */
    CmnRet_t          ret;          /* 関数戻り値     */
    ThreadStackInfo_t *pStackInfo;  /* スタック情報   */

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
static void UnsetKernelStack( ThreadInfo_t *pThreadInfo )
{
    ThreadStackInfo_t *pStackInfo;  /* スタック情報 */

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
static void UnsetUserStack( ThreadInfo_t *pThreadInfo )
{
    ThreadStackInfo_t *pStackInfo;  /* スタック情報 */

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
