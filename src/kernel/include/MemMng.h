/******************************************************************************/
/* src/kernel/include/MemMng.h                                                */
/*                                                                 2017/02/27 */
/* Copyright (C) 2016-2017 Mochi.                                             */
/******************************************************************************/
#ifndef MEMMNG_H
#define MEMMNG_H
/******************************************************************************/
/* インクルード                                                               */
/******************************************************************************/
/* 共通ヘッダ */
#include <stddef.h>
#include <stdint.h>
#include <kernel/MochiKernel.h>

/* 外部モジュールヘッダ */
#include <Cmn.h>


/******************************************************************************/
/* 定義                                                                       */
/******************************************************************************/
/* GDT定義 */
#define MEMMNG_GDT_ENTRY_FULL  (  0 )           /**< GDTエントリ空き無し   */
#define MEMMNG_GDT_ENTRY_MIN   (  1 )           /**< GDTエントリ番号最小値 */
#define MEMMNG_GDT_ENTRY_MAX   (  9 )           /**< GDTエントリ番号最大値 */
#define MEMMNG_GDT_ENTRY_NUM   \
    ( MEMMNG_GDT_ENTRY_MAX + 1 )                /**< GDTエントリ数         */

/* セグメントセレクタ定義 */
#define MEMMNG_SEGSEL_KERNEL_CODE ( 1 * 8     ) /**< カーネルコードセグメント */
#define MEMMNG_SEGSEL_KERNEL_DATA ( 2 * 8     ) /**< カーネルデータセグメント */
#define MEMMNG_SEGSEL_DRIVER_CODE ( 3 * 8 + 1 ) /**< ドライバコードセグメント */
#define MEMMNG_SEGSEL_DRIVER_DATA ( 4 * 8 + 1 ) /**< ドライバデータセグメント */
#define MEMMNG_SEGSEL_SERVER_CODE ( 5 * 8 + 2 ) /**< サーバコードセグメント   */
#define MEMMNG_SEGSEL_SERVER_DATA ( 6 * 8 + 2 ) /**< サーバデータセグメント   */
#define MEMMNG_SEGSEL_USER_CODE   ( 7 * 8 + 3 ) /**< ユーザコードセグメント   */
#define MEMMNG_SEGSEL_USER_DATA   ( 8 * 8 + 3 ) /**< ユーザデータセグメント   */


/******************************************************************************/
/* グローバル関数宣言                                                         */
/******************************************************************************/
/*--------------*/
/* MemMngArea.c */
/*--------------*/
/* メモリ領域割当 */
extern void *MemMngAreaAlloc( size_t size );

/* メモリ領域解放 */
extern CmnRet_t MemMngAreaFree( void *pAddr );


/*-------------*/
/* MemMngGdt.c */
/*-------------*/
/* GDTエントリ追加 */
extern uint16_t MemMngGdtAdd( void    *pBase,
                              size_t  limit,
                              uint8_t limitG,
                              uint8_t sysFlg,
                              uint8_t type,
                              uint8_t level,
                              uint8_t opSize  );


/*--------------*/
/* MemMngInit.c */
/*--------------*/
/* メモリ管理初期化 */
extern void MemMngInit( MochiKernelMemoryMap_t *pMap,
                        size_t                 mapSize );


/******************************************************************************/
#endif
