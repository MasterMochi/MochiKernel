/******************************************************************************/
/*                                                                            */
/* src/booter/Memmng/Memmng.c                                                 */
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
#include "MemmngMap.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                    \
    DebugLogOutput( CMN_MODULE_MEMMNG_MAIN, \
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
 * @brief       メモリ管理初期化
 * @details     メモリ管理のサブモジュールを初期化する。
 */
/******************************************************************************/
void MemmngInit( void )
{
    /* デバッグログ出力 */
    DEBUG_LOG( "%s() start.", __func__ );

    /* メモリマップ管理初期化 */
    MemmngMapInit();

    /* デバッグログ出力 */
    DEBUG_LOG( "%s() end.", __func__ );

    return;
}


/******************************************************************************/
