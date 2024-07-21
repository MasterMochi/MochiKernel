/******************************************************************************/
/*                                                                            */
/* src/kernel/Taskmng/TaskmngProc.c                                           */
/*                                                                 2024/06/18 */
/* Copyright (C) 2018-2024 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>

/* ライブラリヘッダ */
#include <MLib/MLibUtil.h>

/* 共通ヘッダ */
#include <memmap.h>
#include <kernel/config.h>
#include <kernel/proc.h>
#include <kernel/types.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Intmng.h>
#include <Memmng.h>
#include <Taskmng.h>

/* 内部モジュールヘッダ */
#include "TaskmngElf.h"
#include "TaskmngSched.h"
#include "TaskmngTask.h"
#include "TaskmngThread.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* モジュールID */
#define _MODULE_ID_ CMN_MODULE_TASKMNG_PROC

/** プロセス管理情報動的配列チャンクサイズ */
#define PROCTBL_CHUNK_SIZE ( 8 )


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* プロセス管理情報割当 */
static ProcInfo_t *AllocProcInfo( uint8_t type );
/* プロセス管理情報解放 */
static void FreeProcInfo( ProcInfo_t *pProcInfo );
/* 割込みハンドラ */
static void HdlInt( uint32_t        intNo,
                    IntmngContext_t context );
/* ブレイクポイント設定 */
static void SetBreakPoint( MkProcParam_t *pParam );
/* ユーザスタック情報設定 */
static CmnRet_t SetUserStack( ProcInfo_t *pProcInfo );
/* ユーザスタック情報削除 */
static void UnsetUserStack( ProcInfo_t *pProcInfo );


/******************************************************************************/
/* 静的グローバル変数定義                                                     */
/******************************************************************************/
/* プロセス管理情報動的配列 */
static MLibDynamicArray_t gProcTbl;


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       プロセス追加
 * @details     実行ファイルから新しいプロセスを追加する。
 *
 * @param[in]   type   プロセスタイプ
 *                  - MK_PROCESS_TYPE_DRIVER ドライバプロセス
 *                  - MK_PROCESS_TYPE_SERVER サーバプロセス
 *                  - MK_PROCESS_TYPE_USER   ユーザプロセス
 * @param[in]   *pAddr 実行ファイルアドレス
 * @param[in]   size   実行ファイルサイズ
 *
 * @return      追加時に割り当てたプロセスIDを返す。
 * @retval      MK_PID_NULL 失敗
 * @retval      MK_PID_MIN  タスクID最小値
 * @retval      MK_PID_MAX  タスクID最大値
 */
