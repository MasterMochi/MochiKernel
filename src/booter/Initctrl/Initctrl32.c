/******************************************************************************/
/*                                                                            */
/* src/booter/Initctrl/Initctrl32.c                                           */
/*                                                                 2021/11/27 */
/* Copyright (C) 2018-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stddef.h>

/* 共通ヘッダ */
#include <hardware/IA32/IA32Instruction.h>
#include <kernel/kernel.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>
#include <Driver.h>
#include <Intmng.h>
#include <Loadmng.h>
#include <Memmng.h>


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


/******************************************************************************/
/* 変数定義                                                                   */
/******************************************************************************/
/** カーネルメイン関数 */
void ( *MochiKernelMain )( void ) = ( void * ) MK_ADDR_ENTRY;


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       初期化（32bitモード）
 * @details     初期化してカーネルを読込み起動する。
 */
/******************************************************************************/
void InitctrlInit32( void )
{
    CmnRet_t ret;   /* 関数戻り値 */

    /* デバッグ制御初期化 */
    DebugInit();

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "MochiBooter start." );

    /* A20ライン有効化 */
    ret = DriverA20Enable();

    /* 有効化結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "DriverA20Enable() Failure. ret=%u", ret );

        /* アボート */
        CmnAbort();
    }

    /* メモリ管理初期化 */
    MemmngInit();

    /* 割り込み管理初期化 */
    IntmngInit();

    /* ドライバ初期化 */
    DriverInit();

    /* 割込み有効化 */
    IntmngPicEnable();
    IA32InstructionSti();

    /* ロード管理初期化 */
    LoadmngInit();

    /* カーネル読込み */
    LoadmngKernelLoad();

    /* プロセスファイル読込み */
    LoadmngProcLoad();

    /* 割込み無効化 */
    IA32InstructionCli();
    IntmngPicDisable();

    /* ブートデータ領域設定 */
    ret = MemmngMapSetList( 0, 4096, MK_MEM_TYPE_BOOTDATA );

    /* 設定結果判定 */
    if ( ret != CMN_SUCCESS ) {
        /* 失敗 */

        /* デバッグトレースログ出力 */
        DEBUG_LOG( "MemmngMapSetList() failed." );

        /* アボート */
        CmnAbort();
    }

    /* メモリマップ設定 */
    MemmngMapSet();

    /* スタックポインタ再設定 */
    IA32InstructionSetEsp( MK_ADDR_STACK + MK_SIZE_STACK );

    MochiKernelMain();

    /* トレースログ出力 */
    DEBUG_LOG( "%s() end. error.", __func__ );

    CmnAbort();
}


/******************************************************************************/
