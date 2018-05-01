/******************************************************************************/
/* src/kernel/InitCtrl/InitCtrlInit.c                                         */
/*                                                                 2018/05/01 */
/* Copyright (C) 2016-2018 Mochi.                                             */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdarg.h>
#include <kernel/MochiKernel.h>
#include <hardware/IA32/IA32Instruction.h>
#include <MLib/Basic/MLibBasic.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <IntMng.h>
#include <MemMng.h>
#include <TaskMng.h>
#include <TimerMng.h>

/* 内部モジュールヘッダ */


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_INIT_INIT,   \
                    __LINE__,               \
                    __VA_ARGS__ )
#else
#define DEBUG_LOG( ... )
#endif


/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
static void InitLoadProcImg( void );


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       Mochi Kernel起動
 * @details     Mochi Kernelのエントリ関数。各モジュールの初期化を行う。
 */
/******************************************************************************/
void InitCtrlInit( void )
{
    /* [TODO]カーネル起動引数対応まで仮 */
    MochiKernelMemoryMap_t map[] =
        {
            /* カーネル領域 */
            {
                ( void * ) 0x00100000,              /* 先頭アドレス */
                0x04F00000,                         /* メモリサイズ */
                MOCHIKERNEL_MEMORY_TYPE_KERNEL      /* メモリタイプ */
            },
            /* 利用可能領域 */
            {
                ( void * ) 0x05000000,              /* 先頭アドレス */
                0x03000000,                         /* メモリサイズ */
                MOCHIKERNEL_MEMORY_TYPE_AVAILABLE   /* メモリタイプ */
            }
        };
    
    /* デバッグ制御初期化 */
    DebugInit();
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "Mochi Kernel start!!!" );
    
    /* メモリ管理モジュール初期化 */
    MemMngInit( map, 2 );
    
    /* タスク管理モジュール初期化 */
    TaskMngInit();
    
    /* 割込み管理モジュール初期化 */
    IntMngInit();
    
    /* タイマ管理モジュール初期化 */
    TimerMngInit();
    
    /* プロセスイメージ読込 */
    InitLoadProcImg();
    
    /* 割込み有効化 */
    IntMngPicEnable();
    IA32InstructionSti();
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "Idle running." );
    
    /* アイドル（仮） */
    while ( 1 ) {
        /* hlt */
        IA32InstructionHlt();
    }
    
    /* not retern */
}


/******************************************************************************/
/* ローカル関数定義                                                           */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       プロセスイメージ読込
 * @details     プロセスイメージを読み込み、タスクを追加する。
 */
/******************************************************************************/
static void InitLoadProcImg( void )
{
    void                *pAddr;     /* ファイルアドレス */
    uint8_t             type;       /* プロセスタイプ   */
    uint32_t            taskId;     /* タスクID         */
    MochiKernelImgHdr_t *pHeader;   /* ファイルヘッダ   */
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );
    
    /* 初期化 */
    pAddr   = ( void * ) MOCHIKERNEL_ADDR_PROCIMG;
    pHeader = ( MochiKernelImgHdr_t * ) pAddr;
    
    /* ファイル毎に繰り返し */
    while ( pHeader->fileSize != 0 ) {
        
        /* ファイルヘッダアドレス設定 */
        pAddr = pAddr + sizeof ( MochiKernelImgHdr_t );
        
        /* デバッグトレースログ出力 */
        DEBUG_LOG( "Header:" );
        DEBUG_LOG( " Name=%s", pHeader->fileName );
        DEBUG_LOG( " Size=%d", pHeader->fileSize );
        DEBUG_LOG( " Type=%d", pHeader->fileType );
        
        /* プロセスタイプ判定 */
        if ( pHeader->fileType == MOCHIKERNEL_PROCESS_TYPE_DRIVER ) {
            /* ドライバ */
            
            /* プロセスタイプ変換 */
            type = TASKMNG_TASK_TYPE_DRIVER;
            
        } else if ( pHeader->fileType == MOCHIKERNEL_PROCESS_TYPE_SERVER ) {
            /* サーバ */
            
            /* プロセスタイプ変換 */
            type = TASKMNG_TASK_TYPE_SERVER;
            
        } else if ( pHeader->fileType == MOCHIKERNEL_PROCESS_TYPE_USER ) {
            /* ユーザ */
            
            /* プロセスタイプ変換 */
            type = TASKMNG_TASK_TYPE_USER;
        }
        
        /* タスク追加 */
        taskId = TaskMngTaskAdd(
                     type,
                     ( ( void * ) pHeader ) + sizeof ( MochiKernelImgHdr_t ),
                     pHeader->fileSize );
        
        /* タスク追加結果判定 */
        if ( taskId == TASKMNG_TASK_ID_NULL ) {
            /* 失敗 */
            
            /* デバッグトレースログ出力 */
            DEBUG_LOG( "TaskMngTaskAdd() error." );
            
        } else {
            /* 成功 */
            
            /* デバッグトレースログ出力 */
            DEBUG_LOG( "TaskMngTaskAdd() OK. taskId=%d", taskId );
        }
        
        /* アドレス更新 */
        pAddr   = pAddr + MLIB_BASIC_ALIGN( pHeader->fileSize, 512 );
        pHeader = ( MochiKernelImgHdr_t * ) pAddr;
    }
    
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );
    
    return;
}


/******************************************************************************/
