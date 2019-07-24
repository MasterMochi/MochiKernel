/******************************************************************************/
/*                                                                            */
/* src/booter/include/Debug.h                                                 */
/*                                                                 2019/07/24 */
/* Copyright (C) 2017-2019 Mochi.                                             */
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
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
/*---------*/
/* Debug.c */
/*---------*/
/* デバッグ制御初期化 */
extern void DebugInit( void );

/*------------*/
/* DebugLog.c */
/*------------*/
/* トレースログ出力 */
extern void DebugLogOutput( uint32_t moduleId,
                            uint32_t lineNum,
                            char     *format,
                            ...                );


/******************************************************************************/
#endif
