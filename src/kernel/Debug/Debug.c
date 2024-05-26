/******************************************************************************/
/*                                                                            */
/* src/kernel/Debug/Debug.c                                                   */
/*                                                                 2024/05/13 */
/* Copyright (C) 2017-2024 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>
#include <Debug.h>

/* 内部モジュールヘッダ */
#include "DebugMem.h"
#include "DebugVram.h"


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/** デバッグトレースログ出力マクロ */
#ifdef DEBUG_LOG_ENABLE
#define DEBUG_LOG( ... )                 \
    DebugOutput( CMN_MODULE_DEBUG_MAIN,  \
                 __LINE__,               \
                 __VA_ARGS__            )
#else
#define DEBUG_LOG( ... )
#endif


/******************************************************************************/
/* 外部モジュール向けグローバル関数定義                                       */
/******************************************************************************/
/******************************************************************************/
/**
 * @brief       デバッグ初期化
 * @details     デバッグサブモジュールの初期化を行う。
 */
/******************************************************************************/
void DebugInit( void )
{
    /* メモリ出力サブモジュール初期化 */
    DebugMemInit();

    /* VRAM出力サブモジュール初期化 */
    DebugVramInit();

    return;
}


/******************************************************************************/
/**
 * @brief       ログ出力
 * @details     VRAMとメモリにログ出力を行う。
 *
 * @param[in]   moduleId モジュールID
 * @param[in]   lineNo   行番号
 * @param[in]   *pFormat フォーマット文字列
 *
 * @note        デバッグトレースログ出力の有効化は、コンパイルオプションにて
 *              「DEBUG_LOG_ENABLE」マクロを定義する事で行う。
 */
/******************************************************************************/
void DebugOutput( uint16_t   moduleId,
                  uint16_t   lineNo,
                  const char *pFormat,
                  ...                  )
{
    char    str[ 256 ]; /* 文字列           */
    va_list vaList;     /* 可変長引数リスト */

    /* 可変長引数リスト設定 */
    va_start( vaList, pFormat );

    /* フォーマット文字列生成 */
    vsnprintf( str, 256, pFormat, vaList );

    /* メモリ出力 */
    DebugMemOutput( moduleId, lineNo, str );

    /* VRAM出力 */
    DebugVramOutput( moduleId, lineNo, str );

    /* 可変長引数リスト解放 */
    va_end( vaList );

    return;
}


/******************************************************************************/
