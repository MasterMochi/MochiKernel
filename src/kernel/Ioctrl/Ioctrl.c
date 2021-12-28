/******************************************************************************/
/*                                                                            */
/* src/kernel/Ioctrl/Ioctrl.c                                                 */
/*                                                                 2021/11/27 */
/* Copyright (C) 2018-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>

/* 内部モジュールヘッダ */
#include "IoctrlMem.h"
#include "IoctrlPort.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_IOCTRL_MAIN, \
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
 * @brief       入出力制御初期化
 * @details     入出力制御内サブモジュールの初期化を行う。
 */
/******************************************************************************/
void IoctrlInit( void )
{
    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* I/Oポート制御サブモジュール初期化 */
    PortInit();

    /* I/Oメモリ制御サブモジュール初期化 */
    MemInit();

    /* デバッグトレースログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/