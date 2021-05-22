/******************************************************************************/
/*                                                                            */
/* src/kernel/Taskmng/TaskmngProc.c                                           */
/*                                                                 2021/05/22 */
/* Copyright (C) 2018-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>
#include <stdint.h>

/* ライブラリヘッダ */
#include <MLib/MLibUtil.h>

/* 共通ヘッダ */
#include <kernel/config.h>
#include <kernel/proc.h>
#include <kernel/types.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <IntMng.h>
#include <Memmng.h>
#include <Taskmng.h>

/* 内部モジュールヘッダ */
#include "TaskmngElf.h"
#include "TaskmngSched.h"
#include "TaskmngTask.h"
#include "TaskmngTbl.h"
#include "TaskmngThread.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                        \
    DebugLogOutput( CMN_MODULE_TASKMNG_PROC,    \
                    __LINE__,                   \
                    __VA_ARGS__              )
#else
#define DEBUG_LOG( ... )
#endif


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* 割込みハンドラ */
static void HdlInt( uint32_t        intNo,
                    IntMngContext_t context );
/* ブレイクポイント設定 */
static void SetBreakPoint( MkProcParam_t *pParam );


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
    CmnRet_t      ret;          /* 関数戻り値       */
    MkTid_t       tid;          /* スレッドID       */
    TblProcInfo_t *pProcInfo;   /* プロセス管理情報 */

    /* 初期化 */
    ret       = CMN_FAILURE;
    pProcInfo = NULL;

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.",                    __func__          );
    DEBUG_LOG( " type=%u, pAddr=%010p, size=%d", type, pAddr, size );

    /* プロセス管理情報割当 */
    pProcInfo = TblAllocProcInfo();

    /* 取得結果判定 */
    if ( pProcInfo == NULL ) {
        /* 失敗 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=NULL", __func__ );

        return MK_PID_NULL;
    }

    /* ページディレクトリ割当 */
    pProcInfo->dirId = MemmngPageAllocDir( pProcInfo->pid );

    /* 割当結果判定 */
    if ( pProcInfo->dirId == MEMMNG_PAGE_DIR_ID_NULL ) {
        /* 失敗 */

        /* プロセス管理情報解放 */
        TblFreeProcInfo( pProcInfo );

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=NULL", __func__ );

        return MK_PID_NULL;
    }

    /* ページディレクトリベースレジスタ値取得 */
    pProcInfo->pdbr = MemmngPageGetPdbr( pProcInfo->dirId );

    /* 仮想メモリ領域管理開始 */
    ret = MemmngVirtStart( pProcInfo->pid );

    /* 管理開始結果判定 */
    if ( ret == CMN_FAILURE ) {
        /* 失敗 */

        /* [TODO]ページディレクトリ解放 */

        /* プロセス管理情報解放 */
        TblFreeProcInfo( pProcInfo );

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=NULL", __func__ );

        return MK_PID_NULL;
    }

    /* ELFファイル読込 */
    ret = ElfLoad( pAddr,
                   size,
                   pProcInfo->dirId,
                   &( pProcInfo->pEntryPoint ),
                   &( pProcInfo->pEndPoint   )  );

    /* 読込結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* [TODO]ページディレクトリ解放 */

        /* プロセス管理情報解放 */
        TblFreeProcInfo( pProcInfo );

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=NULL", __func__ );

        return MK_PID_NULL;
    }

    /* プロセスタイプ設定 */
    pProcInfo->type = type;

    /* ブレイクポイント設定 */
    pProcInfo->pBreakPoint =
        ( void * ) MLIB_UTIL_ALIGN( ( uint32_t ) pProcInfo->pEndPoint - 1,
                                    IA32_PAGING_PAGE_SIZE                  );

    /* スレッド追加 */
    tid = ThreadAddMain( pProcInfo );

    /* 追加結果判定 */
    if ( tid == MK_TID_NULL ) {
        /* 失敗 */

        /* [TODO]ページディレクトリ解放 */
        /* [TODO]ELFロード領域解放 */

        /* プロセス管理情報解放 */
        TblFreeProcInfo( pProcInfo );

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "%s() end. ret=NULL", __func__ );

        return MK_PID_NULL;
    }

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=%d", __func__, pProcInfo->pid );

    return pProcInfo->pid;
}


/******************************************************************************/
/* モジュール内グローバル関数定義                                             */
/******************************************************************************/
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
    TblProcInfo_t *pIdleProcInfo;   /* アイドルプロセス管理情報 */

    /* 初期化 */
    pIdleProcInfo = NULL;

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* アイドルプロセス管理情報割当 */
    pIdleProcInfo = TblAllocProcInfo();

    /* アイドルプロセス管理情報設定 */
    pIdleProcInfo->type        = TASKMNG_PROC_TYPE_KERNEL;
    pIdleProcInfo->dirId       = MEMMNG_PAGE_DIR_ID_IDLE;
    pIdleProcInfo->pdbr        = MemmngPageGetPdbr( MEMMNG_PAGE_DIR_ID_IDLE );
    pIdleProcInfo->pEntryPoint = NULL;
    pIdleProcInfo->pEndPoint   = NULL;
    pIdleProcInfo->pBreakPoint = NULL;

    /* 割込みハンドラ設定 */
    IntMngHdlSet( MK_PROC_INTNO,            /* 割込み番号     */
                  HdlInt,                   /* 割込みハンドラ */
                  IA32_DESCRIPTOR_DPL_3 );  /* 特権レベル     */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

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
    void          *pPhyAddr;    /* 物理アドレス             */
    void          *pVirtAddr;   /* ページ先頭アドレス       */
    int32_t       remain;       /* 残増減量                 */
    int32_t       quantity;     /* 増減量(ページサイズ以下) */
    uint32_t      breakPoint;   /* ブレイクポイント         */
    uint32_t      oldPageNum;   /* 旧ページ数               */
    uint32_t      newPageNum;   /* 新ページ数               */
    CmnRet_t      ret;          /* 呼出し関数戻り値         */
    TblProcInfo_t *pProcInfo;   /* プロセス管理情報         */

    /* 初期化 */
    pPhyAddr   = NULL;
    pVirtAddr  = NULL;
    remain     = pParam->quantity;
    quantity   = 0;
    oldPageNum = 0;
    newPageNum = 0;
    ret        = CMN_SUCCESS;
    pProcInfo  = SchedGetProcInfo();
    breakPoint = ( uint32_t ) pProcInfo->pBreakPoint;

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

                DEBUG_LOG( "%s(): MemmngPhysAlloc() error.", __func__ );

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

                DEBUG_LOG( "%s(): MemmngPageSet() error(%d).", __func__, ret );

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
        }

        /* ブレイクポイント更新 */
        breakPoint += quantity;
    }

    /* ブレイクポイント設定 */
    pProcInfo->pBreakPoint = ( void * ) breakPoint;

    /* 戻り値設定 */
    pParam->ret         = MK_RET_SUCCESS;
    pParam->err         = MK_ERR_NONE;
    pParam->pBreakPoint = ( void * ) breakPoint;

    return;
}


/******************************************************************************/
