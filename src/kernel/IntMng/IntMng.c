/******************************************************************************/
/*                                                                            */
/* src/kernel/IntMng/IntMng.c                                                 */
/*                                                                 2019/07/24 */
/* Copyright (C) 2016-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>

/* 内部モジュールヘッダ */
#include "IntMngCtrl.h"
#include "IntMngHdl.h"
#include "IntMngIdt.h"
#include "IntMngPic.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_INTMNG_MAIN, \
                    __LINE__,               \
                    __VA_ARGS__             )
#else
#define DEBUG_LOG( ... )
#endif


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       割込み管理初期化
 * @details     割込み管理内サブモジュールの初期化を行う。
 */
/******************************************************************************/
void IntMngInit( void )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* IDT管理サブモジュール初期化 */
    IntMngIdtInit();

    /* ハンドラ管理サブモジュール初期化 */
    IntMngHdlInit();

    /* PIC管理サブモジュール初期化 */
    IntMngPicInit();

    /* ハードウェア割込み制御サブモジュール初期化 */
    IntMngCtrlInit();

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/