/******************************************************************************/
MkPid_t TaskmngProcAdd( uint8_t type,
                        void    *pAddr,
                        size_t  size    )
{
    void       *pEndPoint;      /* エンドポイント   */
    CmnRet_t   ret;             /* 関数戻り値       */
    MkTid_t    tid;             /* スレッドID       */
    ProcInfo_t *pProcInfo;      /* プロセス管理情報 */

    /* 初期化 */
    pEndPoint   = NULL;
    ret         = CMN_FAILURE;
    tid         = MK_TID_NULL;
    pProcInfo   = NULL;

    DEBUG_LOG_TRC( "%s(): start.", __func__ );

    /* プロセス管理情報割当 */
    pProcInfo = AllocProcInfo( type );

    /* 割当結果判定 */
    if ( pProcInfo == NULL ) {
        /* 失敗 */

        DEBUG_LOG_ERR( "%s(): failed.", __func__ );

        return MK_PID_NULL;
    }

    /* ページディレクトリ割当 */
    pProcInfo->dirId = MemmngPageAllocDir( pProcInfo->pid );

    /* 割当結果判定 */
    if ( pProcInfo->dirId == MEMMNG_PAGE_DIR_ID_NULL ) {
        /* 失敗 */

        DEBUG_LOG_ERR( "%s(): failed.", __func__ );

        /* プロセス管理情報解放 */
        FreeProcInfo( pProcInfo );

        return MK_PID_NULL;
    }

    /* ページディレクトリベースレジスタ値取得 */
    pProcInfo->pdbr = MemmngPageGetPdbr( pProcInfo->dirId );

    /* 仮想メモリ領域管理開始 */
    ret = MemmngVirtStart( pProcInfo->pid );

    /* 管理開始結果判定 */
    if ( ret == CMN_FAILURE ) {
        /* 失敗 */

        DEBUG_LOG_ERR( "%s(): failed.", __func__ );

        /* プロセス管理情報解放 */
        FreeProcInfo( pProcInfo );

        return MK_PID_NULL;
    }

    /* ELFファイル読込 */
    ret = ElfLoad( pAddr,
                   size,
                   pProcInfo->dirId,
                   &( pProcInfo->pEntryPoint ),
                   &pEndPoint                   );

    /* 読込結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        DEBUG_LOG_ERR( "%s(): failed.", __func__ );

        /* プロセス管理情報解放 */
        FreeProcInfo( pProcInfo );

        return MK_PID_NULL;
    }

    /* ブレイクポイント設定 */
    pProcInfo->userHeap.pEndPoint   = pEndPoint;
    pProcInfo->userHeap.pBreakPoint =
        ( void * ) MLIB_UTIL_ALIGN( ( uint32_t ) pEndPoint - 1,
                                    IA32_PAGING_PAGE_SIZE       );

    /* ユーザスタック設定 */
    ret = SetUserStack( pProcInfo );

    /* 設定結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        DEBUG_LOG_ERR( "%s(): failed.", __func__ );

        /* プロセス管理情報解放 */
        FreeProcInfo( pProcInfo );

        return MK_PID_NULL;
    }

    /* スレッド追加 */
    tid = ThreadAddMain( pProcInfo );

    /* 追加結果判定 */
    if ( tid == MK_TID_NULL ) {
        /* 失敗 */

        DEBUG_LOG_ERR( "%s(): failed.", __func__ );

        /* プロセス管理情報解放 */
        FreeProcInfo( pProcInfo );

        return MK_PID_NULL;
    }

    DEBUG_LOG_TRC( "%s(): end. ret=%d", __func__, pProcInfo->pid );

    return pProcInfo->pid;
}


/******************************************************************************/
/* モジュール内グローバル関数定義                                             */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       プロセス管理情報取得
 * @details     引数pidのプロセス管理情報を取得する。
 *
 * @param[in]   pid プロセスID
 *
 * @return      プロセス管理情報を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功(プロセス管理情報)
 */
