/******************************************************************************/
/*                                                                            */
/* src/kernel/TaskMng/TaskMngProc.c                                           */
/*                                                                 2019/07/23 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

/* ライブラリヘッダ */
#include <MLib/MLib.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32Instruction.h>
#include <kernel/config.h>
#include <kernel/proc.h>
#include <kernel/types.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <IntMng.h>
#include <MemMng.h>
#include <TaskMng.h>

/* 内部モジュールヘッダ */
#include "TaskMngElf.h"
#include "TaskMngSched.h"
#include "TaskMngTask.h"


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

/** プロセス管理テーブル構造体 */
typedef struct {
    uint8_t    used;                        /**< 使用フラグ           */
    uint8_t    type;                        /**< プロセスタイプ       */
    uint8_t    reserved[ 2 ];               /**< 予約                 */
    uint32_t   pageDirId;                   /**< ページディレクトリID */
    void       *pEntryPoint;                /**< エントリポイント     */
    void       *pEndPoint;                  /**< エンドポイント       */
    void       *pBreakPoint;                /**< ブレイクポイント     */
    MkTaskId_t taskId[ MK_TID_NUM ]; /**< タスクIDリスト       */
} ProcTbl_t;


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** プロセス管理テーブル */
static ProcTbl_t gProcTbl[ MK_PID_NUM ];


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* プロセス開始 */
static void TaskMngProcStart( void );
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
 * @details     プロセス追加を行う。
 *
 * @param[in]   type   プロセスタイプ
 *                  - MK_PROCESS_TYPE_DRIVER ドライバプロセス
 *                  - MK_PROCESS_TYPE_SERVER サーバプロセス
 *                  - MK_PROCESS_TYPE_USER   ユーザプロセス
 * @param[in]   *pAddr 実行ファイルアドレス
 * @param[in]   size   実行ファイルサイズ
 *
 * @return      追加時に割り当てたタスクIDを返す。
 * @retval      MK_PID_NULL 失敗
 * @retval      MK_PID_MIN  タスクID最小値
 * @retval      MK_PID_MAX  タスクID最大値
 */
