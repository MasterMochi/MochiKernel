/******************************************************************************/
/*                                                                            */
/* src/booter/include/MemMng.h                                                */
/*                                                                 2019/07/24 */
/* Copyright (C) 2017-2019 Mochi.                                             */
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
/* MemMng.c */
/*----------*/
/* メモリ管理初期化 */
extern void MemMngInit( void );


/*-------------*/
/* MemMngMap.c */
/*-------------*/
/* メモリマップ設定 */
extern void MemMngMapSet( void );

/* メモリマップリスト設定 */
extern CmnRet_t MemMngMapSetList( void     *pAddr,
                                  size_t   size,
                                  uint32_t type    );


/******************************************************************************/
#endif
