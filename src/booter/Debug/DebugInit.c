/******************************************************************************/
/* src/booter/Debug/DebugInit.c                                               */
/*                                                                 2017/06/26 */
/* Copyright (C) 2017 Mochi.                                                  */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stdarg.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>

/* 内部モジュールヘッダ */
#include "DebugLog.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_DEBUG_INIT,  \
                    __LINE__,               \
                    __VA_ARGS__ )
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