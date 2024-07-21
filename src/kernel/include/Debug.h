/******************************************************************************/
/*                                                                            */
/* src/kernel/include/Debug.h                                                 */
/*                                                                 2024/06/16 */
/* Copyright (C) 2017-2024 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef DEBUG_H
#define DEBUG_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdarg.h>
#include <stdint.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* ログレベル */
#define DEBUG_LV_ABORT      ( 0x00 )    /**< ログレベルAbort   */
#define DEBUG_LV_ERROR      ( 0x01 )    /**< ログレベルError   */
#define DEBUG_LV_WARNING    ( 0x02 )    /**< ログレベルWarning */
#define DEBUG_LV_INFO       ( 0x03 )    /**< ログレベルInfo    */
#define DEBUG_LV_TRACE      ( 0x04 )    /**< ログレベルTrace   */
#define DEBUG_LV_TEMP       ( 0x05 )    /**< ログレベルTemp    */

/* Abortデバッグログ出力マクロ */
#ifdef DEBUG_ABORT_ENABLE
# define DEBUG_LOG_ABT( ... ) \
    DebugOutput( _MODULE_ID_, __LINE__, DEBUG_LV_ABORT, __VA_ARGS__ )
#else
# define DEBUG_LOG_ABT( ... )
#endif

/* Errorデバッグログ出力マクロ */
#ifdef DEBUG_ERROR_ENABLE
# define DEBUG_LOG_ERR( ... ) \
    DebugOutput( _MODULE_ID_, __LINE__, DEBUG_LV_ERROR, __VA_ARGS__ )
#else
# define DEBUG_LOG_ERR( ... )
#endif

/* Warningデバッグログ出力マクロ */
#ifdef DEBUG_WARNING_ENABLE
# define DEBUG_LOG_WRN( ... ) \
    DebugOutput( _MODULE_ID_, __LINE__, DEBUG_LV_WARNING, __VA_ARGS__ )
#else
# define DEBUG_LOG_WRN( ... )
#endif

/* Infoデバッグログ出力マクロ */
#ifdef DEBUG_INFO_ENABLE
# define DEBUG_LOG_INF( ... ) \
    DebugOutput( _MODULE_ID_, __LINE__, DEBUG_LV_INFO, __VA_ARGS__ )
#else
# define DEBUG_LOG_WRN( ... )
#endif

/* Traceデバッグログ出力マクロ */
#ifdef DEBUG_TRACE_ENABLE
# define DEBUG_LOG_TRC( ... ) \
    DebugOutput( _MODULE_ID_, __LINE__, DEBUG_LV_TRACE, __VA_ARGS__ )
#else
# define DEBUG_LOG_TRC( ... )
#endif

/* Tempデバッグログ出力マクロ */
#ifdef DEBUG_TEMP_ENABLE
# define DEBUG_LOG_TMP( ... ) \
    DebugOutput( _MODULE_ID_, __LINE__, DEBUG_LV_TEMP, __VA_ARGS__ )
#else
# define DEBUG_LOG_TMP( ... )
#endif


/******************************************************************************/
/* 外部モジュール向けグローバル関数プロトタイプ宣言                           */
/******************************************************************************/
/*---------*/
/* Debug.c */
/*---------*/
/* デバッグ制御初期化 */
extern void DebugInit( void );

/* ログ出力 */
extern void DebugOutput( uint16_t   moduleId,
                         uint16_t   lineNo,
                         uint8_t    lv,
                         const char *pFormat,
                         ...                  );


/******************************************************************************/
#endif
