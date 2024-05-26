/******************************************************************************/
/*                                                                            */
/* src/kernel/Timermng/Timermng.c                                             */
/*                                                                 2021/11/27 */
/* Copyright (C) 2016-2021 Mochi.                                             */
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
#include "TimermngCtrl.h"
#include "TimermngPit.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                     \
    DebugOutput( CMN_MODULE_TIMERMNG_MAIN,   \
                 __LINE__,                   \
                 __VA_ARGS__               )
#else
#define DEBUG_LOG( ... )
#endif


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       タイマ管理初期化
 * @details     タイマ管理内サブモジュールの初期化を行う。
 */
/******************************************************************************/
void TimermngInit( void )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* タイマ制御サブモジュール初期化 */
    CtrlInit();

    /* PIT管理サブモジュール初期化 */
    TimermngPitInit();

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
