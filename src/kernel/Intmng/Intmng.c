/******************************************************************************/
/*                                                                            */
/* src/kernel/Intmng/Intmng.c                                                 */
/*                                                                 2024/06/01 */
/* Copyright (C) 2016-2024 Mochi.                                             */
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
#include "IntmngCtrl.h"
#include "IntmngHdl.h"
#include "IntmngIdt.h"
#include "IntmngPic.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* モジュールID */
#define _MODULE_ID_ CMN_MODULE_INTMNG_MAIN


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       割込み管理初期化
 * @details     割込み管理内サブモジュールの初期化を行う。
 */
/******************************************************************************/
void IntmngInit( void )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG_TRC( "%s() start.", __func__ );

    /* IDT管理サブモジュール初期化 */
    IntmngIdtInit();

    /* ハンドラ管理サブモジュール初期化 */
    IntmngHdlInit();

    /* PIC管理サブモジュール初期化 */
    IntmngPicInit();

    /* ハードウェア割込み制御サブモジュール初期化 */
    IntmngCtrlInit();

    /* デバッグトレースログ出力 */
    DEBUG_LOG_TRC( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
