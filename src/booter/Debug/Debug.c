/******************************************************************************/
/*                                                                            */
/* src/booter/Debug/Debug.c                                                   */
/*                                                                 2019/07/24 */
/* Copyright (C) 2017-2019 Mochi.                                             */
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
#include "DebugLog.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_DEBUG_MAIN,  \
                    __LINE__,               \
                    __VA_ARGS__            )
#else
#define DEBUG_LOG( ... )
#endif


/******************************************************************************/
/* グローバル関数定義                                                         */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       デバッグ制御初期化
 * @details     デバッグ制御内サブモジュールの初期化を行う。
 */
/******************************************************************************/
void DebugInit( void )
{
    /* ログ管理サブモジュール初期化 */
    DebugLogInit();

    return;
}


/******************************************************************************/