/******************************************************************************/
ProcInfo_t *ProcGetInfo( MkPid_t pid )
{
    MLibErr_t  errMLib;     /* MLIBライブラリエラー値 */
    MLibRet_t  retMLib;     /* MLIBライブラリ戻り値   */
    ProcInfo_t *pProcInfo;  /* プロセス管理情報       */

    /* 初期化 */
    errMLib   = MLIB_ERR_NONE;
    retMLib   = MLIB_RET_FAILURE;
    pProcInfo = NULL;

    /* プロセス管理情報取得 */
    retMLib = MLibDynamicArrayGet( &gProcTbl,
                                   ( uint_t  ) pid,
                                   ( void ** ) &pProcInfo,
                                   &errMLib                );

    /* 取得結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        return NULL;
    }

    return pProcInfo;
}


/******************************************************************************/
/**
 * @brief       プロセス管理初期化
 * @details     プロセス管理情報テーブルの初期化、アイドルプロセス用のプロセス
 *              管理情報の設定、および、プロセス管理提供のカーネルコール用割込
 *              みハンドラを設定する。
 */
/******************************************************************************/
void ProcInit( void )
{
    ProcInfo_t     *pProcInfo;  /* アイドルプロセス管理情報 */
    ProcHeapInfo_t *pUserHeap;  /* ユーザヒープ情報         */

    DEBUG_LOG_TRC( "%s() start.", __func__ );

    /* 初期化 */
    pProcInfo = NULL;
    pUserHeap = NULL;

    /* プロセス管理情報動的配列初期化 */
    MLibDynamicArrayInit( &gProcTbl,
                          PROCTBL_CHUNK_SIZE,
                          sizeof ( ProcInfo_t ),
                          UINT_MAX,
                          NULL                   );

    /* アイドルプロセス管理情報割当 */
    pProcInfo = AllocProcInfo( TASKMNG_PROC_TYPE_KERNEL );

    /* アイドルプロセス管理情報設定 */
    pProcInfo->pEntryPoint = NULL;
    pProcInfo->dirId       = MEMMNG_PAGE_DIR_ID_IDLE;
    pProcInfo->pdbr        = MemmngPageGetPdbr( MEMMNG_PAGE_DIR_ID_IDLE );

    /* ユーザヒープ情報設定 */
    pUserHeap              = &( pProcInfo->userHeap );
    pUserHeap->pEndPoint   = NULL;
    pUserHeap->pBreakPoint = NULL;

    /* アイドルプロセス用スレッド情報設定 */
    ThreadAddMainIdle( pProcInfo );

    /* 割込みハンドラ設定 */
    IntmngHdlSet( MK_PROC_INTNO,            /* 割込み番号     */
                  HdlInt,                   /* 割込みハンドラ */
                  IA32_DESCRIPTOR_DPL_3 );  /* 特権レベル     */

    DEBUG_LOG_TRC( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       プロセス管理情報割当
 * @details     プロセス管理情報を割り当てて、初期化する。
 *
 * @param[in]   type プロセスタイプ
 *                  - MK_PROCESS_TYPE_DRIVER ドライバプロセス
 *                  - MK_PROCESS_TYPE_SERVER サーバプロセス
 *                  - MK_PROCESS_TYPE_USER   ユーザプロセス
 *
 * @return      プロセス管理情報を返す。
 * @retval      NULL     失敗
 * @retval      NULL以外 成功(プロセス管理情報)
 */
/******************************************************************************/
static ProcInfo_t *AllocProcInfo( uint8_t type )
{
    MkPid_t    pid;         /* プロセスID             */
    MLibErr_t  errMLib;     /* MLibライブラリエラー値 */
    MLibRet_t  retMLib;     /* MLibライブラリ戻り値   */
    ProcInfo_t *pProcInfo;  /* プロセス管理情報       */

    /* 初期化 */
    pid       = MK_PID_NULL;
    errMLib   = MLIB_ERR_NONE;
    retMLib   = MLIB_RET_FAILURE;
    pProcInfo = NULL;

    /* プロセス管理情報割当 */
    retMLib = MLibDynamicArrayAlloc( &gProcTbl,
                                     ( uint_t *  ) &pid,
                                     ( void   ** ) &pProcInfo,
                                     &errMLib                  );

    /* 割当結果判定 */
    if ( retMLib != MLIB_RET_SUCCESS ) {
        /* 失敗 */

        return NULL;
    }

    /* プロセス管理情報初期化 */
    MLibUtilSetMemory8( pProcInfo, 0, sizeof ( ProcInfo_t ) );
    pProcInfo->pid  = pid;
    pProcInfo->type = type;

    return pProcInfo;
}


/******************************************************************************/
/**
 * @brief           プロセス複製
 * @details         プロセスを複製する。
 *
 * @param[in,out]   *pParam パラメータ
 */
/******************************************************************************/
static void DoFork( MkProcParam_t *pParam )
{
    MkTid_t    tid;             /* スレッドID         */
    CmnRet_t   ret;             /* 関数戻り値         */
    ProcInfo_t *pChildInfo;     /* 子プロセス管理情報 */
    ProcInfo_t *pParentInfo;    /* 親プロセス管理情報 */
    ProcInfo_t *pNewInfo;       /* 現プロセス管理情報 */

    /* 初期化 */
    tid         = MK_TID_NULL;
    ret         = CMN_FAILURE;
    pChildInfo  = NULL;
    pParentInfo = NULL;

    /* 親プロセス管理情報取得 */
    pParentInfo = SchedGetProcInfo();

    /* 子プロセス管理情報割当 */
    pChildInfo = AllocProcInfo( TASKMNG_PROC_TYPE_USER );

    /* 割当結果判定 */
    if ( pChildInfo == NULL ) {
        /* 失敗 */

        /* 戻り値設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_NO_RESOURCE;

        return;
    }

    /* ページディレクトリ割当 */
    pChildInfo->dirId = MemmngPageAllocDir( pChildInfo->pid );

    /* 割当結果判定 */
    if ( pChildInfo->dirId == MEMMNG_PAGE_DIR_ID_NULL ) {
        /* 失敗 */

        /* プロセス管理情報解放 */
        FreeProcInfo( pChildInfo );

        /* 戻り値設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_NO_RESOURCE;

        return;
    }

    /* PDBR取得 */
    pChildInfo->pdbr = MemmngPageGetPdbr( pChildInfo->dirId );

    /* 仮想メモリ領域管理開始 */
    ret = MemmngVirtStart( pChildInfo->pid );

    /* 開始結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* プロセス管理情報解放 */
        FreeProcInfo( pChildInfo );

        /* 戻り値設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_NO_RESOURCE;

        return;
    }

    /* ページ複製 */
    ret = MemmngPageCopy( pChildInfo->dirId,
                          pParentInfo->dirId,
                          ( void * ) MEMMAP_VADDR_USER,
                          MEMMAP_VSIZE_USER             );

    /* 複製結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* プロセス管理情報解放 */
        FreeProcInfo( pChildInfo );

        /* 戻り値設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_NO_RESOURCE;

        return;
    }

    /* 子プロセス管理情報設定 */
    pChildInfo->parent              = pParentInfo->pid;
    pChildInfo->pEntryPoint         = pParentInfo->pEntryPoint;
    pChildInfo->userHeap            = pParentInfo->userHeap;
    pChildInfo->userStack           = pParentInfo->userStack;
    pChildInfo->userStack.pPhysAddr = NULL;

    /* スレッド複製 */
    tid = ThreadFork( pChildInfo );

    /* 複製結果判定 */
    if ( tid == MK_TID_NULL ) {
        /* 失敗 */

        /* プロセス管理情報解放 */
        FreeProcInfo( pChildInfo );

        /* 戻り値設定 */
        pParam->ret = MK_RET_FAILURE;
        pParam->err = MK_ERR_NO_RESOURCE;

        return;
    }

    /* プロセス管理情報取得 */
    pNewInfo = SchedGetProcInfo();

    /* 子プロセス判定 */
    if ( pNewInfo == pChildInfo ) {
        /* 子プロセス */

        /* プロセスID設定 */
        pParam->pid = pNewInfo->pid;

    } else {
        /* 親プロセス */

        /* プロセスID設定 */
        pParam->pid = MK_PID_NULL;
    }

    /* 戻り値設定 */
    pParam->ret = MK_RET_SUCCESS;
    pParam->err = MK_ERR_NONE;

    return;
}


/******************************************************************************/
/**
 * @brief       プロセス管理情報解放
 * @details     プロセス管理情報を解放する。
 *
 * @param[in]   *pProcInfo プロセス管理情報
 */
/******************************************************************************/
static void FreeProcInfo( ProcInfo_t *pProcInfo )
{
    MLibErr_t errMLib;  /* MLibライブラリエラー値 */

    /* 初期化 */
    errMLib = MLIB_ERR_NONE;

    /* ユーザスタック削除 */
    UnsetUserStack( pProcInfo );

    /* [TODO] ELFロード領域解放 */

    /* ページディレクトリ割当済判定 */
    if ( pProcInfo->dirId != MEMMNG_PAGE_DIR_ID_NULL ) {
        /* 割当済 */

        /* ページディレクトリ解放 */
        MemmngPageFreeDir( pProcInfo->dirId );
    }

    /* プロセス管理情報解放 */
    MLibDynamicArrayFree( &gProcTbl,
                          ( uint_t ) pProcInfo->pid,
                          &errMLib                   );

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
    MkProcParam_t *pParam;  /* パラメータ */

    /* 初期化 */
    pParam = ( MkProcParam_t * ) context.genReg.esi;

    /* パラメータチェック */
    if ( pParam == NULL ) {
        /* 不正 */

        return;
    }

    /* 機能ID判定 */
    switch ( pParam->funcId ) {
        case MK_PROC_FUNCID_SET_BREAKPOINT:
            /* ブレイクポイント設定 */
            SetBreakPoint( pParam );
            break;

        case MK_PROC_FUNCID_FORK:
            /* プロセス複製 */
            DoFork( pParam );
            break;

        default:
            /* 不正 */

            /* アウトプットパラメータ設定 */
            pParam->ret = MK_RET_FAILURE;
            pParam->err = MK_ERR_PARAM;
    }

    return;
}


/******************************************************************************/
/**
 * @brief       ブレイクポイント設定
 * @details     ブレイクポイントを設定する。必要があれば、仮想メモリの割り当て
 *              または解放を行う。
 *
 * @param[in]   *pParam パラメータ
 */
/******************************************************************************/
static void SetBreakPoint( MkProcParam_t *pParam )
{
    void       *pPhyAddr;   /* 物理アドレス             */
    void       *pVirtAddr;  /* ページ先頭アドレス       */
    int32_t    remain;      /* 残増減量                 */
    int32_t    quantity;    /* 増減量(ページサイズ以下) */
    uint32_t   breakPoint;  /* ブレイクポイント         */
    uint32_t   oldPageNum;  /* 旧ページ数               */
    uint32_t   newPageNum;  /* 新ページ数               */
    CmnRet_t   ret;         /* 呼出し関数戻り値         */
    ProcInfo_t *pProcInfo;  /* プロセス管理情報         */

    /* 初期化 */
    pPhyAddr   = NULL;
    pVirtAddr  = NULL;
    remain     = pParam->quantity;
    quantity   = 0;
    oldPageNum = 0;
    newPageNum = 0;
    ret        = CMN_SUCCESS;
    pProcInfo  = SchedGetProcInfo();
    breakPoint = ( uint32_t ) pProcInfo->userHeap.pBreakPoint;

    DEBUG_LOG_TRC(
        "%s(): pid=%#X, quantity=%d, breakPoint=0x%08X",
        __func__,
        pProcInfo->pid,
        remain,
        breakPoint
    );

    while ( remain != 0 ) {
        /* 残増減量比較 */
        if ( ( ( - IA32_PAGING_PAGE_SIZE ) <= remain                ) &&
             ( remain                      <= IA32_PAGING_PAGE_SIZE )    ) {
            /* ページサイズ以内の増減量 */

            quantity = remain;
            remain   = 0;

        } else if ( remain < ( - IA32_PAGING_PAGE_SIZE ) ) {
            /* ページサイズを超えた減量 */

            quantity  = ( - IA32_PAGING_PAGE_SIZE );
            remain   += IA32_PAGING_PAGE_SIZE;

        } else {
            /* ページサイズを超えた増量 */

            quantity  = IA32_PAGING_PAGE_SIZE;
            remain   -= IA32_PAGING_PAGE_SIZE;
        }

        /* ブレイクポイント増減前後のページ数計算 */
        oldPageNum = ( breakPoint - 1            ) / IA32_PAGING_PAGE_SIZE;
        newPageNum = ( breakPoint - 1 + quantity ) / IA32_PAGING_PAGE_SIZE;

        /* 新旧ページ数比較 */
        if ( oldPageNum < newPageNum ) {
            /* ページ数増加 */

            /* 物理メモリ領域割当 */
            pPhyAddr = MemmngPhysAlloc( IA32_PAGING_PAGE_SIZE );

            /* 割当結果判定 */
            if ( pPhyAddr == NULL ) {
                /* 失敗 */

                DEBUG_LOG_WRN( "%s(): MemmngPhysAlloc() error.", __func__ );

                /* 戻り値設定 */
                pParam->ret         = MK_RET_FAILURE;
                pParam->err         = MK_ERR_NO_MEMORY;
                pParam->pBreakPoint = ( void * ) breakPoint;

                return;
            }

            /* 0初期化 */
            MemmngCtrlSet( pPhyAddr, 0, IA32_PAGING_PAGE_SIZE );

            /* ページ先頭アドレス計算 */
            pVirtAddr = ( void * ) ( MLIB_UTIL_ALIGN( breakPoint - 1,
                                                      IA32_PAGING_PAGE_SIZE ) );

            /* ページングマッピング設定 */
            ret = MemmngPageSet( pProcInfo->dirId,
                                 pVirtAddr,
                                 pPhyAddr,
                                 IA32_PAGING_PAGE_SIZE,
                                 IA32_PAGING_G_NO,
                                 IA32_PAGING_US_USER,
                                 IA32_PAGING_RW_RW          );

            /* 設定結果判定 */
            if ( ret != CMN_SUCCESS ) {
                /* 失敗 */

                DEBUG_LOG_WRN( "%s(): MemmngPageSet() error(%d).", __func__, ret );

                /* 戻り値設定 */
                pParam->ret         = MK_RET_FAILURE;
                pParam->err         = MK_ERR_NO_MEMORY;
                pParam->pBreakPoint = ( void * ) breakPoint;

                return;
            }

        } else if ( oldPageNum > newPageNum ) {
            /* ページ数減少 */

            /* ページ先頭アドレス計算 */
            pVirtAddr = ( void * )
                        ( MLIB_UTIL_ALIGN( breakPoint - 1 + quantity,
                                           IA32_PAGING_PAGE_SIZE      ) );

            /* ページングマッピング解除 */
            MemmngPageUnset( pProcInfo->dirId,
                             pVirtAddr,
                             IA32_PAGING_PAGE_SIZE );

            /* 物理メモリ領域0初期化 */

            /* 物理メモリ領域解放 */

        }

        /* ブレイクポイント更新 */
        breakPoint += quantity;
    }

    /* ブレイクポイント設定 */
    pProcInfo->userHeap.pBreakPoint = ( void * ) breakPoint;

    /* 戻り値設定 */
    pParam->ret         = MK_RET_SUCCESS;
    pParam->err         = MK_ERR_NONE;
    pParam->pBreakPoint = ( void * ) breakPoint;

    return;
}


/******************************************************************************/
/**
 * @brief       ユーザスタック設定
 * @details     ユーザスタック領域を割り当てて、プロセス管理情報に設定する。
 *
 * @param[in]   *pProcInfo プロセス管理情報
 *
 * @return      処理結果を返す。
 * @retval      CMN_SUCCESS 成功
 * @retval      CMN_FIALURE 失敗
 */
/******************************************************************************/
static CmnRet_t SetUserStack( ProcInfo_t *pProcInfo )
{
    void            *pPhysAddr;     /* 物理メモリ領域 */
    CmnRet_t        ret;            /* 戻り値         */
    ProcStackInfo_t *pStackInfo;    /* スタック情報   */

    /* 初期化 */
    pPhysAddr  = NULL;
    ret        = CMN_FAILURE;
    pStackInfo = &( pProcInfo->userStack );

    /* 物理メモリ領域割当 */
    pPhysAddr = MemmngPhysAlloc( MEMMAP_VSIZE_USER_STACK );

    /* 割当結果判定 */
    if ( pPhysAddr == NULL ) {
        /* 失敗 */

        return CMN_FAILURE;
    }

    /* ページマッピング設定 */
    ret = MemmngPageSet(
              pProcInfo->dirId,                     /* ページディレクトリID */
              ( void * ) MEMMAP_VADDR_USER_STACK,   /* 仮想アドレス         */
              pPhysAddr,                            /* 物理アドレス         */
              MEMMAP_VSIZE_USER_STACK,              /* マッピングサイズ     */
              IA32_PAGING_G_NO,                     /* グローバルフラグ     */
              IA32_PAGING_US_USER,                  /* ユーザ/スーパバイザ  */
              IA32_PAGING_RW_RW                     /* 読込/書込許可        */
          );

    /* 設定結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* 物理メモリ領域解放 */
        MemmngPhysFree( pPhysAddr );

        return CMN_FAILURE;
    }

    /* ユーザスタック情報設定 */
    pStackInfo->pPhysAddr   = pPhysAddr;
    pStackInfo->pTopAddr    = ( void * ) MEMMAP_VADDR_USER_STACK;
    pStackInfo->pBottomAddr = ( void * ) ( MEMMAP_VADDR_USER_STACK +
                                           MEMMAP_VSIZE_USER_STACK -
                                           sizeof ( uint32_t )       );
    pStackInfo->size        = MEMMAP_VSIZE_USER_STACK;

    return CMN_SUCCESS;
}


/******************************************************************************/
/**
 * @brief       ユーザスタック削除
 * @details     ユーザスタック領域のマッピング解除と物理メモリ領域の解放を行う。
 *
 * @param[in]   *pProcInfo プロセス管理情報
 */
/******************************************************************************/
static void UnsetUserStack( ProcInfo_t *pProcInfo )
{
    /* スタック設定済判定 */
    if ( pProcInfo->userStack.pPhysAddr != NULL ) {
        /* スタック設定済 */

        /* メモリマッピング解除 */
        MemmngPageUnset(
            pProcInfo->dirId,               /* ページディレクトリID */
            pProcInfo->userStack.pTopAddr,  /* 仮想アドレス         */
            pProcInfo->userStack.size       /* マッピングサイズ     */
        );

        /* 物理メモリ領域解放 */
        MemmngHeapFree( pProcInfo->userStack.pPhysAddr );

        /* スタック管理情報初期化 */
        pProcInfo->userStack.pPhysAddr   = NULL;
        pProcInfo->userStack.pTopAddr    = NULL;
        pProcInfo->userStack.pBottomAddr = NULL;
        pProcInfo->userStack.size        = 0;
    }

    return;
}


/******************************************************************************/