/******************************************************************************/
MkPid_t TaskMngProcAdd( uint8_t type,
                        void    *pAddr,
                        size_t  size    )
{
    void       *pEntryPoint;    /* エントリポイント     */
    void       *pEndPoint;      /* エンドポイント       */
    void       *pBreakPoint;    /* ブレイクポイント     */
    uint32_t   pageDirId;       /* ページディレクトリID */
    CmnRet_t   ret;             /* 関数戻り値           */
    MkPid_t    pid;             /* PID                  */
    MkTaskId_t taskId;          /* タスクID             */

    /* 初期化 */
    pEntryPoint = NULL;
    pBreakPoint = NULL;
    pageDirId   = MEMMNG_PAGE_DIR_FULL;
    ret         = CMN_FAILURE;
    pid         = MK_PID_NULL;
    taskId      = MK_TASKID_NULL;

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.",                    __func__          );
    DEBUG_LOG( " type=%u, pAddr=%010p, size=%d", type, pAddr, size );

    /* 未使用PID検索 */
    for ( pid = MK_PID_MIN; pid <= MK_PID_MAX; pid++ ) {
        /* 使用フラグ判定 */
        if ( gProcTbl[ pid ].used == CMN_UNUSED ) {
            /* 未使用 */

            /* 仮想メモリ領域管理開始 */
            ret = MemMngVirtStart( pid );

            /* 管理開始結果判定 */
            if ( ret == CMN_FAILURE ) {
                /* 失敗 */

                /* [TODO] */

                return MK_TASKID_NULL;
            }

            /* ページディレクトリ割当 */
            pageDirId = MemMngPageAllocDir();

            /* 割当結果判定 */
            if ( pageDirId == MEMMNG_PAGE_DIR_FULL ) {
                /* 失敗 */

                /* [TODO] */

                /* デバッグトレースログ出力 */
                DEBUG_LOG( "%s() end. ret=NULL", __func__ );

                return MK_PID_NULL;
            }

            /* 仮想メモリ領域設定 */
            MemMngVirtAllocSpecified( pid,
                                      ( void * ) MK_CONFIG_ADDR_BOOTDATA,
                                      MK_CONFIG_SIZE_BOOTDATA                 );
            MemMngVirtAllocSpecified( pid,
                                      ( void * ) MK_CONFIG_ADDR_KERNEL_START,
                                      MK_CONFIG_SIZE_KERNEL                   );
            MemMngVirtAllocSpecified( pid,
                                      ( void * ) MK_CONFIG_ADDR_APL_START,
                                      MK_CONFIG_SIZE_APL                      );

            /* ELFファイル読込 */
            ret = TaskMngElfLoad( pAddr,
                                  size,
                                  pageDirId,
                                  &pEntryPoint,
                                  &pEndPoint    );

            /* 読込結果判定 */
            if ( ret != CMN_SUCCESS ) {
                /* 失敗 */

                /* [TODO]ページディレクトリ解放 */

                /* デバッグトレースログ出力 */
                DEBUG_LOG( "%s() end. ret=NULL", __func__ );

                return MK_PID_NULL;
            }

            /* タスク追加 */
            taskId = TaskMngTaskAdd( pid, 0, pageDirId, &TaskMngProcStart );

            /* タスク追加結果判定 */
            if ( taskId == MK_TASKID_NULL ) {
                /* 失敗 */

                /* [TODO]ELFファイル読込メモリ解放 */
                /* [TODO]ページディレクトリ解放    */

                /* デバッグトレースログ出力 */
                DEBUG_LOG( "%s() end. ret=NULL", __func__ );

                return MK_PID_NULL;
            }

            /* ブレイクポイント設定 */
            pBreakPoint = ( void * ) MLIB_ALIGN( ( uint32_t ) pEndPoint - 1,
                                                 IA32_PAGING_PAGE_SIZE       );

            /* プロセス情報設定 */
            gProcTbl[ pid ].used        = CMN_USED;     /* 使用フラグ           */
            gProcTbl[ pid ].type        = type;         /* プロセスタイプ       */
            gProcTbl[ pid ].pageDirId   = pageDirId;    /* ページディレクトリID */
            gProcTbl[ pid ].pEntryPoint = pEntryPoint;  /* エントリポイント     */
            gProcTbl[ pid ].pEndPoint   = pEndPoint;    /* エンドポイント       */
            gProcTbl[ pid ].pBreakPoint = pBreakPoint;  /* ブレイクポイント     */
            gProcTbl[ pid ].taskId[ 0 ] = taskId;       /* タスクID             */

            /* スケジューラ追加 */
            ret = TaskMngSchedAdd( taskId );

            /* 追加結果判定 */
            if ( ret == CMN_FAILURE ) {
                /* 失敗 */

                /* [TODO] */

                return MK_TASKID_NULL;
            }

            /* デバッグトレースログ出力 */
            DEBUG_LOG( "%s() end. ret=%d", __func__, pid );

            return pid;
        }
    }

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=NULL", __func__ );

    return MK_PID_NULL;
}


/******************************************************************************/
/**
 * @brief       ページディレクトリID取得
 * @details     指定したプロセスIDのページディレクトリIDを取得する。
 *
 * @param[in]   pid プロセスID
 *                  - MK_PID_MIN プロセスID最小値
 *                  - MK_PID_MAX プロセスID最大値
 *
 * @return      ページディレクトリIDを返す。
 */
/******************************************************************************/
uint32_t TaskMngProcGetPageDirId( MkPid_t pid )
{
    /* ページディレクトリID返却 */
    return gProcTbl[ pid ].pageDirId;
}


/******************************************************************************/
/**
 * @brief       プロセスタイプ取得
 * @details     指定したプロセスIDのプロセスタイプを取得する。
 *
 * @param[in]   pid プロセスID
 *                  - MK_PID_MIN プロセスID最小値
 *                  - MK_PID_MAX プロセスID最大値
 *
 * @return      プロセスタイプを返す。
 * @retval      TASKMNG_PROC_TYPE_KERNEL カーネル
 * @retval      TASKMNG_PROC_TYPE_DRIVER ドライバ
 * @retval      TASKMNG_PROC_TYPE_SERVER サーバ
 * @retval      TASKMNG_PROC_TYPE_USER   ユーザ
 */
/******************************************************************************/
uint8_t TaskMngProcGetType( MkPid_t pid )
{
    /* タスクタイプ返却 */
    return gProcTbl[ pid ].type;
}


