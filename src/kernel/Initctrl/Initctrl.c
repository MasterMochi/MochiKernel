/******************************************************************************/
/*                                                                            */
/* src/kernel/Initctrl/Initctrl.c                                             */
/*                                                                 2024/06/18 */
/* Copyright (C) 2016-2024 Mochi.                                             */
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
#include <Intmng.h>
#include <Ioctrl.h>
#include <Itcctrl.h>
#include <Memmng.h>
#include <Taskmng.h>
#include <Timermng.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* モジュールID */
#define _MODULE_ID_ CMN_MODULE_INIT_MAIN

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
void InitctrlInit( void )
{
    /* MLIB初期化 */
    MLibWrapperInit( gMLibFunc );

    /* デバッグ制御初期化 */
    DebugInit();

    DEBUG_LOG_INF( "Mochi Kernel start!!!" );

    /* メモリ管理モジュール初期化 */
    MemmngInit( ( BiosE820Entry_t * ) 0x00000504,
                *( ( size_t * )       0x00000500 ),
                ( MkMemMapEntry_t * ) 0x00000CD8,
                *( ( size_t * )       0x00000CD4 )  );

    /* 割込み管理モジュール初期化 */
    IntmngInit();

    /* タスク管理モジュール初期化 */
    TaskmngInit();

    /* タイマ管理モジュール初期化 */
    TimermngInit();

    /* タスク間通信制御モジュール初期化 */
    ItcctrlInit();

    /* 入出力制御モジュール初期化 */
    IoctrlInit();

    /* プロセスイメージ読込 */
    LoadProcImg();

    /* 割込み有効化 */
    IntmngPicEnable();
    IA32InstructionSti();

    DEBUG_LOG_INF( "Idle running." );

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

    DEBUG_LOG_TRC( "%s() start.", __func__ );

    /* 初期化 */
    pAddr   = ( void * ) MK_ADDR_PROCIMG;
    type    = TASKMNG_PROC_TYPE_USER;
    pid     = MK_PID_NULL;
    pHeader = ( MkImgHdr_t * ) pAddr;

    /* ファイル毎に繰り返し */
    while ( pHeader->fileSize != 0 ) {

        /* ファイルヘッダアドレス設定 */
        pAddr = pAddr + sizeof ( MkImgHdr_t );

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
        pid = TaskmngProcAdd( type,
                              pAddr,
                              pHeader->fileSize );

        /* プロセス追加結果判定 */
        if ( pid == MK_PID_NULL ) {
            /* 失敗 */

            DEBUG_LOG_ERR(
                "%s(): failed. name=%s, size=%d, type=%d",
                __func__,
                pHeader->fileName,
                pHeader->fileSize,
                pHeader->fileType
            );

        } else {
            /* 成功 */

            DEBUG_LOG_INF(
                "%s(): name=%s, size=%d, type=%d, pid=%d",
                __func__,
                pHeader->fileName,
                pHeader->fileSize,
                pHeader->fileType,
                pid
            );
        }

        /* アドレス更新 */
        pAddr   = pAddr + MLIB_UTIL_ALIGN( pHeader->fileSize, 512 );
        pHeader = ( MkImgHdr_t * ) pAddr;
    }

    DEBUG_LOG_TRC( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
