/******************************************************************************/
/*                                                                            */
/* src/booter/include/Memmng.h                                                */
/*                                                                 2021/11/27 */
/* Copyright (C) 2017-2021 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef MEMMNG_H
#define MEMMNG_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stddef.h>
#include <stdint.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>


/******************************************************************************/
/* グローバル関数プロトタイプ宣言                                             */
/******************************************************************************/
/*----------*/
/* Memmng.c */
/*----------*/
/* メモリ管理初期化 */
extern void MemmngInit( void );


/*-------------*/
/* MemmngMap.c */
/*-------------*/
/* メモリマップ設定 */
extern void MemmngMapSet( void );

/* メモリマップリスト設定 */
extern CmnRet_t MemmngMapSetList( void     *pAddr,
                                  size_t   size,
                                  uint32_t type    );


/******************************************************************************/
#endif