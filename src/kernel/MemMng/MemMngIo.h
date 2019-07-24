/******************************************************************************/
/*                                                                            */
/* src/kernel/MemMng/MemMngIo.h                                               */
/*                                                                 2019/07/22 */
/* Copyright (C) 2018-2019 Mochi.                                             */
/*                                                                            */
/******************************************************************************/
#ifndef MEMMNG_IO_H
#define MEMMNG_IO_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 標準ヘッダ */
#include <stdint.h>

/* 共通ヘッダ */
#include <kernel/kernel.h>


/******************************************************************************/
/* 内部モジュール向けグローバル関数プロトタイプ宣言                           */
/******************************************************************************/
/* I/Oメモリ領域管理初期化 */
extern void IoInit( MkMemMapEntry_t *pMemMap,
                    size_t          entryNum  );


/******************************************************************************/
#endif
