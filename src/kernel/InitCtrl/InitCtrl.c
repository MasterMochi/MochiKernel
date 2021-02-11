/******************************************************************************/
/*                                                                            */
/* src/kernel/InitCtrl/InitCtrl.c                                             */
/*                                                                 2021/02/01 */
/* Copyright (C) 2016-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>

/* ライブラリヘッダ */
#include <MLib/MLibUtil.h>
#include <MLib/MLibWrapper.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32Instruction.h>
#include <kernel/config.h>
#include <kernel/kernel.h>
#include <kernel/types.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <IntMng.h>
#include <IoCtrl.h>
#include <ItcCtrl.h>
#include <Memmng.h>
#include <TaskMng.h>
#include <TimerMng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_INIT_MAIN,   \
                    __LINE__,               \
                    __VA_ARGS__           )
#else
#define DEBUG_LOG( ... )
#endif

/** ラッパー関数テーブル */
static const MLibWrapperFunc_t gMLibFunc =
    {
        &MLibUtilCopyMemoryCmpt,    /* memcpy */
        &MLibUtilSetMemoryCmpt,     /* memset */
        &MemmngHeapAlloc,           /* malloc */
        &MemmngHeapFree             /* free   */
    };

/******************************************************************************/
/* ローカル関数宣言                                                           */
/******************************************************************************/
/* プロセスイメージ読込 */
static void LoadProcImg( void );


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       MochiKernel起動
 * @details     MochiKernelのエントリ関数。各モジュールの初期化を行う。
 */
/******************************************************************************/
void InitCtrlInit( void )
{
    /* MLIB初期化 */
    MLibWrapperInit( gMLibFunc );

    /* デバッグ制御初期化 */
    DebugInit();

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "Mochi Kernel start!!!" );

    /* メモリ管理モジュール初期化 */
    MemmngInit( ( BiosE820Entry_t * ) 0x00000504,
                *( ( size_t * )       0x00000500 ),
                ( MkMemMapEntry_t * ) 0x00000CD8,
                *( ( size_t * )       0x00000CD4 )  );

    /* 割込み管理モジュール初期化 */
    IntMngInit();

    /* タスク管理モジュール初期化 */
    TaskMngInit();

    /* タイマ管理モジュール初期化 */
    TimerMngInit();

    /* タスク間通信制御モジュール初期化 */
    ItcCtrlInit();

    /* 入出力制御モジュール初期化 */
    IoCtrlInit();

    /* プロセスイメージ読込 */
    LoadProcImg();

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
 * @details     プロセスイメージを読み込み、プロセスを追加する。
 */
/******************************************************************************/
static void LoadProcImg( void )
{
    void       *pAddr;      /* ファイルアドレス */
    uint8_t    type;        /* プロセスタイプ   */
    MkPid_t    pid;         /* プロセスID       */
    MkImgHdr_t *pHeader;    /* ファイルヘッダ   */

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* 初期化 */
    pAddr   = ( void * ) MK_ADDR_PROCIMG;
    type    = TASKMNG_PROC_TYPE_USER;
    pid     = MK_PID_NULL;
    pHeader = ( MkImgHdr_t * ) pAddr;

    /* ファイル毎に繰り返し */
    while ( pHeader->fileSize != 0 ) {

        /* ファイルヘッダアドレス設定 */
        pAddr = pAddr + sizeof ( MkImgHdr_t );

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "Header:" );
        DEBUG_LOG( " Name=%s", pHeader->fileName );
        DEBUG_LOG( " Size=%d", pHeader->fileSize );
        DEBUG_LOG( " Type=%d", pHeader->fileType );

        /* プロセスタイプ判定 */
        if ( pHeader->fileType == MK_PROC_TYPE_DRIVER ) {
            /* ドライバ */

            /* プロセスタイプ変換 */
            type = TASKMNG_PROC_TYPE_DRIVER;

        } else if ( pHeader->fileType == MK_PROC_TYPE_SERVER ) {
            /* サーバ */

            /* プロセスタイプ変換 */
            type = TASKMNG_PROC_TYPE_SERVER;

        } else if ( pHeader->fileType == MK_PROC_TYPE_USER ) {
            /* ユーザ */

            /* プロセスタイプ変換 */
            type = TASKMNG_PROC_TYPE_USER;
        }

        /* プロセス追加 */
        pid = TaskMngProcAdd( type,
                              pAddr,
                              pHeader->fileSize );

        /* プロセス追加結果判定 */
        if ( pid == MK_PID_NULL ) {
            /* 失敗 */

            /* デバッグトレースログ出力 */
            DEBUG_LOG( "TaskMngProcAdd() error." );

        } else {
            /* 成功 */

            /* デバッグトレースログ出力 */
            DEBUG_LOG( "TaskMngProcAdd() OK. pid=%d", pid );
        }

        /* アドレス更新 */
        pAddr   = pAddr + MLIB_UTIL_ALIGN( pHeader->fileSize, 512 );
        pHeader = ( MkImgHdr_t * ) pAddr;
    }

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
