/******************************************************************************/
/*                                                                            */
/* src/kernel/Memmng/MemmngMap.h                                              */
/*                                                                 2020/11/03 */
/* Copyright (C) 2018-2020 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef MEMMNG_MAP_H
#define MEMMNG_MAP_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdint.h>

/* 共通ヘッダ */
#include <firmware/bios/e820.h>
#include <kernel/kernel.h>


/******************************************************************************/
/* 内部モジュール向けグローバル関数プロトタイプ宣言                           */
/******************************************************************************/
/* メモリマップ管理初期化 */
extern void MapInit( BiosE820Entry_t *pBiosE820,
                     size_t          biosE820Num,
                     MkMemMapEntry_t *pMemMap,
                     size_t          memMapNum    );


/******************************************************************************/
#endif