/******************************************************************************/
/**
 * @brief       プロセス管理初期化
 * @details     プロセス管理サブモジュールの初期化を行う。
 */
/******************************************************************************/
void TaskMngProcInit( void )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* タスク管理テーブル初期化 */
    memset( gProcTbl, 0, sizeof ( gProcTbl ) );

    /* カーネルプロセス設定 */
    gProcTbl[ TASKMNG_PID_IDLE ].used        = CMN_USED;
    gProcTbl[ TASKMNG_PID_IDLE ].type        = TASKMNG_PROC_TYPE_KERNEL;
    gProcTbl[ TASKMNG_PID_IDLE ].pageDirId   = MEMMNG_PAGE_DIR_ID_IDLE;
    gProcTbl[ TASKMNG_PID_IDLE ].pEntryPoint = NULL;
    gProcTbl[ TASKMNG_PID_IDLE ].taskId[ 0 ] = TASKMNG_TASKID_IDLE;

    /* 割込みハンドラ設定 */
    IntMngHdlSet( MK_CONFIG_INTNO_PROC,     /* 割込み番号     */
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
 * @brief       プロセス起動開始
 * @details     プロセスの起動を開始する。
 */
/******************************************************************************/
void TaskMngProcStart( void )
{
    void       *pEntryPoint;    /* エントリポイント         */
    void       *pStack;         /* スタックアドレス         */
    MkPid_t    pid;             /* プロセスID               */
    uint8_t    type;            /* プロセスタイプ           */
    uint32_t   codeSegSel;      /* コードセグメントセレクタ */
    uint32_t   dataSegSel;      /* データセグメントセレクタ */
    ProcTbl_t  *pProcInfo;      /* プロセス情報             */
    MkTaskId_t taskId;          /* タスクID                 */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* 初期化 */
    taskId      = TaskMngSchedGetTaskId();          /* タスクID         */
    pid         = TaskMngTaskGetPid( taskId );      /* PID              */
    pStack      = TaskMngTaskGetAplStack( taskId ); /* スタックアドレス */
    pProcInfo   = &( gProcTbl[ pid ] );             /* プロセス情報     */
    pEntryPoint = pProcInfo->pEntryPoint;           /* エントリポイント */
    type        = pProcInfo->type;                  /* プロセスタイプ   */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "taskId=%u, pid=%u, pEntryPoint=%p, pStack=%p",
               taskId,
               pid,
               pEntryPoint,
               pStack );

    /* タスクタイプ判定 */
    if ( type == TASKMNG_PROC_TYPE_KERNEL ) {
        /* カーネル */

        /* セグメントセレクタ設定 */
        codeSegSel = MEMMNG_SEGSEL_KERNEL_CODE;     /* コード */
        dataSegSel = MEMMNG_SEGSEL_KERNEL_DATA;     /* データ */

    } else {
        /* アプリ */

        codeSegSel = MEMMNG_SEGSEL_APL_CODE;        /* コード */
        dataSegSel = MEMMNG_SEGSEL_APL_DATA;        /* データ */
    }

    /* iretd命令用スタック設定 */
    __asm__ __volatile__ ( "push %0\n"              /* ss     */
                           "push %1\n"              /* esp    */
                           "push 0x00003202\n"      /* eflags */
                           "push %2\n"              /* cs     */
                           "push %3\n"              /* eip    */
                           :
                           : "a" ( dataSegSel ),
                             "b" ( ( uint32_t ) pStack - 16 ),
                             "c" ( codeSegSel ),
                             "d" ( ( uint32_t ) pEntryPoint )  );

    /* セグメントレジスタ設定用スタック設定 */
    __asm__ __volatile__ ( "push eax\n"             /* gs */
                           "push eax\n"             /* fs */
                           "push eax\n"             /* es */
                           "push eax\n" );          /* ds */

    /* 汎用レジスタ設定用スタック設定 */
    IA32InstructionPush( 0 );                       /* eax           */
    IA32InstructionPush( 0 );                       /* ecx           */
    IA32InstructionPush( 0 );                       /* edx           */
    IA32InstructionPush( 0 );                       /* ebx           */
    IA32InstructionPush( 0 );                       /* esp( 未使用 ) */
    IA32InstructionPush( 0 );                       /* ebp           */
    IA32InstructionPush( 0 );                       /* esi           */
    IA32InstructionPush( 0 );                       /* edi           */

    /* 汎用レジスタ設定 */
    IA32InstructionPopad();

    /* セグメントレジスタ設定 */
    IA32InstructionPopDs();
    IA32InstructionPopEs();
    IA32InstructionPopFs();
    IA32InstructionPopGs();

    /* タスクエントリポイントへ移行 */
    IA32InstructionIretd();

    /* not return */
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
            pParam->ret   = MK_PROC_RET_FAILURE;
            pParam->errNo = MK_PROC_ERR_PARAM_FUNCID;
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
    void       *pPhyAddr;       /* 物理アドレス             */
    void       *pVirtAddr;      /* ページ先頭アドレス       */
    int32_t    remain;          /* 残増減量                 */
    int32_t    quantity;        /* 増減量(ページサイズ以下) */
    MkPid_t    pid;             /* プロセスID               */
    uint32_t   breakPoint;      /* ブレイクポイント         */
    uint32_t   oldPageNum;      /* 旧ページ数               */
    uint32_t   newPageNum;      /* 新ページ数               */
    CmnRet_t   ret;             /* 呼出し関数戻り値         */
    MkTaskId_t taskId;          /* タスクID                 */

    /* 初期化 */
    taskId     = TaskMngSchedGetTaskId();
    pid        = TaskMngTaskGetPid( taskId );
    breakPoint = ( uint32_t ) gProcTbl[ pid ].pBreakPoint;
    remain     = pParam->quantity;

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
            pPhyAddr = MemMngPhysAlloc( IA32_PAGING_PAGE_SIZE );

            /* 割当結果判定 */
            if ( pPhyAddr == NULL ) {
                /* 失敗 */

                DEBUG_LOG( "%s(): MemMngPhysAlloc() error.", __func__ );

                /* 戻り値設定 */
                pParam->errNo       = MK_PROC_ERR_NO_MEMORY;
                pParam->ret         = MK_PROC_RET_FAILURE;
                pParam->pBreakPoint = ( void * ) breakPoint;

                return;
            }

            /* 0初期化 */
            MemMngCtrlSet( pPhyAddr, 0, IA32_PAGING_PAGE_SIZE );

            /* ページ先頭アドレス計算 */
            pVirtAddr = ( void * ) ( MLIB_ALIGN( breakPoint - 1,
                                                 IA32_PAGING_PAGE_SIZE ) );

            /* ページングマッピング設定 */
            ret = MemMngPageSet( gProcTbl[ pid ].pageDirId,
                                 pVirtAddr,
                                 pPhyAddr,
                                 IA32_PAGING_PAGE_SIZE,
                                 IA32_PAGING_G_NO,
                                 IA32_PAGING_US_USER,
                                 IA32_PAGING_RW_RW          );

            /* 設定結果判定 */
            if ( ret != CMN_SUCCESS ) {
                /* 失敗 */

                DEBUG_LOG( "%s(): MemMngPageSet() error(%d).", __func__, ret );

                /* 戻り値設定 */
                pParam->errNo       = MK_PROC_ERR_NO_MEMORY;
                pParam->ret         = MK_PROC_RET_FAILURE;
                pParam->pBreakPoint = ( void * ) breakPoint;

                return;
            }

        } else if ( oldPageNum > newPageNum ) {
            /* ページ数減少 */

            /* ページ先頭アドレス計算 */
            pVirtAddr = ( void * ) ( MLIB_ALIGN( breakPoint - 1 + quantity,
                                                 IA32_PAGING_PAGE_SIZE      ) );

            /* ページングマッピング解除 */
            MemMngPageUnset( gProcTbl[ pid ].pageDirId,
                             pVirtAddr,
                             IA32_PAGING_PAGE_SIZE      );
        }

        /* ブレイクポイント更新 */
        breakPoint += quantity;
    }

    /* ブレイクポイント設定 */
    gProcTbl[ pid ].pBreakPoint = ( void * ) breakPoint;

    /* 戻り値設定 */
    pParam->errNo       = MK_PROC_ERR_NONE;
    pParam->ret         = MK_PROC_RET_SUCCESS;
    pParam->pBreakPoint = ( void * ) breakPoint;

    return;
}


/******************************************************************************/
