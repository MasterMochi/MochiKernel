/******************************************************************************/
/* src/kernel/TaskMng/TaskMngProc.c                                           */
/*                                                                 2018/05/14 */
/* Copyright (C) 2018 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <hardware/IA32/IA32Instruction.h>
#include <kernel/config.h>
#include <kernel/types.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <MemMng.h>
#include <TaskMng.h>

/* 内部モジュールヘッダ */
#include "TaskMngElf.h"
#include "TaskMngSched.h"
#include "TaskMngTask.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                        \
    DebugLogOutput( CMN_MODULE_TASKMNG_PROC,    \
                    __LINE__,                   \
                    __VA_ARGS__ )
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
    MkTaskId_t taskId[ MK_CONFIG_TID_NUM ]; /**< タスクIDリスト       */
} ProcTbl_t;


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** プロセス管理テーブル */
static ProcTbl_t gProcTbl[ MK_CONFIG_PID_NUM ];


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* プロセス開始 */
static void TaskMngProcStart( void );


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
 * @return      タスクID
 * @retval      MK_CONFIG_PID_NULL 失敗
 * @retval      MK_CONFIG_PID_MIN  タスクID最小値
 * @retval      MK_CONFIG_PID_MAX  タスクID最大値
 */
/******************************************************************************/
MkPid_t TaskMngProcAdd( uint8_t type,
                        void    *pAddr,
                        size_t  size    )
{
    void       *pEntryPoint;    /* エントリポイント     */
    uint32_t   pageDirId;       /* ページディレクトリID */
    CmnRet_t   ret;             /* 関数戻り値           */
    MkPid_t    pid;             /* PID                  */
    MkTaskId_t taskId;          /* タスクID             */
    
    /* 初期化 */
    pEntryPoint = NULL;
    pageDirId   = MEMMNG_PAGE_DIR_FULL;
    ret         = CMN_FAILURE;
    pid         = MK_CONFIG_PID_NULL;
    taskId      = MK_CONFIG_TASKID_NULL;
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.",                    __func__          );
    DEBUG_LOG( " type=%u, pAddr=%010p, size=%d", type, pAddr, size );
    
    /* 未使用PID検索 */
    for ( pid = MK_CONFIG_PID_MIN; pid <= MK_CONFIG_PID_MAX; pid++ ) {
        /* 使用フラグ判定 */
        if ( gProcTbl[ pid ].used == CMN_UNUSED ) {
            /* 未使用 */
            
            /* 仮想メモリ領域管理開始 */
            ret = MemMngVirtStart( pid );
            
            /* 管理開始結果判定 */
            if ( ret == CMN_FAILURE ) {
                /* 失敗 */
                
                /* [TODO] */
                
                return MK_CONFIG_TASKID_NULL;
            }
            
            /* ページディレクトリ割当 */
            pageDirId = MemMngPageAllocDir();
            
            /* 割当結果判定 */
            if ( pageDirId == MEMMNG_PAGE_DIR_FULL ) {
                /* 失敗 */
                
                /* [TODO] */
                
                /* デバッグトレースログ出力 */
                DEBUG_LOG( "%s() end. ret=NULL", __func__ );
                
                return MK_CONFIG_PID_NULL;
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
            ret = TaskMngElfLoad( pAddr, size, pageDirId, &pEntryPoint );
            
            /* 読込結果判定 */
            if ( ret != CMN_SUCCESS ) {
                /* 失敗 */
                
                /* [TODO]ページディレクトリ解放 */
                
                /* デバッグトレースログ出力 */
                DEBUG_LOG( "%s() end. ret=NULL", __func__ );
                
                return MK_CONFIG_PID_NULL;
            }
            
            /* タスク追加 */
            taskId = TaskMngTaskAdd( pid, 0, pageDirId, &TaskMngProcStart );
            
            /* タスク追加結果判定 */
            if ( taskId == MK_CONFIG_TASKID_NULL ) {
                /* 失敗 */
                
                /* [TODO]ELFファイル読込メモリ解放 */
                /* [TODO]ページディレクトリ解放    */
                
                /* デバッグトレースログ出力 */
                DEBUG_LOG( "%s() end. ret=NULL", __func__ );
                
                return MK_CONFIG_PID_NULL;
            }
            
            /* プロセス情報設定 */
            gProcTbl[ pid ].used        = CMN_USED;     /* 使用フラグ           */
            gProcTbl[ pid ].type        = type;         /* プロセスタイプ       */
            gProcTbl[ pid ].pageDirId   = pageDirId;    /* ページディレクトリID */
            gProcTbl[ pid ].pEntryPoint = pEntryPoint;  /* エントリポイント     */
            gProcTbl[ pid ].taskId[ 0 ] = taskId;       /* タスクID             */
            
            /* スケジューラ追加 */
            ret = TaskMngSchedAdd( taskId );
            
            /* 追加結果判定 */
            if ( ret == CMN_FAILURE ) {
                /* 失敗 */
                
                /* [TODO] */
                
                return MK_CONFIG_TASKID_NULL;
            }
            
            /* デバッグトレースログ出力 */
            DEBUG_LOG( "%s() end. ret=%d", __func__, pid );
            
            return pid;
        }
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end. ret=NULL", __func__ );
    
    return MK_CONFIG_PID_NULL;
}


/******************************************************************************/
/**
 * @brief       ページディレクトリID取得
 * @details     指定したプロセスIDのページディレクトリIDを取得する。
 * 
 * @param[in]   pid プロセスID
 *                  - MK_CONFIG_PID_MIN プロセスID最小値
 *                  - MK_CONFIG_PID_MAX プロセスID最大値
 * 
 * @return      ページディレクトリID
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
 *                  - MK_CONFIG_PID_MIN プロセスID最小値
 *                  - MK_CONFIG_PID_MAX プロセスID最大値
 * 
 * @return      プロセスタイプ
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
